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

#include "FirebaseCredentialIssuerCommands.h"
#include "FirebaseEmulatorClient.h"

#include <lib/support/logging/CHIPLogging.h>

FirebaseCredentialIssuerCommands::FirebaseCredentialIssuerCommands(const std::string & firebaseHost, uint16_t authPort,
                                                                   uint16_t funcPort) :
    mDacVerifier(nullptr), mFirebaseHost(firebaseHost), mAuthPort(authPort), mFuncPort(funcPort)
{}

FirebaseCredentialIssuerCommands::~FirebaseCredentialIssuerCommands() {}

CHIP_ERROR FirebaseCredentialIssuerCommands::InitializeCredentialsIssuer(chip::PersistentStorageDelegate & storage)
{
    // Initialize the Firebase operational credentials issuer
    ReturnErrorOnFailure(mFirebaseOpCredsIssuer.Initialize(storage));

    // Initialize Firebase client
    ReturnErrorOnFailure(InitializeFirebaseClient());

    return CHIP_NO_ERROR;
}

CHIP_ERROR FirebaseCredentialIssuerCommands::InitializeFirebaseClient()
{
    if (mFirebaseInitialized)
    {
        return CHIP_NO_ERROR;
    }

    ChipLogProgress(chipTool, "Initializing Firebase client: %s:%d/%d", mFirebaseHost.c_str(), mAuthPort, mFuncPort);

    mFirebaseClient = std::make_unique<chip::MatterCommissioningTool::FirebaseEmulatorClient>();

    // Initialize the client
    ReturnErrorOnFailure(mFirebaseClient->Initialize(mFirebaseHost, mAuthPort, mFuncPort));

    // Authenticate with Firebase
    auto authResult = mFirebaseClient->Authenticate("test@example.com", "testme");
    if (!authResult.success)
    {
        ChipLogError(chipTool, "Firebase authentication failed: %s", authResult.errorMessage.c_str());
        return CHIP_ERROR_ACCESS_DENIED;
    }

    // Store the ID token for later use
    mIdToken = authResult.idToken;

    mFirebaseInitialized = true;
    ChipLogProgress(chipTool, "Firebase client initialized successfully");

    return CHIP_NO_ERROR;
}

CHIP_ERROR FirebaseCredentialIssuerCommands::SetupDeviceAttestation(
    chip::Controller::SetupParams & setupParams, const chip::Credentials::AttestationTrustStore * trustStore,
    chip::Credentials::DeviceAttestationRevocationDelegate * revocationDelegate)
{
    chip::Credentials::SetDeviceAttestationCredentialsProvider(chip::Credentials::Examples::GetExampleDACProvider());

    mDacVerifier = chip::Credentials::GetDefaultDACVerifier(trustStore, revocationDelegate);
    VerifyOrDie(mDacVerifier != nullptr);

    mDacVerifier->EnableCdTestKeySupport(mAllowTestCdSigningKey);
    mDacVerifier->EnableVerboseLogs(true);
    setupParams.deviceAttestationVerifier = mDacVerifier;

    return CHIP_NO_ERROR;
}

chip::Controller::OperationalCredentialsDelegate * FirebaseCredentialIssuerCommands::GetCredentialIssuer()
{
    return &mFirebaseOpCredsIssuer;
}

void FirebaseCredentialIssuerCommands::SetCredentialIssuerCATValues(chip::CATValues cats)
{
    mFirebaseOpCredsIssuer.SetCATValuesForNextNOCRequest(cats);
}

CHIP_ERROR FirebaseCredentialIssuerCommands::GenerateControllerNOCChain(chip::NodeId nodeId, chip::FabricId fabricId,
                                                                        const chip::CATValues & cats,
                                                                        chip::Crypto::P256Keypair & keypair,
                                                                        chip::MutableByteSpan & rcac, chip::MutableByteSpan & icac,
                                                                        chip::MutableByteSpan & noc)
{
    // Get certificates from Firebase and set them in the issuer
    ReturnErrorOnFailure(GetCertificatesFromFirebase(rcac, icac));

    // Set the Firebase certificates in our custom issuer
    mFirebaseOpCredsIssuer.SetFirebaseCertificates(rcac, icac);
    // Provide the ICA keypair reference used to generate the CSR so NOCs are signed with matching key
    mFirebaseOpCredsIssuer.SetICAKeypairReference(mFirebaseClient->GetICAKeypairPtr());

    // Use the Firebase operational credentials issuer to generate the NOC chain
    return mFirebaseOpCredsIssuer.GenerateControllerNOCChain(nodeId, fabricId, cats, keypair, rcac, icac, noc);
}

