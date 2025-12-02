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

#include <commands/common/CredentialIssuerCommands.h>
#include <controller/CHIPDeviceControllerFactory.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <string>

#include "FirebaseEmulatorClient.h"
#include "FirebaseOperationalCredentialsIssuer.h"

class FirebaseCredentialIssuerCommands : public CredentialIssuerCommands
{
public:
    FirebaseCredentialIssuerCommands(const std::string & firebaseHost = "192.168.1.2", uint16_t authPort = 9099,
                                     uint16_t funcPort = 5001);
    ~FirebaseCredentialIssuerCommands();

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
    CHIP_ERROR InitializeFirebaseClient();
    CHIP_ERROR GetCertificatesFromFirebase(chip::MutableByteSpan & rcac, chip::MutableByteSpan & icac);

    std::unique_ptr<chip::MatterCommissioningTool::FirebaseEmulatorClient> mFirebaseClient;
    chip::MatterCommissioningTool::FirebaseOperationalCredentialsIssuer mFirebaseOpCredsIssuer;
    chip::Credentials::DeviceAttestationVerifier * mDacVerifier;

    std::string mFirebaseHost;
    uint16_t mAuthPort;
    uint16_t mFuncPort;
    std::string mIdToken;

    bool mUsesMaxSizedCerts     = false;
    bool mAllowTestCdSigningKey = true;
    bool mFirebaseInitialized   = false;
};
