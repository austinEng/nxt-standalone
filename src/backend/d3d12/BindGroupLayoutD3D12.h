// Copyright 2017 The NXT Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef BACKEND_D3D12_BINDGROUPLAYOUTD3D12_H_
#define BACKEND_D3D12_BINDGROUPLAYOUTD3D12_H_

#include "common/BindGroupLayout.h"

#include "d3d12_platform.h"

namespace backend {
namespace d3d12 {

    class Device;

    //* Each bind group layout maps to one or two D3D12 descriptor tables (Two because in D3D12, samplers must be in a separate table).
    //* This class is responsible for creating and caching the ranges that will be used to populate the descriptor tables in d3d12::PipelineLayout.
    //* These ranges will be copied in d3d12::PipelineLayout because the base register needs to be offset by kMaxBindingsPerGroup.
    //* (This is because SPIRV-Cross uses the binding index as the register index. We renumber these so that register = bindingIndex + bindingSet * kMaxBindingsPerGroup)
    //* Because we want to minimize the size of the root signature, the descriptors will be packed so there is one range for CBV descriptors,
    //* one range for UAV descriptors, one range for SRV descriptors, and one range for Sampler descriptors.
    //* When we record descriptors into a descriptor heap, we need to know their offsets into the descriptor table. These offsets are stored in BindGroupLayout::bindingOffsets

    class BindGroupLayout : public BindGroupLayoutBase {
        public:
            BindGroupLayout(Device* device, BindGroupLayoutBuilder* builder);

            enum DescriptorType {
                CBV,
                UAV,
                SRV,
                Sampler,
                Count,
            };

            const std::array<uint32_t, kMaxBindingsPerGroup>& GetBindingOffsets() const;
            uint32_t GetCbvUavSrvDescriptorTableSize() const;
            uint32_t GetSamplerDescriptorTableSize() const;
            uint32_t GetCbvUavSrvDescriptorCount() const;
            uint32_t GetSamplerDescriptorCount() const;
            const D3D12_DESCRIPTOR_RANGE* GetCbvUavSrvDescriptorRanges() const;
            const D3D12_DESCRIPTOR_RANGE* GetSamplerDescriptorRanges() const;

        private:
            Device* device;
            std::array<uint32_t, kMaxBindingsPerGroup> bindingOffsets;
            std::array<uint32_t, DescriptorType::Count> descriptorCounts;
            D3D12_DESCRIPTOR_RANGE ranges[DescriptorType::Count];
    };

}
}

#endif // BACKEND_D3D12_BINDGROUPLAYOUTD3D12_H_
