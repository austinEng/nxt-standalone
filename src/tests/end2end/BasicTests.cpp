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

#include "tests/NXTTest.h"

#include "utils/NXTHelpers.h"

class BasicTests : public NXTTest {
};

// Test Buffer::SetSubData changes the content of the buffer, but really this is the most
// basic test possible, and tests the test harness
TEST_P(BasicTests, BufferSetSubData) {
    nxt::Buffer buffer = device.CreateBufferBuilder()
        .SetSize(4)
        .SetAllowedUsage(nxt::BufferUsageBit::TransferSrc | nxt::BufferUsageBit::TransferDst)
        .SetInitialUsage(nxt::BufferUsageBit::TransferDst)
        .GetResult();

    uint32_t value = 3094587;
    buffer.SetSubData(0, 1, &value);

    EXPECT_BUFFER_U32_EQ(value, buffer, 0);
}

TEST_P(BasicTests, ReadPixelsTest) {
    RGBA8 red(255, 0, 0, 255);
    nxt::Buffer buffer = utils::CreateFrozenBufferFromData(device, &red, sizeof(red), nxt::BufferUsageBit::TransferSrc);

    nxt::Texture texture = device.CreateTextureBuilder()
        .SetDimension(nxt::TextureDimension::e2D)
        .SetExtent(1, 1, 1)
        .SetMipLevels(1)
        .SetAllowedUsage(nxt::TextureUsageBit::TransferSrc | nxt::TextureUsageBit::TransferDst)
        .SetFormat(nxt::TextureFormat::R8G8B8A8Unorm)
        .GetResult();

    nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
        .TransitionTextureUsage(texture, nxt::TextureUsageBit::TransferDst)
        .CopyBufferToTexture(buffer, 0, texture, 0, 0, 0, 1, 1, 1, 0)
        .GetResult();

    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(red, texture, 0, 0);
}

TEST_P(BasicTests, Buffer2Texture2Buffer) {
    static constexpr unsigned int kSize = 64;

    uint8_t data[4 * kSize * kSize] = {};
    for (unsigned int i = 0; i < 4 * kSize * kSize; ++i) {
        data[i] = i % 256;
    }

    nxt::Buffer srcBuffer = utils::CreateFrozenBufferFromData(device, data, 4 * kSize * kSize, nxt::BufferUsageBit::TransferSrc);

    nxt::Texture texture = device.CreateTextureBuilder()
        .SetDimension(nxt::TextureDimension::e2D)
        .SetExtent(kSize, kSize, 1)
        .SetFormat(nxt::TextureFormat::R8G8B8A8Unorm)
        .SetMipLevels(1)
        .SetAllowedUsage(nxt::TextureUsageBit::TransferDst | nxt::TextureUsageBit::TransferSrc)
        .GetResult();

    nxt::Buffer dstBuffer = device.CreateBufferBuilder()
        .SetAllowedUsage(nxt::BufferUsageBit::TransferDst | nxt::BufferUsageBit::TransferSrc)
        .SetInitialUsage(nxt::BufferUsageBit::TransferDst)
        .SetSize(4 * kSize * kSize)
        .GetResult();

    nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
        .TransitionTextureUsage(texture, nxt::TextureUsageBit::TransferDst)
        .CopyBufferToTexture(srcBuffer, 0, texture, 0, 0, 0, kSize, kSize, 1, 0)

        .TransitionTextureUsage(texture, nxt::TextureUsageBit::TransferSrc)
        .TransitionBufferUsage(dstBuffer, nxt::BufferUsageBit::TransferDst)
        .CopyTextureToBuffer(texture, 0, 0, 0, kSize, kSize, 1, 0, dstBuffer, 0)

        .GetResult();

    queue.Submit(1, &commands);

    const uint32_t* dataView = reinterpret_cast<uint32_t*>(data);
    EXPECT_BUFFER_U32_RANGE_EQ(dataView, dstBuffer, 0, kSize * kSize);
}

NXT_INSTANTIATE_TEST(BasicTests, MetalBackend, D3D12Backend)
