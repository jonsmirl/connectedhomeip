/*
 * Copyright (c) 2025 Project CHIP Authors
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "CredentialIssuerStructs.h"
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace MatterCommissioningTool {

// CertificateBundle implementation
void CertificateBundle::LogCertificateInfo() const
{
    if (!IsValid())
    {
        ChipLogError(AppServer, "Certificate bundle is invalid: %s", errorMessage.c_str());
        return;
    }

    ChipLogProgress(AppServer, "Certificate Bundle:");
    ChipLogProgress(AppServer, "  DAC Certificate: %zu bytes", dacCert.size());
    ChipLogProgress(AppServer, "  PAI Certificate: %zu bytes", paiCert.size());
    ChipLogProgress(AppServer, "  Certificate Declaration: %zu bytes", certDeclaration.size());
    ChipLogProgress(AppServer, "  Root CA Certificate: %zu bytes", rootCaCert.size());
    ChipLogProgress(AppServer, "  Firmware Information: %zu bytes", firmwareInfo.size());
}

void CertificateBundle::Clear()
{
    dacCert.clear();
    paiCert.clear();
    certDeclaration.clear();
    rootCaCert.clear();
    firmwareInfo.clear();
    success = false;
    errorMessage.clear();
}

} // namespace MatterCommissioningTool
} // namespace chip
