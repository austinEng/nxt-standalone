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

#include "DepthStencilStateD3D12.h"

#include "D3D12Backend.h"

namespace backend {
namespace d3d12 {

    namespace {

        D3D12_DEPTH_WRITE_MASK D3D12DepthWriteMask(bool writeMask) {
            return writeMask ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
        }

        D3D12_COMPARISON_FUNC D3D12CompareFunction(nxt::CompareFunction compareFunction) {
            switch (compareFunction) {
                case nxt::CompareFunction::Never:
                    return D3D12_COMPARISON_FUNC_NEVER;
                case nxt::CompareFunction::Less:
                    return D3D12_COMPARISON_FUNC_LESS;
                case nxt::CompareFunction::LessEqual:
                    return D3D12_COMPARISON_FUNC_LESS_EQUAL;
                case nxt::CompareFunction::Greater:
                    return D3D12_COMPARISON_FUNC_GREATER;
                case nxt::CompareFunction::GreaterEqual:
                    return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
                case nxt::CompareFunction::NotEqual:
                    return D3D12_COMPARISON_FUNC_NOT_EQUAL;
                case nxt::CompareFunction::Equal:
                    return D3D12_COMPARISON_FUNC_EQUAL;
                case nxt::CompareFunction::Always:
                    return D3D12_COMPARISON_FUNC_ALWAYS;
            }
        }

        D3D12_STENCIL_OP D3D12StencilOperation(nxt::StencilOperation stencilOperation) {
            switch (stencilOperation) {
                case nxt::StencilOperation::Keep:
                    return D3D12_STENCIL_OP_KEEP;
                case nxt::StencilOperation::Zero:
                    return D3D12_STENCIL_OP_ZERO;
                case nxt::StencilOperation::Replace:
                    return D3D12_STENCIL_OP_REPLACE;
                case nxt::StencilOperation::Invert:
                    return D3D12_STENCIL_OP_INVERT;
                case nxt::StencilOperation::IncrementClamp:
                    return D3D12_STENCIL_OP_INCR_SAT;
                case nxt::StencilOperation::DecrementClamp:
                    return D3D12_STENCIL_OP_DECR_SAT;
                case nxt::StencilOperation::IncrementWrap:
                    return D3D12_STENCIL_OP_INCR;
                case nxt::StencilOperation::DecrementWrap:
                    return D3D12_STENCIL_OP_DECR;
            }
        }
    }

    DepthStencilState::DepthStencilState(Device* device, DepthStencilStateBuilder* builder) : DepthStencilStateBase(builder) {

        depthStencilDesc.DepthEnable = DepthTestEnabled();
        if (depthStencilDesc.DepthEnable) {
            auto& depth = GetDepth();

            depthStencilDesc.DepthWriteMask = D3D12DepthWriteMask(depth.depthWriteEnabled);
            depthStencilDesc.DepthFunc = D3D12CompareFunction(depth.compareFunction);
        }

        depthStencilDesc.StencilEnable = StencilTestEnabled();
        if (depthStencilDesc.StencilEnable) {
            auto& stencil = GetStencil();

            depthStencilDesc.StencilReadMask = stencil.readMask;
            depthStencilDesc.StencilWriteMask = stencil.writeMask;

            depthStencilDesc.BackFace.StencilFailOp = D3D12StencilOperation(stencil.back.stencilFail);
            depthStencilDesc.BackFace.StencilDepthFailOp = D3D12StencilOperation(stencil.back.depthFail);
            depthStencilDesc.BackFace.StencilPassOp = D3D12StencilOperation(stencil.back.depthStencilPass);
            depthStencilDesc.BackFace.StencilFunc = D3D12CompareFunction(stencil.back.compareFunction);

            depthStencilDesc.FrontFace.StencilFailOp = D3D12StencilOperation(stencil.front.stencilFail);
            depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12StencilOperation(stencil.front.depthFail);
            depthStencilDesc.FrontFace.StencilPassOp = D3D12StencilOperation(stencil.front.depthStencilPass);
            depthStencilDesc.FrontFace.StencilFunc = D3D12CompareFunction(stencil.front.compareFunction);
        }
    }

    const D3D12_DEPTH_STENCIL_DESC& DepthStencilState::GetD3D12DepthStencilDescriptor() const {
        return depthStencilDesc;
    }
}
}
