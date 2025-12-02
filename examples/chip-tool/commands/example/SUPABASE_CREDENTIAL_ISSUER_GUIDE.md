# Supabase Credential Issuer Implementation Guide

## Overview

This guide explains how to create Supabase credential issuer files for chip-tool to replace the Firebase emulator dependency.

## Files to Create

### 1. SupabaseClient.h ✅ (Created)
Header file defining the Supabase HTTP client interface.

### 2. SupabaseClient.cpp (To Create)

**Key Endpoints:**
- **Auth**: `https://vmhzoaoyvxfdlubxnudv.supabase.co/auth/v1/token?grant_type=password`
- **List Homes**: `https://vmhzoaoyvxfdlubxnudv.supabase.co/functions/v1/list-homes`
- **Access Home**: `https://vmhzoaoyvxfdlubxnudv.supabase.co/functions/v1/access-home`

**Authentication Request:**
```json
{
  "email": "test@example.com",
  "password": "password"
}
```

**Headers:**
```
apikey: {supabase-anon-key}
Content-Type: application/json
Authorization: Bearer {access_token}  // For authenticated requests
```

**Authentication Response:**
```json
{
  "access_token": "eyJhbGc...",
  "refresh_token": "...",
  "expires_in": 3600
}
```

**Key Differences from Firebase:**
1. Use `access_token` instead of `idToken`
2. No need for `grant_type` or API key in URL (use header instead)
3. Edge Functions use simpler JSON (no `data` wrapper)
4. Production URLs instead of localhost emulator

### 3. SupabaseOperationalCredentialsIssuer.h (To Create)

Copy from `FirebaseOperationalCredentialsIssuer.h` and rename:
- Class: `FirebaseOperationalCredentialsIssuer` → `SupabaseOperationalCredentialsIssuer`
- Methods: Keep same interface
- Comments: Update to reference Supabase

### 4. SupabaseOperationalCredentialsIssuer.cpp (To Create)

Copy from `FirebaseOperationalCredentialsIssuer.cpp` and:
- Rename class to `SupabaseOperationalCredentialsIssuer`
- Update method names: `SetFirebaseCertificates` → `SetSupabaseCertificates`
- Keep all logic the same (it's backend-agnostic)

### 5. SupabaseCredentialIssuerCommands.h (To Create)

```cpp
class SupabaseCredentialIssuerCommands : public CredentialIssuerCommands
{
public:
    SupabaseCredentialIssuerCommands(const std::string & supabaseUrl = "https://vmhzoaoyvxfdlubxnudv.supabase.co",
                                     const std::string & anonKey = "your-anon-key");
    ~SupabaseCredentialIssuerCommands();

    CHIP_ERROR InitializeCredentialsIssuer(chip::PersistentStorageDelegate & storage) override;
    // ... same methods as Firebase version

private:
    CHIP_ERROR InitializeSupabaseClient();
    CHIP_ERROR GetCertificatesFromSupabase(chip::MutableByteSpan & rcac, chip::MutableByteSpan & icac);

    std::unique_ptr<chip::MatterCommissioningTool::SupabaseClient> mSupabaseClient;
    chip::MatterCommissioningTool::SupabaseOperationalCredentialsIssuer mSupabaseOpCredsIssuer;

    std::string mSupabaseUrl;
    std::string mAnonKey;
    std::string mAccessToken;
};
```

### 6. SupabaseCredentialIssuerCommands.cpp (To Create)

Key changes from Firebase version:
```cpp
SupabaseCredentialIssuerCommands::SupabaseCredentialIssuerCommands(
    const std::string & supabaseUrl, const std::string & anonKey) :
    mDacVerifier(nullptr), mSupabaseUrl(supabaseUrl), mAnonKey(anonKey)
{}

CHIP_ERROR SupabaseCredentialIssuerCommands::InitializeSupabaseClient()
{
    if (mSupabaseInitialized)
        return CHIP_NO_ERROR;

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

## Implementation Steps

1. **Create SupabaseClient.cpp** (largest file)
   - Copy FirebaseEmulatorClient.cpp as template
   - Update all URLs to use Supabase endpoints
   - Change authentication to use Supabase Auth API
   - Update JSON request/response parsing for Supabase format

2. **Create SupabaseOperationalCredentialsIssuer.h/cpp**
   - Copy Firebase versions
   - Rename class
   - Update method names
   - Keep same logic

3. **Create SupabaseCredentialIssuerCommands.h/cpp**
   - Copy Firebase versions
   - Update constructor to take supabaseUrl and anonKey
   - Update initialization logic
   - Change mFirebaseClient to mSupabaseClient

## Configuration

**Default Values:**
- Supabase URL: `https://vmhzoaoyvxfdlubxnudv.supabase.co`
- Anon Key: Store in code or pass as parameter
- Test Email: `test@example.com`
- Test Password: `testme`

## Usage in chip-tool

Once implemented, chip-tool can use Supabase:

```bash
# Commission with Supabase backend
chip-tool pairing onnetwork-long 1 20202021 3841 \
    --commissioner-name alpha \
    --use-supabase
```

## Testing

1. Test authentication:
```cpp
SupabaseClient client;
client.Initialize("https://vmhzoaoyvxfdlubxnudv.supabase.co", "anon-key");
auto result = client.Authenticate("test@example.com", "password");
// Should return access_token
```

2. Test certificate retrieval:
```cpp
auto certs = client.GetCertificates(accessToken);
// Should return Root CA and ICA certificates
```

3. Test commissioning:
```bash
# Run chip-tool with Supabase backend
./chip-tool ...
```

## Notes

- All Edge Functions are already deployed and working
- Supabase Auth is configured for email/password
- Certificates are generated by access-home Edge Function
- No local emulator needed - uses production Supabase

## Complete Implementation

The complete implementation files are too large to include here. You can:
1. Copy Firebase files as templates
2. Use search/replace to change Firebase → Supabase
3. Update URLs and authentication logic as described above
4. Test with real Supabase endpoints
