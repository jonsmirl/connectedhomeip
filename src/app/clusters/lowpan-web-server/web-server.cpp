/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "web-server.h"

#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/data-model/Encode.h>
#include <clusters/LowpanWebServer/AttributeIds.h>
#include <clusters/LowpanWebServer/ClusterId.h>
#include <lib/support/logging/CHIPLogging.h>

// Platform-specific JWT implementation (provided by application)
extern "C" {
    // Returns ESP_OK (0) on success, fills token_out buffer
    int http_server_create_jwt_impl(char *token_out, size_t *token_len, uint32_t expiry_secs);
}

// Weak default implementation (returns failure if not provided)
__attribute__((weak)) int http_server_create_jwt_impl(char *token_out, size_t *token_len, uint32_t expiry_secs)
{
    (void)token_out;
    (void)token_len;
    (void)expiry_secs;
    return -1; // Not implemented
}

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LowpanWebServer;

namespace {

class LowpanWebServerAttrAccess : public app::AttributeAccessInterface
{
public:
    LowpanWebServerAttrAccess() :
        app::AttributeAccessInterface(Optional<EndpointId>::Missing(), LowpanWebServer::Id) {}

    CHIP_ERROR Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder) override;
};

LowpanWebServerAttrAccess gLowpanWebServerAttrAccess;

CHIP_ERROR LowpanWebServerAttrAccess::Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::Jwt::Id: {
        // Generate JWT token
        char tokenBuffer[256];
        size_t tokenLen = 0;

        int result = http_server_create_jwt_impl(tokenBuffer, &tokenLen, 0);
        if (result != 0)
        {
            ChipLogError(Zcl, "LowpanWebServer: Failed to create JWT token");
            return aEncoder.Encode(CharSpan());
        }

        ChipLogProgress(Zcl, "LowpanWebServer: JWT attribute read, token length %u", (unsigned)tokenLen);
        return aEncoder.Encode(CharSpan(tokenBuffer, tokenLen));
    }
    default:
        break;
    }

    return CHIP_NO_ERROR;
}

} // anonymous namespace

void MatterLowpanWebServerPluginServerInitCallback()
{
    app::AttributeAccessInterfaceRegistry::Instance().Register(&gLowpanWebServerAttrAccess);
    ChipLogProgress(Zcl, "LowpanWebServer: Server initialized");
}

void MatterLowpanWebServerPluginServerShutdownCallback()
{
    app::AttributeAccessInterfaceRegistry::Instance().Unregister(&gLowpanWebServerAttrAccess);
    ChipLogProgress(Zcl, "LowpanWebServer: Server shutdown");
}
