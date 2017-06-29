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

#ifndef BACKEND_D3D12_DEPTHSTENCILSTATED3D12_H_
#define BACKEND_D3D12_DEPTHSTENCILSTATED3D12_H_

#include "common/DepthStencilState.h"

#include "d3d12_platform.h"

namespace backend {
namespace d3d12 {

    class Device;

    class DepthStencilState : public DepthStencilStateBase {
        public:
            DepthStencilState(Device* device, DepthStencilStateBuilder* builder);

            const D3D12_DEPTH_STENCIL_DESC& GetD3D12DepthStencilDescriptor() const;

        private:
            D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
    };

}
}

#endif // BACKEND_D3D12_DEPTHSTENCILSTATED3D12_H_
