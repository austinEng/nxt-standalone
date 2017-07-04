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

#ifndef BACKEND_D3D12_BINDGROUPD3D12_H_
#define BACKEND_D3D12_BINDGROUPD3D12_H_

#include "common/BindGroup.h"

#include "d3d12_platform.h"

#include "DescriptorHeapAllocator.h"

namespace backend {
namespace d3d12 {

    class Device;

    class BindGroup : public BindGroupBase {
        public:
            BindGroup(Device* device, BindGroupBuilder* builder);

            //* Record descriptors of this BindGroup's BindGroupLayout into the provided descriptor heap and offsets.
            //* These offsets are remembered in BindGroup::cbvUavSrvHeapOffset and BindGroup::samplerHeapOffset so that
            //* handle to the recorded descriptors can be retrieved later.
            //* The input offsets are incremented by the number of descriptors written.
            //* The BindGroup also keeps track of the heapSerial which serves as an id of the descriptor heap
            //* that it last wrote to. A BindGroup may be used multiple times but it only needs to be recorded in a heap once
            void RecordDescriptors(const DescriptorHeapHandle &cbvSrvUavHeapStart, uint32_t* cbvUavSrvHeapOffset, const DescriptorHeapHandle &samplerHeapStart, uint32_t* samplerHeapOffset, uint64_t serial);

            uint32_t GetCbvUavSrvHeapOffset() const;
            uint32_t GetSamplerHeapOffset() const;
            uint64_t GetHeapSerial() const;

        private:
            Device* device;
            uint32_t cbvUavSrvHeapOffset;
            uint32_t samplerHeapOffset;
            uint64_t heapSerial = 0;
    };

}
}

#endif // BACKEND_D3D12_BINDGROUPD3D12_H_
