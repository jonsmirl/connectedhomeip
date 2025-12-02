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

#pragma once

#include <commands/common/CredentialIssuerCommands.h>
#include <controller/CHIPDeviceControllerFactory.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <string>

#include "SupabaseClient.h"
#include "SupabaseOperationalCredentialsIssuer.h"

class SupabaseCredentialIssuerCommands : public CredentialIssuerCommands
{
public:
    SupabaseCredentialIssuerCommands(const std::string & supabaseUrl = "https://vmhzoaoyvxfdlubxnudv.supabase.co",
                                     const std::string & anonKey = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InZtaHpvYW95dnhmZGx1Ynh"
                                                                   "udWR2Iiwicm9sZSI6ImFub24iLCJpYXQiOjE3NjAyNzU4NDIsImV4cCI6MjA3NTg1MTg0Mn0.a2NjscBPMR_vxYnIGkiF7txQGP6WESyYATf9YbA1CTc");
    ~SupabaseCredentialIssuerCommands();

    CHIP_ERROR InitializeCredentialsIssuer(chip::PersistentStorageDelegate & storage) override;
    CHIP_ERROR SetupDeviceAttestation(chip::Controller::SetupParams & setupParams,
                                      const chip::Credentials::AttestationTrustStore * trustStore,
                                      chip::Credentials::DeviceAttestationRevocationDelegate * revocationDelegate) override;

    chip::Controller::OperationalCredentialsDelegate * GetCredentialIssuer() override;
    void SetCredentialIssuerCATValues(chip::CATValues cats) override;

    CHIP_ERROR GenerateControllerNOCChain(chip::NodeId nodeId, chip::FabricId fabricId, const chip::CATValues & cats,
                                          chip::Crypto::P256Keypair & keypair, chip::MutableByteSpan & rcac,
                                          chip::MutableByteSpan & icac, chip::MutableByteSpan & noc) override;

    CHIP_ERROR AddAdditionalCDVerifyingCerts(const std::vector<std::vector<uint8_t>> & additionalCdCerts) override;

    void SetCredentialIssuerOption(CredentialIssuerOptions option, bool isEnabled) override;
    bool GetCredentialIssuerOption(CredentialIssuerOptions option) override;

private:
    CHIP_ERROR InitializeSupabaseClient();
    CHIP_ERROR GetCertificatesFromSupabase(chip::MutableByteSpan & rcac, chip::MutableByteSpan & icac);

    std::unique_ptr<chip::MatterCommissioningTool::SupabaseClient> mSupabaseClient;
    chip::MatterCommissioningTool::SupabaseOperationalCredentialsIssuer mSupabaseOpCredsIssuer;
    chip::Credentials::DeviceAttestationVerifier * mDacVerifier;

    std::string mSupabaseUrl;
    std::string mAnonKey;
    std::string mAccessToken;

    bool mUsesMaxSizedCerts     = false;
    bool mAllowTestCdSigningKey = true;
    bool mSupabaseInitialized   = false;
};
