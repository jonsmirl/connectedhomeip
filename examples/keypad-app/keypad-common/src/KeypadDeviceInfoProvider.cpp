/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#include "KeypadDeviceInfoProvider.h"

#include <lib/core/TLV.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/SafeInt.h>
#include <lib/support/Span.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <stdlib.h>
#include <string.h>

namespace chip {
namespace DeviceLayer {

KeypadDeviceInfoProvider & KeypadDeviceInfoProvider::GetDefaultInstance()
{
    static KeypadDeviceInfoProvider sInstance;
    return sInstance;
}

DeviceInfoProvider::FixedLabelIterator * KeypadDeviceInfoProvider::IterateFixedLabel(EndpointId endpoint)
{
    // Only provide FixedLabel for endpoints 2-9 (endpoint 1 is the aggregator, endpoint 0 is root)
    if (endpoint >= 2 && endpoint <= 9)
    {
        return chip::Platform::New<FixedLabelIteratorImpl>(endpoint);
    }
    return nullptr;
}

KeypadDeviceInfoProvider::FixedLabelIteratorImpl::FixedLabelIteratorImpl(EndpointId endpoint) : mEndpoint(endpoint)
{
    mIndex = 0;
}

size_t KeypadDeviceInfoProvider::FixedLabelIteratorImpl::Count()
{
    // Each endpoint has one label
    return kNumSupportedFixedLabels;
}

bool KeypadDeviceInfoProvider::FixedLabelIteratorImpl::Next(FixedLabelType & output)
{
    // Only return one label per endpoint
    if (mIndex >= kNumSupportedFixedLabels)
    {
        return false;
    }

    // Set label to "button" and value to "BtnX" where X is the button number (1-8)
    // Endpoint 2 = Btn1, Endpoint 3 = Btn2, ..., Endpoint 9 = Btn8
    output.label = "button"_span;

    // Create the button label based on endpoint number
    // Endpoints 2-9 map to buttons 1-8
    static char buttonLabels[8][5] = { "Btn1", "Btn2", "Btn3", "Btn4", "Btn5", "Btn6", "Btn7", "Btn8" };

    // Validate endpoint is in range 2-9
    if (mEndpoint >= 2 && mEndpoint <= 9)
    {
        output.value = CharSpan::fromCharString(buttonLabels[mEndpoint - 2]);
    }
    else
    {
        return false;
    }

    mIndex++;
    return true;
}

DeviceInfoProvider::UserLabelIterator * KeypadDeviceInfoProvider::IterateUserLabel(EndpointId endpoint)
{
    // No user labels for keypad app
    (void) endpoint;
    return nullptr;
}

DeviceInfoProvider::SupportedLocalesIterator * KeypadDeviceInfoProvider::IterateSupportedLocales()
{
    // No supported locales for keypad app
    return nullptr;
}

DeviceInfoProvider::SupportedCalendarTypesIterator * KeypadDeviceInfoProvider::IterateSupportedCalendarTypes()
{
    // No supported calendar types for keypad app
    return nullptr;
}

CHIP_ERROR KeypadDeviceInfoProvider::SetUserLabelLength(EndpointId endpoint, size_t val)
{
    // Not implemented for keypad app
    (void) endpoint;
    (void) val;
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR KeypadDeviceInfoProvider::GetUserLabelLength(EndpointId endpoint, size_t & val)
{
    // Not implemented for keypad app
    (void) endpoint;
    (void) val;
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR KeypadDeviceInfoProvider::SetUserLabelAt(EndpointId endpoint, size_t index, const UserLabelType & userLabel)
{
    // Not implemented for keypad app
    (void) endpoint;
    (void) index;
    (void) userLabel;
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR KeypadDeviceInfoProvider::DeleteUserLabelAt(EndpointId endpoint, size_t index)
{
    // Not implemented for keypad app
    (void) endpoint;
    (void) index;
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

} // namespace DeviceLayer
} // namespace chip