CHIP_ERROR FirebaseCredentialIssuerCommands::GetCertificatesFromFirebase(chip::MutableByteSpan & rcac, chip::MutableByteSpan & icac)
{
    if (!mFirebaseInitialized)
    {
        ReturnErrorOnFailure(InitializeFirebaseClient());
    }

    ChipLogProgress(chipTool, "Retrieving certificates from Firebase");

    auto certBundle = mFirebaseClient->GetCertificates(mIdToken);
    if (!certBundle.IsValid())
    {
        ChipLogError(chipTool, "Failed to retrieve valid certificates from Firebase: %s", certBundle.errorMessage.c_str());
        return CHIP_ERROR_INTERNAL;
    }

    ChipLogProgress(chipTool, "Successfully retrieved certificates from Firebase");
    ChipLogProgress(chipTool, "Root CA length: %zu", certBundle.rootCaCert.size());
    ChipLogProgress(chipTool, "ICA length: %zu", certBundle.icaCert.size());

    // Copy Root CA certificate
    if (!certBundle.rootCaCert.empty())
    {
        if (certBundle.rootCaCert.size() > rcac.size())
        {
            ChipLogError(chipTool, "Root CA certificate too large: %zu > %zu", certBundle.rootCaCert.size(), rcac.size());
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
        memcpy(rcac.data(), certBundle.rootCaCert.data(), certBundle.rootCaCert.size());
        rcac.reduce_size(certBundle.rootCaCert.size());
        ChipLogProgress(chipTool, "Copied Root CA certificate: %zu bytes", certBundle.rootCaCert.size());
    }

    // Copy ICA certificate
    if (!certBundle.icaCert.empty())
    {
        if (certBundle.icaCert.size() > icac.size())
        {
            ChipLogError(chipTool, "ICA certificate too large: %zu > %zu", certBundle.icaCert.size(), icac.size());
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
        memcpy(icac.data(), certBundle.icaCert.data(), certBundle.icaCert.size());
        icac.reduce_size(certBundle.icaCert.size());
        ChipLogProgress(chipTool, "Copied ICA certificate: %zu bytes", certBundle.icaCert.size());
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
FirebaseCredentialIssuerCommands::AddAdditionalCDVerifyingCerts(const std::vector<std::vector<uint8_t>> & additionalCdCerts)
{
    VerifyOrReturnError(mDacVerifier != nullptr, CHIP_ERROR_INCORRECT_STATE);

    for (const auto & cert : additionalCdCerts)
    {
        auto cdTrustStore = mDacVerifier->GetCertificationDeclarationTrustStore();
        VerifyOrReturnError(cdTrustStore != nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorOnFailure(cdTrustStore->AddTrustedKey(chip::ByteSpan(cert.data(), cert.size())));
    }

    return CHIP_NO_ERROR;
}

void FirebaseCredentialIssuerCommands::SetCredentialIssuerOption(CredentialIssuerOptions option, bool isEnabled)
{
    switch (option)
    {
    case CredentialIssuerOptions::kMaximizeCertificateSizes:
        mUsesMaxSizedCerts = isEnabled;
        // Note: Firebase operational credentials issuer doesn't support maximally large certs
        // This is a no-op for Firebase issuer
        break;
    case CredentialIssuerOptions::kAllowTestCdSigningKey:
        mAllowTestCdSigningKey = isEnabled;
        if (mDacVerifier != nullptr)
        {
            mDacVerifier->EnableCdTestKeySupport(isEnabled);
        }
        break;
    default:
        break;
    }
}

bool FirebaseCredentialIssuerCommands::GetCredentialIssuerOption(CredentialIssuerOptions option)
{
    switch (option)
    {
    case CredentialIssuerOptions::kMaximizeCertificateSizes:
        return mUsesMaxSizedCerts;
    case CredentialIssuerOptions::kAllowTestCdSigningKey:
        return mAllowTestCdSigningKey;
    default:
        return false;
    }
}
