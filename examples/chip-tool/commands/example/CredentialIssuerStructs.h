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

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace chip {
namespace MatterCommissioningTool {

struct AuthResult
{
    std::string accessToken;  // Used by Supabase
    std::string idToken;      // Used by Firebase
    std::string refreshToken;
    bool success = false;
    std::string errorMessage;

    AuthResult() = default;
    bool IsValid() const { return success && (!accessToken.empty() || !idToken.empty()); }
};

struct CertificateBundle
{
    std::vector<uint8_t> dacCert;
    std::vector<uint8_t> paiCert;
    std::vector<uint8_t> certDeclaration;
    std::vector<uint8_t> rootCaCert;
    std::vector<uint8_t> icaCert; // ICA certificate for signing device NOCs
    std::vector<uint8_t> firmwareInfo;
    bool success = false;
    std::string errorMessage;

    CertificateBundle() = default;
    bool IsValid() const { return success && !dacCert.empty() && !paiCert.empty() && !certDeclaration.empty(); }
    void LogCertificateInfo() const;
    void Clear();
};

} // namespace MatterCommissioningTool
} // namespace chip
