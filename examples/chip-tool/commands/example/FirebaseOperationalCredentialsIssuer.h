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

#pragma once

#include <controller/OperationalCredentialsDelegate.h>
#include <credentials/CertificationDeclaration.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/Span.h>

#include <memory>
#include <vector>

namespace chip {
namespace MatterCommissioningTool {

// Custom operational credentials issuer that uses Firebase ICA to sign device certificates locally
// This mimics the behavior of the M2 Android app's certificate generation
class FirebaseOperationalCredentialsIssuer : public chip::Controller::OperationalCredentialsDelegate
{
public:
    FirebaseOperationalCredentialsIssuer();
    virtual ~FirebaseOperationalCredentialsIssuer();

    CHIP_ERROR Initialize(chip::PersistentStorageDelegate & storage);
    void SetFirebaseCertificates(const chip::ByteSpan & rcaCert, const chip::ByteSpan & icaCert);
    // Provide ICA private key generated during CSR so signatures match the ICA cert from Firebase
    void SetICAKeypairReference(chip::Crypto::P256Keypair * keypair);

    // OperationalCredentialsDelegate implementation
    CHIP_ERROR GenerateNOCChain(const chip::ByteSpan & csrElements, const chip::ByteSpan & csrNonce,
                                const chip::ByteSpan & attestationSignature, const chip::ByteSpan & attestationChallenge,
                                const chip::ByteSpan & DAC, const chip::ByteSpan & PAI,
                                chip::Callback::Callback<chip::Controller::OnNOCChainGeneration> * onCompletion) override;

    void SetNodeIdForNextNOCRequest(chip::NodeId nodeId) override;
    void SetFabricIdForNextNOCRequest(chip::FabricId fabricId) override;
    void SetCATValuesForNextNOCRequest(chip::CATValues cats);

    // Additional methods for Firebase integration
    CHIP_ERROR GenerateControllerNOCChain(chip::NodeId nodeId, chip::FabricId fabricId, const chip::CATValues & cats,
                                          chip::Crypto::P256Keypair & keypair, chip::MutableByteSpan & rcac,
                                          chip::MutableByteSpan & icac, chip::MutableByteSpan & noc);

private:
    CHIP_ERROR GenerateDeviceNOC(const chip::Crypto::P256PublicKey & devicePublicKey, chip::MutableByteSpan & noc);
    CHIP_ERROR ExtractICAPrivateKey();
    CHIP_ERROR CreateMatterSubjectName(chip::NodeId nodeId, chip::FabricId fabricId, const chip::CATValues & cats,
                                       chip::MutableByteSpan & subjectName);

    chip::NodeId mNextRequestedNodeId     = 1;
    chip::FabricId mNextRequestedFabricId = 1;
    chip::CATValues mNextRequestedCATs;

    // Firebase certificates (kept for compatibility)
    std::vector<uint8_t> mRootCACert;
    std::vector<uint8_t> mICACert;
    bool mCertificatesSet = false;
    bool mICAKeyExtracted = false;

    // Certificate generation keypairs
    chip::Crypto::P256Keypair mRootCAKeypair;
    // Optional external ICA keypair reference provided by Firebase client (preferred)
    chip::Crypto::P256Keypair * mExternalICAKeypair = nullptr;

    chip::Crypto::P256Keypair mICAKeypair;
    bool mRootCAKeyInitialized = false;
    bool mICAKeyInitialized    = false;

    // Matter fabric constants (matching M2 app exactly)
    static constexpr chip::FabricId kM2FabricId = 0xFAB000000000001DULL;
    static constexpr uint32_t kM2CATSubject     = 0xABCD0001UL;          // CAT subject from M2 app
    static constexpr uint64_t kM2ACLCATSubject  = 0xFFFFFFFDABCD0001ULL; // ACL CAT subject with prefix
};

} // namespace MatterCommissioningTool
} // namespace chip
