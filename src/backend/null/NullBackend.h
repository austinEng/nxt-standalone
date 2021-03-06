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

#ifndef BACKEND_NULL_NULLBACKEND_H_
#define BACKEND_NULL_NULLBACKEND_H_

#include "nxt/nxtcpp.h"

#include "backend/Buffer.h"
#include "backend/BindGroup.h"
#include "backend/BindGroupLayout.h"
#include "backend/Device.h"
#include "backend/CommandBuffer.h"
#include "backend/DepthStencilState.h"
#include "backend/InputState.h"
#include "backend/Framebuffer.h"
#include "backend/Pipeline.h"
#include "backend/PipelineLayout.h"
#include "backend/Queue.h"
#include "backend/RenderPass.h"
#include "backend/Sampler.h"
#include "backend/ShaderModule.h"
#include "backend/Texture.h"
#include "backend/ToBackend.h"

namespace backend {
namespace null {

    using BindGroup = BindGroupBase;
    using BindGroupLayout = BindGroupLayoutBase;
    class Buffer;
    using BufferView = BufferViewBase;
    class CommandBuffer;
    using DepthStencilState = DepthStencilStateBase;
    class Device;
    using InputState = InputStateBase;
    using Framebuffer = FramebufferBase;
    using Pipeline = PipelineBase;
    using PipelineLayout = PipelineLayoutBase;
    class Queue;
    using RenderPass = RenderPassBase;
    using Sampler = SamplerBase;
    using ShaderModule = ShaderModuleBase;
    class Texture;
    using TextureView = TextureViewBase;

    struct NullBackendTraits {
        using BindGroupType = BindGroup;
        using BindGroupLayoutType = BindGroupLayout;
        using BufferType = Buffer;
        using BufferViewType = BufferView;
        using CommandBufferType = CommandBuffer;
        using DepthStencilStateType = DepthStencilState;
        using DeviceType = Device;
        using InputStateType = InputState;
        using FramebufferType = Framebuffer;
        using PipelineType = Pipeline;
        using PipelineLayoutType = PipelineLayout;
        using QueueType = Queue;
        using RenderPassType = RenderPass;
        using SamplerType = Sampler;
        using ShaderModuleType = ShaderModule;
        using TextureType = Texture;
        using TextureViewType = TextureView;
    };

    template<typename T>
    auto ToBackend(T&& common) -> decltype(ToBackendBase<NullBackendTraits>(common)) {
        return ToBackendBase<NullBackendTraits>(common);
    }

    struct PendingOperation {
        virtual ~PendingOperation() = default;
        virtual void Execute() = 0;
    };

    class Device : public DeviceBase {
        public:
            Device();
            ~Device();

            BindGroupBase* CreateBindGroup(BindGroupBuilder* builder) override;
            BindGroupLayoutBase* CreateBindGroupLayout(BindGroupLayoutBuilder* builder) override;
            BufferBase* CreateBuffer(BufferBuilder* builder) override;
            BufferViewBase* CreateBufferView(BufferViewBuilder* builder) override;
            CommandBufferBase* CreateCommandBuffer(CommandBufferBuilder* builder) override;
            DepthStencilStateBase* CreateDepthStencilState(DepthStencilStateBuilder* builder) override;
            InputStateBase* CreateInputState(InputStateBuilder* builder) override;
            FramebufferBase* CreateFramebuffer(FramebufferBuilder* builder) override;
            PipelineBase* CreatePipeline(PipelineBuilder* builder) override;
            PipelineLayoutBase* CreatePipelineLayout(PipelineLayoutBuilder* builder) override;
            QueueBase* CreateQueue(QueueBuilder* builder) override;
            RenderPassBase* CreateRenderPass(RenderPassBuilder* builder) override;
            SamplerBase* CreateSampler(SamplerBuilder* builder) override;
            ShaderModuleBase* CreateShaderModule(ShaderModuleBuilder* builder) override;
            TextureBase* CreateTexture(TextureBuilder* builder) override;
            TextureViewBase* CreateTextureView(TextureViewBuilder* builder) override;

            void TickImpl() override;

            void AddPendingOperation(std::unique_ptr<PendingOperation> operation);
            std::vector<std::unique_ptr<PendingOperation>> AcquirePendingOperations();

        private:
            std::vector<std::unique_ptr<PendingOperation>> pendingOperations;
    };

    class Buffer : public BufferBase {
        public:
            Buffer(BufferBuilder* builder);
            ~Buffer();

            void MapReadOperationCompleted(uint32_t serial, const void* ptr);

        private:
            void SetSubDataImpl(uint32_t start, uint32_t count, const uint32_t* data) override;
            void MapReadAsyncImpl(uint32_t serial, uint32_t start, uint32_t count) override;
            void UnmapImpl() override;
            void TransitionUsageImpl(nxt::BufferUsageBit currentUsage, nxt::BufferUsageBit targetUsage) override;

            std::unique_ptr<char[]> backingData;
    };

    class CommandBuffer : public CommandBufferBase {
        public:
            CommandBuffer(CommandBufferBuilder* builder);
            ~CommandBuffer();

            void Execute();

        private:
            CommandIterator commands;
    };

    class Queue : public QueueBase {
        public:
            Queue(QueueBuilder* builder);
            ~Queue();

            // NXT API
            void Submit(uint32_t numCommands, CommandBuffer* const * commands);
    };

    class Texture : public TextureBase {
        public:
            Texture(TextureBuilder* buidler);
            ~Texture();

        private:
            void TransitionUsageImpl(nxt::TextureUsageBit currentUsage, nxt::TextureUsageBit targetUsage) override;
    };

}
}

#endif // BACKEND_NULL_NULLBACKEND_H_
