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
#pragma once

#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPMem.h>
#include <platform/DeviceInfoProvider.h>

namespace chip {
namespace DeviceLayer {

/**
 * @brief Custom DeviceInfoProvider for Keypad App
 *
 * This provider initializes the FixedLabel cluster on endpoints 1-8
 * with button labels "Btn1" through "Btn8".
 */
class KeypadDeviceInfoProvider : public DeviceInfoProvider
{
public:
    KeypadDeviceInfoProvider() = default;
    ~KeypadDeviceInfoProvider() override {}

    // Iterators
    FixedLabelIterator * IterateFixedLabel(EndpointId endpoint) override;
    UserLabelIterator * IterateUserLabel(EndpointId endpoint) override;
    SupportedLocalesIterator * IterateSupportedLocales() override;
    SupportedCalendarTypesIterator * IterateSupportedCalendarTypes() override;

    static KeypadDeviceInfoProvider & GetDefaultInstance();

protected:
    class FixedLabelIteratorImpl : public FixedLabelIterator
    {
    public:
        FixedLabelIteratorImpl(EndpointId endpoint);
        size_t Count() override;
        bool Next(FixedLabelType & output) override;
        void Release() override { chip::Platform::Delete(this); }

    private:
        static constexpr size_t kNumSupportedFixedLabels = 1;
        EndpointId mEndpoint                             = 0;
        size_t mIndex                                    = 0;
    };

    CHIP_ERROR SetUserLabelLength(EndpointId endpoint, size_t val) override;
    CHIP_ERROR GetUserLabelLength(EndpointId endpoint, size_t & val) override;
    CHIP_ERROR SetUserLabelAt(EndpointId endpoint, size_t index, const UserLabelType & userLabel) override;
    CHIP_ERROR DeleteUserLabelAt(EndpointId endpoint, size_t index) override;
};

} // namespace DeviceLayer
} // namespace chip

