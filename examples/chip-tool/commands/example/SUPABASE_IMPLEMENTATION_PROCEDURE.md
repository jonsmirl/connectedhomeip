# Complete Procedure: Supabase Credential Issuer for chip-tool

## Table of Contents
1. [Overview](#overview)
2. [Architecture](#architecture)
3. [Prerequisites](#prerequisites)
4. [File Structure](#file-structure)
5. [Step-by-Step Implementation](#step-by-step-implementation)
6. [Testing Procedure](#testing-procedure)
7. [Integration with chip-tool](#integration-with-chip-tool)
8. [Troubleshooting](#troubleshooting)

---

## Overview

This document provides complete step-by-step instructions for implementing a Supabase credential issuer for chip-tool, replacing the Firebase emulator dependency with production Supabase backend services.

**Purpose**: Enable chip-tool to retrieve Matter certificates (Root CA and ICA) from Supabase Edge Functions for commissioning Matter devices.

**What We're Replacing**:
- Firebase Emulator authentication → Supabase Auth API
- Firebase Functions → Supabase Edge Functions
- Local emulator URLs → Production Supabase URLs

---

## Architecture

### Component Diagram

```
chip-tool (Matter commissioning tool)
    ├── SupabaseCredentialIssuerCommands (main entry point)
    │   ├── SupabaseClient (HTTP client for Supabase)
    │   │   ├── Authenticate with Supabase Auth
    │   │   ├── Call list-homes Edge Function
    │   │   ├── Generate CSR (Certificate Signing Request)
    │   │   └── Call access-home Edge Function
    │   └── SupabaseOperationalCredentialsIssuer (certificate management)
    │       ├── Store Root CA and ICA certificates
    │       ├── Sign device NOCs with ICA private key
    │       └── Build certificate chain
    └── Matter Device Commissioning
```

### Authentication Flow

```
1. chip-tool starts commissioning
2. SupabaseCredentialIssuerCommands::Initialize()
3. SupabaseClient::Authenticate(email, password)
   → POST https://vmhzoaoyvxfdlubxnudv.supabase.co/auth/v1/token
   → Returns: access_token
4. SupabaseClient::ListHomes(access_token)
   → POST https://vmhzoaoyvxfdlubxnudv.supabase.co/functions/v1/list-homes
   → Returns: array of homes with homeId
5. SupabaseClient::GenerateRealCSR()
   → Creates P256 keypair
   → Generates PKCS#10 CSR
   → Base64 encodes CSR
6. SupabaseClient::AccessHome(access_token, homeId, csrData)
   → POST https://vmhzoaoyvxfdlubxnudv.supabase.co/functions/v1/access-home
   → Returns: rcaCert (Root CA), icaCert (ICA signed with CSR)
7. SupabaseOperationalCredentialsIssuer stores certificates
8. Matter device commissioning proceeds with certificates
```

---

## Prerequisites

### 1. Existing Files (Templates)
Located in: `~/aosp/esp-matter/connectedhomeip/connectedhomeip/examples/chip-tool/commands/example/`

- ✅ `FirebaseCredentialIssuerCommands.h` (template)
- ✅ `FirebaseCredentialIssuerCommands.cpp` (template)
- ✅ `FirebaseEmulatorClient.h` (template)
- ✅ `FirebaseEmulatorClient.cpp` (template)
- ✅ `FirebaseOperationalCredentialsIssuer.h` (template)
- ✅ `FirebaseOperationalCredentialsIssuer.cpp` (template)
- ✅ `ExampleCredentialIssuerCommands.h` (parent class)

### 2. Supabase Configuration
- **Project URL**: `https://vmhzoaoyvxfdlubxnudv.supabase.co`
- **Anon Key**: (get from Supabase dashboard → Project Settings → API → anon public)
- **Test Account**: `test@example.com` / `testme`

### 3. Edge Functions (Already Deployed)
- ✅ `list-homes` - Lists user's homes
- ✅ `access-home` - Generates and signs certificates

### 4. Development Tools
- C++ compiler with C++17 support
- Matter SDK build environment
- curl (for HTTP requests)
- OpenSSL (for certificate handling)

---

## File Structure

### Files to Create

```
examples/chip-tool/commands/example/
├── SupabaseClient.h                           (✅ Created)
├── SupabaseClient.cpp                         (📝 To create - ~900 lines)
├── SupabaseOperationalCredentialsIssuer.h     (📝 To create - ~120 lines)
├── SupabaseOperationalCredentialsIssuer.cpp   (📝 To create - ~250 lines)
├── SupabaseCredentialIssuerCommands.h         (📝 To create - ~80 lines)
└── SupabaseCredentialIssuerCommands.cpp       (📝 To create - ~220 lines)
```

**Total**: 6 files (~1,570 lines of code)

---

## Step-by-Step Implementation

### Step 1: Create SupabaseClient.cpp

**Source Template**: `FirebaseEmulatorClient.cpp`

**Location**: `~/aosp/esp-matter/connectedhomeip/connectedhomeip/examples/chip-tool/commands/example/SupabaseClient.cpp`

#### 1.1 Copy Template and Rename

```bash
cd ~/aosp/esp-matter/connectedhomeip/connectedhomeip/examples/chip-tool/commands/example/
cp FirebaseEmulatorClient.cpp SupabaseClient.cpp
```

#### 1.2 Update Header Include (Line 18)

**Before**:
```cpp
#include "FirebaseEmulatorClient.h"
```

**After**:
```cpp
#include "SupabaseClient.h"
```

#### 1.3 Rename Class (Global Search/Replace)

**Search**: `FirebaseEmulatorClient`
**Replace**: `SupabaseClient`

**Occurrences**: ~40 instances

#### 1.4 Update Initialize Method (Lines 57-76)

**Before**:
```cpp
CHIP_ERROR FirebaseEmulatorClient::Initialize(const std::string & emulatorHost, uint16_t authPort, uint16_t functionsPort)
{
    if (emulatorHost.empty())
    {
        ChipLogError(AppServer, "Firebase emulator host cannot be empty");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    mEmulatorHost  = emulatorHost;
    mAuthPort      = authPort;
    mFunctionsPort = functionsPort;
    mInitialized   = true;

    ChipLogProgress(AppServer, "Firebase Emulator Client initialized:");
    ChipLogProgress(AppServer, "  Host: %s", mEmulatorHost.c_str());
    ChipLogProgress(AppServer, "  Auth Port: %u", mAuthPort);
    ChipLogProgress(AppServer, "  Functions Port: %u", mFunctionsPort);

    return CHIP_NO_ERROR;
}
```

**After**:
```cpp
CHIP_ERROR SupabaseClient::Initialize(const std::string & supabaseUrl, const std::string & anonKey)
{
    if (supabaseUrl.empty())
    {
        ChipLogError(AppServer, "Supabase URL cannot be empty");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (anonKey.empty())
    {
        ChipLogError(AppServer, "Supabase anon key cannot be empty");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    mSupabaseUrl = supabaseUrl;
    mAnonKey     = anonKey;
    mInitialized = true;

    ChipLogProgress(AppServer, "Supabase Client initialized:");
    ChipLogProgress(AppServer, "  URL: %s", mSupabaseUrl.c_str());
    ChipLogProgress(AppServer, "  Anon Key: %.20s...", mAnonKey.c_str());

    return CHIP_NO_ERROR;
}
```

#### 1.5 Update BuildAuthUrl Method (Lines 439-446)

**Before**:
```cpp
std::string FirebaseEmulatorClient::BuildAuthUrl() const
{
    std::ostringstream url;
    url << "http://" << mEmulatorHost << ":" << mAuthPort << "/identitytoolkit.googleapis.com/v1/accounts:signInWithPassword";
    // Add API key parameter required by Firebase emulator
    url << "?key=fake-api-key";
    return url.str();
}
```

**After**:
```cpp
std::string SupabaseClient::BuildAuthUrl() const
{
    std::ostringstream url;
    url << mSupabaseUrl << "/auth/v1/token?grant_type=password";
    return url.str();
}
```

#### 1.6 Update BuildAccessHomeUrl Method (Lines 455-460)

**Before**:
```cpp
std::string FirebaseEmulatorClient::BuildAccessHomeUrl() const
{
    std::ostringstream url;
    url << "http://" << mEmulatorHost << ":" << mFunctionsPort << "/lowpan-6b042/us-central1/accessHome?key=fake-api-key";
    return url.str();
}
```

**After**:
```cpp
std::string SupabaseClient::BuildAccessHomeUrl() const
{
    std::ostringstream url;
    url << mSupabaseUrl << "/functions/v1/access-home";
    return url.str();
}
```

#### 1.7 Update BuildListHomesUrl Method (Lines 462-467)

**Before**:
```cpp
std::string FirebaseEmulatorClient::BuildListHomesUrl() const
{
    std::ostringstream url;
    url << "http://" << mEmulatorHost << ":" << mFunctionsPort << "/lowpan-6b042/us-central1/listHomes?key=fake-api-key";
    return url.str();
}
```

**After**:
```cpp
std::string SupabaseClient::BuildListHomesUrl() const
{
    std::ostringstream url;
    url << mSupabaseUrl << "/functions/v1/list-homes";
    return url.str();
}
```

#### 1.8 Update MakeHttpRequest Method (Lines 382-437)

**Add Supabase Headers** - Modify to include apikey and proper authorization:

**Find** (line ~389):
```cpp
if (!headers.empty())
{
    // Split headers by newline and add each as a separate -H flag
    std::istringstream headerStream(headers);
    std::string header;
    while (std::getline(headerStream, header))
    {
        if (!header.empty())
        {
            curlCommand += " -H \"" + header + "\"";
        }
    }
}
```

**Add After** (before the headers processing):
```cpp
// Always add Supabase apikey header
curlCommand += " -H \"apikey: " + mAnonKey + "\"";

if (!headers.empty())
{
    // Split headers by newline and add each as a separate -H flag
    std::istringstream headerStream(headers);
    std::string header;
    while (std::getline(headerStream, header))
    {
        if (!header.empty())
        {
            curlCommand += " -H \"" + header + "\"";
        }
    }
}
```

#### 1.9 Update ParseAuthResponse Method (Lines 469-532)

**Change Token Field Name**:

**Find** (line ~476):
```cpp
size_t idTokenPos = response.find("\"idToken\"");
if (idTokenPos != std::string::npos)
{
    size_t valueStart = response.find("\"", idTokenPos + 9);
    if (valueStart != std::string::npos)
    {
        valueStart++; // Skip opening quote
        size_t valueEnd = response.find("\"", valueStart);
        if (valueEnd != std::string::npos)
        {
            result.idToken = response.substr(valueStart, valueEnd - valueStart);
        }
    }
}
```

**Replace With**:
```cpp
size_t accessTokenPos = response.find("\"access_token\"");
if (accessTokenPos != std::string::npos)
{
    size_t valueStart = response.find("\"", accessTokenPos + 14); // +14 for "access_token"
    if (valueStart != std::string::npos)
    {
        valueStart++; // Skip opening quote
        size_t valueEnd = response.find("\"", valueStart);
        if (valueEnd != std::string::npos)
        {
            result.accessToken = response.substr(valueStart, valueEnd - valueStart);
        }
    }
}
```

**Also Update** (line ~530):
```cpp
// Success if we have an access token
result.success = !result.accessToken.empty();
return result.success;
```

#### 1.10 Update AccessHome Method (Lines 273-309)

**Remove Firebase "data" Wrapper**:

**Find** (line ~283):
```cpp
std::string payload = "{\"data\":{\"homeId\":\"" + homeId + "\",\"csrData\":\"" + csrData + "\"}}";
```

**Replace With**:
```cpp
std::string payload = "{\"homeId\":\"" + homeId + "\",\"csrData\":\"" + csrData + "\"}";
```

#### 1.11 Update ListHomes Method (Lines 311-379)

**Remove Firebase "data" Wrapper**:

**Find** (line ~319):
```cpp
std::string payload = "{\"data\":{}}";
```

**Replace With**:
```cpp
std::string payload = "{}";
```

#### 1.12 Update Log Messages

**Global Search/Replace**:
- `"Firebase"` → `"Supabase"`
- `"Firebase emulator"` → `"Supabase"`
- `"Firebase Functions"` → `"Supabase Edge Functions"`

**Save the file**.

---

### Step 2: Create SupabaseOperationalCredentialsIssuer.h

**Source Template**: `FirebaseOperationalCredentialsIssuer.h`

```bash
cp FirebaseOperationalCredentialsIssuer.h SupabaseOperationalCredentialsIssuer.h
```

#### 2.1 Update Header Guard

**Find** (lines 1-3):
```cpp
#pragma once

#include "FirebaseOperationalCredentialsIssuer.h"
```

**Replace With**:
```cpp
#pragma once

#include <controller/ExampleOperationalCredentialsIssuer.h>
```

#### 2.2 Rename Class

**Search**: `FirebaseOperationalCredentialsIssuer`
**Replace**: `SupabaseOperationalCredentialsIssuer`

#### 2.3 Update Method Names

**Find**:
```cpp
void SetFirebaseCertificates(const chip::ByteSpan & rcac, const chip::ByteSpan & icac);
```

**Replace With**:
```cpp
void SetSupabaseCertificates(const chip::ByteSpan & rcac, const chip::ByteSpan & icac);
```

**Save the file**.

---

### Step 3: Create SupabaseOperationalCredentialsIssuer.cpp

**Source Template**: `FirebaseOperationalCredentialsIssuer.cpp`

```bash
cp FirebaseOperationalCredentialsIssuer.cpp SupabaseOperationalCredentialsIssuer.cpp
```

#### 3.1 Update Header Include

**Find** (line ~18):
```cpp
#include "FirebaseOperationalCredentialsIssuer.h"
```

**Replace With**:
```cpp
#include "SupabaseOperationalCredentialsIssuer.h"
```

#### 3.2 Rename Class

**Search**: `FirebaseOperationalCredentialsIssuer`
**Replace**: `SupabaseOperationalCredentialsIssuer`

#### 3.3 Update Method Names

**Find**:
```cpp
void FirebaseOperationalCredentialsIssuer::SetFirebaseCertificates(...)
```

**Replace With**:
```cpp
void SupabaseOperationalCredentialsIssuer::SetSupabaseCertificates(...)
```

#### 3.4 Update Log Messages

**Global Search/Replace**:
- `"Firebase"` → `"Supabase"`

**Save the file**.

---

### Step 4: Create SupabaseCredentialIssuerCommands.h

**Source Template**: `FirebaseCredentialIssuerCommands.h`

```bash
cp FirebaseCredentialIssuerCommands.h SupabaseCredentialIssuerCommands.h
```

#### 4.1 Update Includes (Lines 31-32)

**Before**:
```cpp
#include "FirebaseEmulatorClient.h"
#include "FirebaseOperationalCredentialsIssuer.h"
```

**After**:
```cpp
#include "SupabaseClient.h"
#include "SupabaseOperationalCredentialsIssuer.h"
```

#### 4.2 Rename Class and Update Constructor (Lines 34-39)

**Before**:
```cpp
class FirebaseCredentialIssuerCommands : public CredentialIssuerCommands
{
public:
    FirebaseCredentialIssuerCommands(const std::string & firebaseHost = "192.168.1.2", uint16_t authPort = 9099,
                                     uint16_t funcPort = 5001);
    ~FirebaseCredentialIssuerCommands();
```

**After**:
```cpp
class SupabaseCredentialIssuerCommands : public CredentialIssuerCommands
{
public:
    SupabaseCredentialIssuerCommands(const std::string & supabaseUrl = "https://vmhzoaoyvxfdlubxnudv.supabase.co",
                                     const std::string & anonKey = "");
    ~SupabaseCredentialIssuerCommands();
```

#### 4.3 Update Private Members (Lines 58-73)

**Before**:
```cpp
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
```

**After**:
```cpp
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
```

**Save the file**.

---

### Step 5: Create SupabaseCredentialIssuerCommands.cpp

**Source Template**: `FirebaseCredentialIssuerCommands.cpp`

```bash
cp FirebaseCredentialIssuerCommands.cpp SupabaseCredentialIssuerCommands.cpp
```

#### 5.1 Update Header Includes (Lines 19-20)

**Before**:
```cpp
#include "FirebaseCredentialIssuerCommands.h"
#include "FirebaseEmulatorClient.h"
```

**After**:
```cpp
#include "SupabaseCredentialIssuerCommands.h"
#include "SupabaseClient.h"
```

#### 5.2 Update Constructor (Lines 24-27)

**Before**:
```cpp
FirebaseCredentialIssuerCommands::FirebaseCredentialIssuerCommands(const std::string & firebaseHost, uint16_t authPort,
                                                                   uint16_t funcPort) :
    mDacVerifier(nullptr), mFirebaseHost(firebaseHost), mAuthPort(authPort), mFuncPort(funcPort)
{}
```

**After**:
```cpp
SupabaseCredentialIssuerCommands::SupabaseCredentialIssuerCommands(const std::string & supabaseUrl,
                                                                   const std::string & anonKey) :
    mDacVerifier(nullptr), mSupabaseUrl(supabaseUrl), mAnonKey(anonKey)
{}
```

#### 5.3 Update Destructor (Line 29)

**Before**:
```cpp
FirebaseCredentialIssuerCommands::~FirebaseCredentialIssuerCommands() {}
```

**After**:
```cpp
SupabaseCredentialIssuerCommands::~SupabaseCredentialIssuerCommands() {}
```

#### 5.4 Rename All Class Methods

**Search**: `FirebaseCredentialIssuerCommands::`
**Replace**: `SupabaseCredentialIssuerCommands::`

#### 5.5 Update InitializeCredentialsIssuer Method (Lines 31-40)

**Before**:
```cpp
CHIP_ERROR FirebaseCredentialIssuerCommands::InitializeCredentialsIssuer(chip::PersistentStorageDelegate & storage)
{
    // Initialize the Firebase operational credentials issuer
    ReturnErrorOnFailure(mFirebaseOpCredsIssuer.Initialize(storage));

    // Initialize Firebase client
    ReturnErrorOnFailure(InitializeFirebaseClient());

    return CHIP_NO_ERROR;
}
```

**After**:
```cpp
CHIP_ERROR SupabaseCredentialIssuerCommands::InitializeCredentialsIssuer(chip::PersistentStorageDelegate & storage)
{
    // Initialize the Supabase operational credentials issuer
    ReturnErrorOnFailure(mSupabaseOpCredsIssuer.Initialize(storage));

    // Initialize Supabase client
    ReturnErrorOnFailure(InitializeSupabaseClient());

    return CHIP_NO_ERROR;
}
```

#### 5.6 Update InitializeSupabaseClient Method (Lines 42-71)

**Before**:
```cpp
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
```

**After**:
```cpp
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
    auto authResult = mSupabaseClient->Authenticate("test@example.com", "testme");
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
```

#### 5.7 Update GetCredentialIssuer Method (Lines 89-92)

**Before**:
```cpp
chip::Controller::OperationalCredentialsDelegate * FirebaseCredentialIssuerCommands::GetCredentialIssuer()
{
    return &mFirebaseOpCredsIssuer;
}
```

**After**:
```cpp
chip::Controller::OperationalCredentialsDelegate * SupabaseCredentialIssuerCommands::GetCredentialIssuer()
{
    return &mSupabaseOpCredsIssuer;
}
```

#### 5.8 Update SetCredentialIssuerCATValues Method (Lines 94-97)

**Before**:
```cpp
void FirebaseCredentialIssuerCommands::SetCredentialIssuerCATValues(chip::CATValues cats)
{
    mFirebaseOpCredsIssuer.SetCATValuesForNextNOCRequest(cats);
}
```

**After**:
```cpp
void SupabaseCredentialIssuerCommands::SetCredentialIssuerCATValues(chip::CATValues cats)
{
    mSupabaseOpCredsIssuer.SetCATValuesForNextNOCRequest(cats);
}
```

#### 5.9 Update GenerateControllerNOCChain Method (Lines 99-115)

**Before**:
```cpp
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
```

**After**:
```cpp
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
```

#### 5.10 Update GetCertificatesFromSupabase Method (Lines 117-164)

**Before**:
```cpp
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
    ...
}
```

**After**:
```cpp
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
    ...
}
```

#### 5.11 Update Log Messages

**Global Search/Replace**:
- `"Firebase"` → `"Supabase"`

**Save the file**.

---

## Testing Procedure

### Test 1: Verify File Compilation

```bash
cd ~/aosp/esp-matter/connectedhomeip/connectedhomeip/examples/chip-tool
./build_examples.py --target linux-x64-chip-tool build
```

**Expected**: All 6 files compile without errors.

**If Errors**: Check for:
- Missing semicolons
- Incorrect class/method names
- Missing includes

### Test 2: Test Supabase Authentication

Create test file: `test_supabase_auth.cpp`

```cpp
#include "commands/example/SupabaseClient.h"
#include <lib/support/logging/CHIPLogging.h>

int main()
{
    chip::MatterCommissioningTool::SupabaseClient client;

    // Initialize
    auto initErr = client.Initialize(
        "https://vmhzoaoyvxfdlubxnudv.supabase.co",
        "YOUR_ANON_KEY_HERE"
    );

    if (initErr != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to initialize Supabase client");
        return 1;
    }

    // Test authentication
    auto authResult = client.Authenticate("test@example.com", "testme");

    if (authResult.success)
    {
        ChipLogProgress(AppServer, "✅ Authentication successful!");
        ChipLogProgress(AppServer, "Access Token: %s", authResult.accessToken.c_str());
        return 0;
    }
    else
    {
        ChipLogError(AppServer, "❌ Authentication failed: %s", authResult.errorMessage.c_str());
        return 1;
    }
}
```

Compile and run:
```bash
g++ -std=c++17 test_supabase_auth.cpp SupabaseClient.cpp -o test_auth \
    -I../../src \
    -lchip
./test_auth
```

**Expected Output**:
```
✅ Authentication successful!
Access Token: eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...
```

### Test 3: Test Certificate Retrieval

```cpp
// After authentication...
auto certBundle = client.GetCertificates(authResult.accessToken);

if (certBundle.success)
{
    ChipLogProgress(AppServer, "✅ Certificate retrieval successful!");
    ChipLogProgress(AppServer, "Root CA size: %zu bytes", certBundle.rootCaCert.size());
    ChipLogProgress(AppServer, "ICA size: %zu bytes", certBundle.icaCert.size());
}
```

**Expected Output**:
```
✅ Certificate retrieval successful!
Root CA size: 456 bytes
ICA size: 478 bytes
```

### Test 4: Test Full Commissioning

```bash
# Reset Matter devices
./scripts/reset-matter-devices.sh

# Commission device using Supabase backend
./out/linux-x64-chip-tool/chip-tool pairing onnetwork-long 1 20202021 3841 \
    --commissioner-name alpha \
    --use-supabase

# Expected: Device commissions successfully with Supabase certificates
```

---

## Integration with chip-tool

### Option 1: Command-Line Flag

Modify chip-tool main to accept `--use-supabase` flag:

**File**: `examples/chip-tool/main.cpp` (approximate location)

```cpp
// Add option parsing
if (useSupabase)
{
    credIssuerCommands = std::make_unique<SupabaseCredentialIssuerCommands>(
        "https://vmhzoaoyvxfdlubxnudv.supabase.co",
        "YOUR_ANON_KEY"
    );
}
else
{
    credIssuerCommands = std::make_unique<ExampleCredentialIssuerCommands>();
}
```

### Option 2: Environment Variables

```bash
export SUPABASE_URL="https://vmhzoaoyvxfdlubxnudv.supabase.co"
export SUPABASE_ANON_KEY="your-anon-key"
export USE_SUPABASE=1

./chip-tool pairing onnetwork-long 1 20202021 3841
```

### Option 3: Configuration File

Create `chip-tool.conf`:
```ini
[supabase]
enabled=true
url=https://vmhzoaoyvxfdlubxnudv.supabase.co
anon_key=your-anon-key
test_email=test@example.com
test_password=testme
```

---

## Troubleshooting

### Error: "Supabase authentication failed"

**Cause**: Invalid credentials or network issue

**Solutions**:
1. Check test account exists in Supabase Auth
2. Verify Supabase URL is correct
3. Check network connectivity:
   ```bash
   curl -v https://vmhzoaoyvxfdlubxnudv.supabase.co/auth/v1/health
   ```

### Error: "Failed to retrieve certificates"

**Cause**: No homes exist or access-home failed

**Solutions**:
1. Create a home for the test account:
   ```bash
   curl -X POST https://vmhzoaoyvxfdlubxnudv.supabase.co/functions/v1/create-home \
     -H "Authorization: Bearer $ACCESS_TOKEN" \
     -H "apikey: $ANON_KEY" \
     -H "Content-Type: application/json" \
     -d '{"nickname": "Test Home"}'
   ```

2. Check Edge Function logs in Supabase dashboard

### Error: "Certificate format validation failed"

**Cause**: Base64 decoding issue or invalid certificate

**Solutions**:
1. Verify access-home returns valid base64-encoded certificates
2. Test base64 decoding:
   ```bash
   echo "$BASE64_CERT" | base64 -d | openssl x509 -inform DER -text
   ```

### Error: Compilation fails with "undefined reference"

**Cause**: Missing library links or source files

**Solutions**:
1. Add all 6 Supabase files to build system
2. Link required libraries: `-lcurl`, `-lssl`, `-lcrypto`
3. Check `BUILD.gn` includes all sources

---

## Configuration Reference

### Supabase Endpoints

| Purpose | Endpoint | Method |
|---------|----------|--------|
| Authentication | `/auth/v1/token?grant_type=password` | POST |
| List Homes | `/functions/v1/list-homes` | POST |
| Access Home | `/functions/v1/access-home` | POST |
| Health Check | `/auth/v1/health` | GET |

### Required Headers

**All Requests**:
```
apikey: {supabase-anon-key}
Content-Type: application/json
```

**Authenticated Requests** (add):
```
Authorization: Bearer {access_token}
```

### Test Credentials

**Default Test Account**:
- Email: `test@example.com`
- Password: `testme`

**Create Additional Test Accounts** (if needed):
```bash
curl -X POST https://vmhzoaoyvxfdlubxnudv.supabase.co/auth/v1/signup \
  -H "apikey: $ANON_KEY" \
  -H "Content-Type: application/json" \
  -d '{
    "email": "new-test@example.com",
    "password": "securepassword"
  }'
```

---

## Summary

### Files Created
1. ✅ `SupabaseClient.h` (already created)
2. 📝 `SupabaseClient.cpp` (~900 lines)
3. 📝 `SupabaseOperationalCredentialsIssuer.h` (~120 lines)
4. 📝 `SupabaseOperationalCredentialsIssuer.cpp` (~250 lines)
5. 📝 `SupabaseCredentialIssuerCommands.h` (~80 lines)
6. 📝 `SupabaseCredentialIssuerCommands.cpp` (~220 lines)

### Key Changes from Firebase
- **URLs**: Localhost emulator → Production Supabase
- **Auth**: `idToken` → `accessToken`
- **Headers**: Added `apikey` header
- **Requests**: Removed Firebase `data` wrapper
- **Configuration**: Host/ports → Single URL + anon key

### Next Steps
1. Follow implementation steps 1-5
2. Run compilation test
3. Test authentication
4. Test certificate retrieval
5. Integrate with chip-tool
6. Commission Matter devices

---

**Total Estimated Time**: 4-6 hours for implementation and testing

**Difficulty**: Moderate (mostly copy/paste/rename with specific API changes)

**Dependencies**: All Supabase Edge Functions already deployed and functional
