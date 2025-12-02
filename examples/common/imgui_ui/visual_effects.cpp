/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include "visual_effects.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/identify-server/identify-server.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

namespace example {
namespace Ui {

VisualEffects::VisualEffects()
{
    mLastGlobalUpdate = std::chrono::steady_clock::now();
    ChipLogProgress(AppServer, "VisualEffects system initialized");
}

void VisualEffects::Update()
{
    auto now = std::chrono::steady_clock::now();

    // Throttle updates to avoid excessive polling
    if (now - mLastGlobalUpdate < UPDATE_INTERVAL)
    {
        return;
    }

    mLastGlobalUpdate = now;

    // Update all monitored endpoints
    for (auto & pair : mEndpointStates)
    {
        UpdateEndpointState(pair.first);
    }
}

void VisualEffects::StartMonitoring(chip::EndpointId endpoint)
{
    if (mEndpointStates.find(endpoint) != mEndpointStates.end())
    {
        ChipLogProgress(AppServer, "Already monitoring endpoint %d for identify effects", endpoint);
        return;
    }

    IdentifyState state;

    // Safely read initial IdentifyTime with proper locking
    uint16_t initialTime = 0;
    {
        chip::DeviceLayer::StackLock lock;
        chip::Protocols::InteractionModel::Status status =
            chip::app::Clusters::Identify::Attributes::IdentifyTime::Get(endpoint, &initialTime);
        if (status != chip::Protocols::InteractionModel::Status::Success)
        {
            initialTime = 0;
        }
    }

    state.identifyTime = initialTime;
    state.lastUpdate   = std::chrono::steady_clock::now();
    state.isActive     = (state.identifyTime > 0);

    mEndpointStates[endpoint] = state;

    ChipLogProgress(AppServer, "Started monitoring endpoint %d for identify effects (current time: %d)", endpoint,
                    state.identifyTime);
}

void VisualEffects::StopMonitoring(chip::EndpointId endpoint)
{
    auto it = mEndpointStates.find(endpoint);
    if (it != mEndpointStates.end())
    {
        mEndpointStates.erase(it);
        ChipLogProgress(AppServer, "Stopped monitoring endpoint %d for identify effects", endpoint);
    }
}

uint16_t VisualEffects::GetCurrentIdentifyTime(chip::EndpointId endpoint)
{
    // Return cached value to avoid thread safety issues
    // The value is updated via callbacks from the CHIP thread
    auto it = mEndpointStates.find(endpoint);
    if (it != mEndpointStates.end())
    {
        return it->second.identifyTime;
    }

    // If not monitored, return 0 (safe default)
    return 0;
}

bool VisualEffects::IsIdentifying(chip::EndpointId endpoint)
{
    auto it = mEndpointStates.find(endpoint);
    if (it != mEndpointStates.end())
    {
        return it->second.isActive;
    }

    // If not monitored, safely check directly with proper locking
    uint16_t identifyTime = 0;
    {
        chip::DeviceLayer::StackLock lock;
        chip::Protocols::InteractionModel::Status status =
            chip::app::Clusters::Identify::Attributes::IdentifyTime::Get(endpoint, &identifyTime);
        if (status != chip::Protocols::InteractionModel::Status::Success)
        {
            identifyTime = 0;
        }
    }
    return identifyTime > 0;
}

VisualEffects::EffectType VisualEffects::GetCurrentEffect(chip::EndpointId endpoint)
{
    auto it = mEndpointStates.find(endpoint);
    if (it != mEndpointStates.end())
    {
        return it->second.currentEffect;
    }

    return EffectType::None;
}

void VisualEffects::SetEffect(chip::EndpointId endpoint, EffectType effect)
{
    auto it = mEndpointStates.find(endpoint);
    if (it != mEndpointStates.end())
    {
        it->second.currentEffect = effect;
        ApplyEffect(endpoint, effect);
        ChipLogProgress(AppServer, "Set effect %d for endpoint %d", static_cast<int>(effect), endpoint);
    }
}

const VisualEffects::IdentifyState * VisualEffects::GetIdentifyState(chip::EndpointId endpoint) const
{
    auto it = mEndpointStates.find(endpoint);
    if (it != mEndpointStates.end())
    {
        return &it->second;
    }

    return nullptr;
}

void VisualEffects::Reset()
{
    mEndpointStates.clear();
    ChipLogProgress(AppServer, "VisualEffects system reset");
}

void VisualEffects::UpdateEndpointState(chip::EndpointId endpoint)
{
    auto it = mEndpointStates.find(endpoint);
    if (it == mEndpointStates.end())
    {
        return;
    }

    IdentifyState & state = it->second;

    // Safely read current IdentifyTime with proper locking
    uint16_t currentTime = 0;
    {
        chip::DeviceLayer::StackLock lock;
        chip::Protocols::InteractionModel::Status status =
            chip::app::Clusters::Identify::Attributes::IdentifyTime::Get(endpoint, &currentTime);
        if (status != chip::Protocols::InteractionModel::Status::Success)
        {
            currentTime = 0;
        }
    }

    uint16_t previousTime = state.identifyTime;

    // Update state
    state.identifyTime = currentTime;
    state.lastUpdate   = std::chrono::steady_clock::now();
    bool wasActive     = state.isActive;
    state.isActive     = (currentTime > 0);

    // Handle state changes
    if (previousTime != currentTime)
    {
        HandleIdentifyTimeChange(endpoint, previousTime, currentTime);
    }

    // Handle activation/deactivation
    if (!wasActive && state.isActive)
    {
        ChipLogProgress(AppServer, "Identify activated on endpoint %d (time: %d)", endpoint, currentTime);
        ApplyEffect(endpoint, EffectType::Blink); // Default to blink effect
    }
    else if (wasActive && !state.isActive)
    {
        ChipLogProgress(AppServer, "Identify deactivated on endpoint %d", endpoint);
        ApplyEffect(endpoint, EffectType::None);
    }
}

void VisualEffects::HandleIdentifyTimeChange(chip::EndpointId endpoint, uint16_t oldTime, uint16_t newTime)
{
    if (oldTime == 0 && newTime > 0)
    {
        ChipLogProgress(AppServer, "Identify started on endpoint %d (time: %d)", endpoint, newTime);
    }
    else if (oldTime > 0 && newTime == 0)
    {
        ChipLogProgress(AppServer, "Identify finished on endpoint %d", endpoint);
    }
    else if (oldTime != newTime && newTime > 0)
    {
        ChipLogProgress(AppServer, "Identify time updated on endpoint %d (%d -> %d)", endpoint, oldTime, newTime);
    }
}

void VisualEffects::ApplyEffect(chip::EndpointId endpoint, EffectType effect)
{
    // This is where visual effects would be applied
    // For now, we just log the effect application

    switch (effect)
    {
    case EffectType::None:
        ChipLogProgress(AppServer, "Applying no effect to endpoint %d", endpoint);
        break;
    case EffectType::Blink:
        ChipLogProgress(AppServer, "Applying blink effect to endpoint %d", endpoint);
        break;
    case EffectType::Breathe:
        ChipLogProgress(AppServer, "Applying breathe effect to endpoint %d", endpoint);
        break;
    case EffectType::Okay:
        ChipLogProgress(AppServer, "Applying okay effect to endpoint %d", endpoint);
        break;
    case EffectType::ChannelChange:
        ChipLogProgress(AppServer, "Applying channel change effect to endpoint %d", endpoint);
        break;
    case EffectType::FinishEffect:
        ChipLogProgress(AppServer, "Applying finish effect to endpoint %d", endpoint);
        break;
    case EffectType::StopEffect:
        ChipLogProgress(AppServer, "Applying stop effect to endpoint %d", endpoint);
        break;
    }
}

VisualEffects::EffectType VisualEffects::ConvertMatterEffect(uint8_t effectId)
{
    switch (effectId)
    {
    case 0x00:
        return EffectType::Blink;
    case 0x01:
        return EffectType::Breathe;
    case 0x02:
        return EffectType::Okay;
    case 0x0B:
        return EffectType::ChannelChange;
    case 0xFE:
        return EffectType::FinishEffect;
    case 0xFF:
        return EffectType::StopEffect;
    default:
        return EffectType::None;
    }
}

} // namespace Ui
} // namespace example
