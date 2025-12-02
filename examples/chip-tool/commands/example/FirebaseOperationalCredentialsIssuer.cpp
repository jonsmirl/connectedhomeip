/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "FirebaseOperationalCredentialsIssuer.h"

#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>
#include <lib/support/Base64.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <cstdio>
#include <lib/support/BytesToHex.h>

using namespace chip;
using namespace chip::Crypto;
using namespace chip::Credentials;
using namespace chip::TLV;

// Certificate validity period (40 years in seconds) — ensures validity beyond current LKG time
static constexpr uint32_t kCertificateValidityPeriodSec = 365 * 24 * 3600 * 40;

// Increased buffer sizes for M2 certificate generation
static constexpr uint32_t kMaxM2DERCertLength  = 1200; // Double the standard size
static constexpr uint32_t kMaxM2CHIPCertLength = 800;  // Double the standard size

// M2 app IPK (Identity Protection Key) - matches the base64 decoded value from M2 app
static constexpr uint8_t kM2IPK[16] = {
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, // "12345678"
    0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36  // "90123456"
};

namespace chip {
namespace MatterCommissioningTool {

FirebaseOperationalCredentialsIssuer::FirebaseOperationalCredentialsIssuer()
{
    // Initialize with M2 app fabric and CAT values
    mNextRequestedFabricId       = kM2FabricId;
    mNextRequestedCATs.values[0] = kM2CATSubject;
    mNextRequestedCATs.values[1] = chip::kUndefinedCAT;
    mNextRequestedCATs.values[2] = chip::kUndefinedCAT;
}

FirebaseOperationalCredentialsIssuer::~FirebaseOperationalCredentialsIssuer() {}

CHIP_ERROR FirebaseOperationalCredentialsIssuer::Initialize(chip::PersistentStorageDelegate & storage)
{
    // Initialize the ICA keypair
    ReturnErrorOnFailure(mICAKeypair.Initialize(ECPKeyTarget::ECDSA));

    ChipLogProgress(chipTool, "Firebase operational credentials issuer initialized");
    return CHIP_NO_ERROR;
}

void FirebaseOperationalCredentialsIssuer::SetFirebaseCertificates(const chip::ByteSpan & rcaCert, const chip::ByteSpan & icaCert)
{
    mRootCACert.assign(rcaCert.data(), rcaCert.data() + rcaCert.size());
    mICACert.assign(icaCert.data(), icaCert.data() + icaCert.size());
    mCertificatesSet = true;
    mICAKeyExtracted = false; // Reset key extraction flag

    ChipLogProgress(chipTool, "Firebase certificates set: RCA=%zu bytes, ICA=%zu bytes", mRootCACert.size(), mICACert.size());
}

void FirebaseOperationalCredentialsIssuer::SetICAKeypairReference(chip::Crypto::P256Keypair * keypair)
{
    mExternalICAKeypair = keypair;
    if (keypair != nullptr)
    {
        mICAKeyInitialized = true;
        mICAKeyExtracted   = true;
        ChipLogProgress(chipTool, "ICA keypair reference set from Firebase client");
    }
}

CHIP_ERROR FirebaseOperationalCredentialsIssuer::ExtractICAPrivateKey()
{
    if (mICAKeyExtracted)
    {
        return CHIP_NO_ERROR;
    }

    // Prefer an external ICA keypair that was used to generate the CSR for Firebase ICA
    if (mExternalICAKeypair != nullptr)
    {
        mICAKeyExtracted = true;
        ChipLogProgress(chipTool, "ICA private key reference available (from Firebase client)");
        return CHIP_NO_ERROR;
    }

    // Do not generate a random ICA keypair here: it would not match the Firebase ICA cert
    ChipLogError(chipTool, "ICA private key not set. Call SetICAKeypairReference() before generating NOCs");
    return CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR FirebaseOperationalCredentialsIssuer::CreateMatterSubjectName(chip::NodeId nodeId, chip::FabricId fabricId,
                                                                         const chip::CATValues & cats,
                                                                         chip::MutableByteSpan & subjectName)
{
    // Create Matter-compliant subject name exactly like M2 app
    // Uses specific Matter OIDs for proper compressed fabric ID generation

    ChipDN subjectDN;

    // Add Matter Node ID (OID: 1.3.6.1.4.1.37244.1.1)
    ReturnErrorOnFailure(subjectDN.AddAttribute_MatterNodeId(nodeId));

    // Add Matter Fabric ID (OID: 1.3.6.1.4.1.37244.1.5)
    ReturnErrorOnFailure(subjectDN.AddAttribute_MatterFabricId(fabricId));

    // Add CAT values (OID: 1.3.6.1.4.1.37244.1.6)
    ReturnErrorOnFailure(subjectDN.AddCATs(cats));

    ChipLogProgress(chipTool, "Created Matter subject DN: NodeId=0x%016llx, FabricId=0x%016llx, CAT=0x%08x",
                    static_cast<unsigned long long>(nodeId), static_cast<unsigned long long>(fabricId), cats.values[0]);

    // ChipDN doesn't need to be encoded to TLV for certificate generation
    // It's used directly in X509CertRequestParams
    return CHIP_NO_ERROR;
}

CHIP_ERROR FirebaseOperationalCredentialsIssuer::GenerateDeviceNOC(const chip::Crypto::P256PublicKey & devicePublicKey,
                                                                   chip::MutableByteSpan & noc)
{
    VerifyOrReturnError(mCertificatesSet, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(ExtractICAPrivateKey());

    // Use the ICA from Firebase as issuer and the exact ICA private key used for the CSR
    ChipDN issuerDN;
    ReturnErrorOnFailure(ExtractSubjectDNFromX509Cert(ByteSpan(mICACert.data(), mICACert.size()), issuerDN));

    // Use M2 fabric ID and CAT values
    ChipDN subjectDN;
    ReturnErrorOnFailure(subjectDN.AddAttribute_MatterNodeId(mNextRequestedNodeId));
    ReturnErrorOnFailure(subjectDN.AddAttribute_MatterFabricId(kM2FabricId));

    CATValues m2CATValues;
    m2CATValues.values[0] = kM2CATSubject;
    m2CATValues.values[1] = chip::kUndefinedCAT;
    m2CATValues.values[2] = chip::kUndefinedCAT;
    ReturnErrorOnFailure(subjectDN.AddCATs(m2CATValues));

    // Generate random serial number using DRBG
    uint32_t serialNumber;
    ReturnErrorOnFailure(chip::Crypto::DRBG_get_bytes(reinterpret_cast<uint8_t *>(&serialNumber), sizeof(serialNumber)));

    // Use a wide validity window relative to CHIP epoch (Y2K): [2000-01-01, 2000-01-01 + 40y)
    // NOTE: Use 0 for NotBefore to avoid relying on wall clock; controller validates against LKG time.
    uint32_t notBeforeSeconds        = 0;                             // 2000-01-01 (CHIP epoch)
    uint32_t validity                = kCertificateValidityPeriodSec; // 40 years
    X509CertRequestParams certParams = { serialNumber, notBeforeSeconds, notBeforeSeconds + validity, subjectDN, issuerDN };

    // Generate the NOC certificate in DER format first
    Platform::ScopedMemoryBuffer<uint8_t> derBuffer;
    VerifyOrReturnError(derBuffer.Alloc(kMaxM2DERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan derSpan(derBuffer.Get(), kMaxM2DERCertLength);

    // Sign with the external ICA keypair (must match Firebase ICA cert)
    VerifyOrReturnError(mExternalICAKeypair != nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(NewNodeOperationalX509Cert(certParams, devicePublicKey, *mExternalICAKeypair, derSpan));

    // Return DER certificate (commissioner converts DER->CHIP as needed)
    VerifyOrReturnError(noc.size() >= derSpan.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(noc.data(), derSpan.data(), derSpan.size());
    noc.reduce_size(derSpan.size());

    ChipLogProgress(chipTool, "Generated device NOC (DER): %zu bytes for node 0x%016llx, fabric 0x%016llx, CAT 0x%08x",
                    derSpan.size(), static_cast<unsigned long long>(mNextRequestedNodeId),
                    static_cast<unsigned long long>(kM2FabricId), kM2CATSubject);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FirebaseOperationalCredentialsIssuer::GenerateNOCChain(
    const chip::ByteSpan & csrElements, const chip::ByteSpan & csrNonce, const chip::ByteSpan & attestationSignature,
    const chip::ByteSpan & attestationChallenge, const chip::ByteSpan & DAC, const chip::ByteSpan & PAI,
    chip::Callback::Callback<chip::Controller::OnNOCChainGeneration> * onCompletion)
{
    VerifyOrReturnError(onCompletion != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mCertificatesSet, CHIP_ERROR_INCORRECT_STATE);

    // Extract the actual CSR from the TLV-wrapped csrElements
    TLV::TLVReader reader;
    reader.Init(csrElements);
    ReturnErrorOnFailure(reader.Next(kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType containerType;
    ReturnErrorOnFailure(reader.EnterContainer(containerType));
    ReturnErrorOnFailure(reader.Next(kTLVType_ByteString, TLV::ContextTag(1)));

    ByteSpan csr(reader.GetReadPoint(), reader.GetLength());
    reader.ExitContainer(containerType);

    // Extract the public key from the actual DER-encoded CSR
    P256PublicKey devicePublicKey;
    ReturnErrorOnFailure(VerifyCertificateSigningRequest(csr.data(), csr.size(), devicePublicKey));

    // Generate the device NOC
    Platform::ScopedMemoryBuffer<uint8_t> nocBuffer;
    VerifyOrReturnError(nocBuffer.Alloc(kMaxM2CHIPCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan nocSpan(nocBuffer.Get(), kMaxM2CHIPCertLength);

    ReturnErrorOnFailure(GenerateDeviceNOC(devicePublicKey, nocSpan));

    // Prepare the certificate chain for the callback (all in DER). Controller will:
    // - Use DER RCAC/ICAC/NOC for FabricTable validation and state
    // - Convert RCAC to CHIP TLV for AddTrustedRootCertificate (we added a conversion hook in DeviceCommissioner)
    ByteSpan rootCert(mRootCACert.data(), mRootCACert.size());
    ByteSpan icaCert(mICACert.data(), mICACert.size());
    ByteSpan nocCert = nocSpan;

    ChipLogProgress(chipTool, "Prepared chain (DER): RCAC=%zu, ICAC=%zu, NOC=%zu bytes", rootCert.size(), icaCert.size(),
                    nocCert.size());

    // Prepare the M2 IPK and admin subject for the callback
    Crypto::IdentityProtectionKeySpan ipkSpan(kM2IPK);
    Optional<Crypto::IdentityProtectionKeySpan> ipkOptional = MakeOptional(ipkSpan);
    Optional<NodeId> adminSubjectOptional                   = MakeOptional(static_cast<NodeId>(kM2ACLCATSubject));

    // Call the completion callback with the generated certificates, IPK, and admin subject
    onCompletion->mCall(onCompletion->mContext, CHIP_NO_ERROR, nocCert, icaCert, rootCert, ipkOptional, adminSubjectOptional);

    return CHIP_NO_ERROR;
}

void FirebaseOperationalCredentialsIssuer::SetNodeIdForNextNOCRequest(chip::NodeId nodeId)
{
    mNextRequestedNodeId = nodeId;
    ChipLogProgress(chipTool, "Set next NOC request node ID: 0x%016llx", static_cast<unsigned long long>(nodeId));
}

void FirebaseOperationalCredentialsIssuer::SetFabricIdForNextNOCRequest(chip::FabricId fabricId)
{
    mNextRequestedFabricId = fabricId;
    ChipLogProgress(chipTool, "Set next NOC request fabric ID: 0x%016llx", static_cast<unsigned long long>(fabricId));
}

void FirebaseOperationalCredentialsIssuer::SetCATValuesForNextNOCRequest(chip::CATValues cats)
{
    mNextRequestedCATs = cats;
    ChipLogProgress(chipTool, "Set next NOC request CAT values");
}

CHIP_ERROR FirebaseOperationalCredentialsIssuer::GenerateControllerNOCChain(
    chip::NodeId nodeId, chip::FabricId fabricId, const chip::CATValues & cats, chip::Crypto::P256Keypair & keypair,
    chip::MutableByteSpan & rcac, chip::MutableByteSpan & icac, chip::MutableByteSpan & noc)
{
    VerifyOrReturnError(mCertificatesSet, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mExternalICAKeypair != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // Use the Firebase-provided RCAC/ICAC directly; do not generate new ones
    VerifyOrReturnError(rcac.size() >= mRootCACert.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(rcac.data(), mRootCACert.data(), mRootCACert.size());
    rcac.reduce_size(mRootCACert.size());

    VerifyOrReturnError(icac.size() >= mICACert.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(icac.data(), mICACert.data(), mICACert.size());
    icac.reduce_size(mICACert.size());

    // Issuer DN is taken from the ICA cert; fall back to Root CA subject if needed (for robustness)
    ChipDN issuerDN;
    {
        CHIP_ERROR __dnErr = ExtractSubjectDNFromX509Cert(ByteSpan(mICACert.data(), mICACert.size()), issuerDN);
        if (__dnErr != CHIP_NO_ERROR)
        {
            ChipLogError(chipTool, "Failed to extract ICA subject DN (err=%" CHIP_ERROR_FORMAT "). Falling back to RCAC subject.",
                         __dnErr.Format());
            ReturnErrorOnFailure(ExtractSubjectDNFromX509Cert(ByteSpan(mRootCACert.data(), mRootCACert.size()), issuerDN));
        }
    }

    // Subject DN: M2 fabric + node + CATs
    ChipDN subjectDN;
    ReturnErrorOnFailure(subjectDN.AddAttribute_MatterFabricId(kM2FabricId));
    ReturnErrorOnFailure(subjectDN.AddAttribute_MatterNodeId(nodeId));

    CATValues m2CATValues;
    m2CATValues.values[0] = kM2CATSubject;
    m2CATValues.values[1] = chip::kUndefinedCAT;
    m2CATValues.values[2] = chip::kUndefinedCAT;
    ReturnErrorOnFailure(subjectDN.AddCATs(m2CATValues));

    // Serial/time
    uint32_t serialNumber;
    ReturnErrorOnFailure(chip::Crypto::DRBG_get_bytes(reinterpret_cast<uint8_t *>(&serialNumber), sizeof(serialNumber)));
    uint32_t validity = kCertificateValidityPeriodSec; // 40 years

    // Use CHIP epoch-relative time window to avoid wall-clock dependency
    uint32_t notBefore               = 0; // 2000-01-01
    X509CertRequestParams nocRequest = { serialNumber, notBefore, notBefore + validity, subjectDN, issuerDN };

    // Sign controller NOC with the exact ICA keypair that Firebase signed
    ReturnErrorOnFailure(NewNodeOperationalX509Cert(nocRequest, keypair.Pubkey(), *mExternalICAKeypair, noc));

    // Compute and log compressed fabric ID using RCAC pubkey + fabric ID from NOC
    // Convert RCAC DER -> CHIP cert
    uint8_t rcacChipBuf[kMaxM2CHIPCertLength];
    MutableByteSpan rcacChipSpan(rcacChipBuf, sizeof(rcacChipBuf));
    ReturnErrorOnFailure(ConvertX509CertToChipCert(ByteSpan(rcac.data(), rcac.size()), rcacChipSpan));

    // Convert generated controller NOC DER -> CHIP cert
    uint8_t nocChipBuf[kMaxM2CHIPCertLength];
    MutableByteSpan nocChipSpan(nocChipBuf, sizeof(nocChipBuf));
    ReturnErrorOnFailure(ConvertX509CertToChipCert(noc, nocChipSpan));

    CompressedFabricId compressedId = 0;
    FabricId extractedFabricId      = 0;
    NodeId extractedNodeId          = 0;
    ReturnErrorOnFailure(ExtractNodeIdFabricIdCompressedFabricIdFromOpCerts(rcacChipSpan, nocChipSpan, compressedId,
                                                                            extractedFabricId, extractedNodeId));
    ChipLogProgress(chipTool, "Compressed Fabric ID (from Firebase RCAC + NOC) = 0x%016" PRIx64,
                    static_cast<uint64_t>(compressedId));

    ChipLogProgress(chipTool, "Generated controller NOC chain using Firebase RCAC/ICAC: RCA=%zu, ICA=%zu, NOC=%zu bytes",
                    rcac.size(), icac.size(), noc.size());

    return CHIP_NO_ERROR;
}

} // namespace MatterCommissioningTool
} // namespace chip
