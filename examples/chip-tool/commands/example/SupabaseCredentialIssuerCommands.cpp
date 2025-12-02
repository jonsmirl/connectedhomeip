/*
 *   Copyright (c) 2025 Project CHIP Authors
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

#include "SupabaseCredentialIssuerCommands.h"
#include "SupabaseClient.h"

#include <lib/support/logging/CHIPLogging.h>

SupabaseCredentialIssuerCommands::SupabaseCredentialIssuerCommands(const std::string & supabaseUrl,
                                                                   const std::string & anonKey) :
    mDacVerifier(nullptr), mSupabaseUrl(supabaseUrl), mAnonKey(anonKey)
{}

SupabaseCredentialIssuerCommands::~SupabaseCredentialIssuerCommands() {}

CHIP_ERROR SupabaseCredentialIssuerCommands::InitializeCredentialsIssuer(chip::PersistentStorageDelegate & storage)
{
    // Initialize the Supabase operational credentials issuer
    ReturnErrorOnFailure(mSupabaseOpCredsIssuer.Initialize(storage));

    // Initialize Supabase client
    ReturnErrorOnFailure(InitializeSupabaseClient());

    return CHIP_NO_ERROR;
}

CHIP_ERROR SupabaseCredentialIssuerCommands::InitializeSupabaseClient()
{
    if (mSupabaseInitialized)
    {
        return CHIP_NO_ERROR;
    }

    ChipLogProgress(chipTool, "Initializing Supabase client: %s", mSupabaseUrl.c_str());

    mSupabaseClient = std::make_unique<chip::MatterCommissioningTool::SupabaseClient>();

    // Initialize the client
    ReturnErrorOnFailure(mSupabaseClient->Initialize(mSupabaseUrl, mAnonKey));

    // Authenticate with Supabase
    auto authResult = mSupabaseClient->Authenticate("test@lowpan.com", "testme");
    if (!authResult.success)
    {
        ChipLogError(chipTool, "Supabase authentication failed: %s", authResult.errorMessage.c_str());
        return CHIP_ERROR_ACCESS_DENIED;
    }

    // Store the access token for later use
    mAccessToken = authResult.accessToken;

    mSupabaseInitialized = true;
    ChipLogProgress(chipTool, "Supabase client initialized successfully");

    return CHIP_NO_ERROR;
}

CHIP_ERROR SupabaseCredentialIssuerCommands::SetupDeviceAttestation(
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

chip::Controller::OperationalCredentialsDelegate * SupabaseCredentialIssuerCommands::GetCredentialIssuer()
{
    return &mSupabaseOpCredsIssuer;
}

void SupabaseCredentialIssuerCommands::SetCredentialIssuerCATValues(chip::CATValues cats)
{
    mSupabaseOpCredsIssuer.SetCATValuesForNextNOCRequest(cats);
}

CHIP_ERROR SupabaseCredentialIssuerCommands::GenerateControllerNOCChain(chip::NodeId nodeId, chip::FabricId fabricId,
                                                                        const chip::CATValues & cats,
                                                                        chip::Crypto::P256Keypair & keypair,
                                                                        chip::MutableByteSpan & rcac, chip::MutableByteSpan & icac,
                                                                        chip::MutableByteSpan & noc)
{
    // Get certificates from Supabase and set them in the issuer
    ReturnErrorOnFailure(GetCertificatesFromSupabase(rcac, icac));

    // Set the Supabase certificates in our custom issuer
    mSupabaseOpCredsIssuer.SetSupabaseCertificates(rcac, icac);
    // Provide the ICA keypair reference used to generate the CSR so NOCs are signed with matching key
    mSupabaseOpCredsIssuer.SetICAKeypairReference(mSupabaseClient->GetICAKeypairPtr());

    // Use the Supabase operational credentials issuer to generate the NOC chain
    return mSupabaseOpCredsIssuer.GenerateControllerNOCChain(nodeId, fabricId, cats, keypair, rcac, icac, noc);
}

CHIP_ERROR SupabaseCredentialIssuerCommands::GetCertificatesFromSupabase(chip::MutableByteSpan & rcac, chip::MutableByteSpan & icac)
{
    if (!mSupabaseInitialized)
    {
        ReturnErrorOnFailure(InitializeSupabaseClient());
    }

    ChipLogProgress(chipTool, "Retrieving certificates from Supabase");

    auto certBundle = mSupabaseClient->GetCertificates(mAccessToken);
    if (!certBundle.IsValid())
    {
        ChipLogError(chipTool, "Failed to retrieve valid certificates from Supabase: %s", certBundle.errorMessage.c_str());
        return CHIP_ERROR_INTERNAL;
    }

    ChipLogProgress(chipTool, "Successfully retrieved certificates from Supabase");
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
SupabaseCredentialIssuerCommands::AddAdditionalCDVerifyingCerts(const std::vector<std::vector<uint8_t>> & additionalCdCerts)
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

void SupabaseCredentialIssuerCommands::SetCredentialIssuerOption(CredentialIssuerOptions option, bool isEnabled)
{
    switch (option)
    {
    case CredentialIssuerOptions::kMaximizeCertificateSizes:
        mUsesMaxSizedCerts = isEnabled;
        // Note: Supabase operational credentials issuer doesn't support maximally large certs
        // This is a no-op for Supabase issuer
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

bool SupabaseCredentialIssuerCommands::GetCredentialIssuerOption(CredentialIssuerOptions option)
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
