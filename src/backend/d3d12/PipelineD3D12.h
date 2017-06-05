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

#ifndef BACKEND_D3D12_PIPELINED3D12_H_
#define BACKEND_D3D12_PIPELINED3D12_H_

#include "common/Pipeline.h"

#include "d3d12.h"

namespace backend {
namespace d3d12 {

    class Device;
    class ShaderModule;

    class Pipeline : public PipelineBase {
        public:
            Pipeline(Device* device, PipelineBuilder* builder);

            Microsoft::WRL::ComPtr<ID3D12PipelineState> GetRenderPipelineState();

        private:
            Device* device;

            Microsoft::WRL::ComPtr<ID3D12PipelineState> renderPipelineState;
            Microsoft::WRL::ComPtr<ID3D12PipelineState> computePipelineState;
    };

}
}

#endif // BACKEND_D3D12_PIPELINED3D12_H_
