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

#include "CredentialIssuerStructs.h"
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/CHIPMem.h>

#include <string>
#include <vector>

namespace chip {
namespace MatterCommissioningTool {

/**
 * @brief Supabase HTTP client for certificate retrieval
 *
 * Handles authentication and certificate retrieval from Supabase
 * using Edge Functions and Supabase Auth.
 */
class SupabaseClient
{
public:
    SupabaseClient()  = default;
    ~SupabaseClient() = default;

    /**
     * @brief Initialize the Supabase client
     * @param supabaseUrl Supabase project URL (e.g., "https://vmhzoaoyvxfdlubxnudv.supabase.co")
     * @param anonKey Supabase anonymous key for API access
     */
    CHIP_ERROR Initialize(const std::string & supabaseUrl, const std::string & anonKey);

    /**
     * @brief Authenticate with Supabase Auth
     * @param email User email for authentication
     * @param password User password for authentication
     * @return Authentication result with access token
     */
    AuthResult Authenticate(const std::string & email, const std::string & password);

    /**
     * @brief Retrieve certificate bundle from Supabase Edge Functions
     * @param accessToken Authentication token from Authenticate()
     * @return Certificate bundle with Root CA and ICA
     */
    CertificateBundle GetCertificates(const std::string & accessToken);
    CertificateBundle AccessHome(const std::string & accessToken, const std::string & homeId, const std::string & csrData);
    std::vector<std::pair<std::string, std::string>> ListHomes(const std::string & accessToken);

    /**
     * @brief Test connectivity to Supabase
     */
    bool TestConnectivity();

    /**
     * @brief Test CSR generation functionality
     */
    bool TestCSRGeneration();

    /**
     * @brief Get the current configuration
     */
    std::string GetSupabaseUrl() const { return mSupabaseUrl; }
    std::string GetAnonKey() const { return mAnonKey; }

    /**
     * @brief Set the preferred home name to use for certificate retrieval
     * @param homeName Name of the home to prioritize (e.g., "Florida")
     */
    void SetPreferredHomeName(const std::string & homeName) { mPreferredHomeName = homeName; }
    std::string GetPreferredHomeName() const { return mPreferredHomeName; }

    // Expose the ICA keypair used for the CSR so the issuer can sign with the same key
    chip::Crypto::P256Keypair * GetICAKeypairPtr() { return &mICAKeypair; }

    chip::Crypto::P256Keypair mICAKeypair;
    bool mICAKeypairInitialized = false;

private:
    std::string mSupabaseUrl;
    std::string mAnonKey;
    std::string mPreferredHomeName = "Florida";  // Default to Florida for backwards compatibility
    bool mInitialized = false;

    // HTTP client methods
    std::string MakeHttpRequest(const std::string & url, const std::string & method, const std::string & headers,
                                const std::string & body);

    std::string BuildAuthUrl() const;
    std::string BuildAccessHomeUrl() const;
    std::string BuildListHomesUrl() const;

    // JSON parsing helpers
    bool ParseAuthResponse(const std::string & response, AuthResult & result);
    bool ParseCertificateResponse(const std::string & response, CertificateBundle & bundle);
    bool ParseAccessHomeResponse(const std::string & response, CertificateBundle & bundle);

    // Certificate conversion helpers
    std::vector<uint8_t> Base64Decode(const std::string & encoded);
    bool ValidateCertificateFormat(const std::vector<uint8_t> & cert);
    bool ParseCertificateField(const std::string & response, const std::string & fieldName, std::vector<uint8_t> & output);

    // CSR generation
    std::string GenerateRealCSR();
    std::string Base64EncodeCSR(const uint8_t * csrData, size_t csrLength);

    // Mock certificate generators for testing
    std::vector<uint8_t> CreateMockDACCertificate();
    std::vector<uint8_t> CreateMockPAICertificate();
    std::vector<uint8_t> CreateMockCertificationDeclaration();
    std::vector<uint8_t> CreateMockRootCACertificate();
    std::vector<uint8_t> CreateMockFirmwareInfo();
    std::vector<uint8_t> DecodeBase64Certificate(const std::string & base64Cert);
};

} // namespace MatterCommissioningTool
} // namespace chip
