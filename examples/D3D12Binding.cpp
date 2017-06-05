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

#include "BackendBinding.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#include <exception>
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <comdef.h>

using Microsoft::WRL::ComPtr;

namespace backend {
namespace d3d12 {
    void Init(ComPtr<ID3D12Device> d3d12Device, nxtProcTable* procs, nxtDevice* device);
    ComPtr<ID3D12CommandQueue> GetCommandQueue(nxtDevice device);
    void SetNextRenderTarget(nxtDevice device, ComPtr<ID3D12Resource> renderTargetResource, D3D12_CPU_DESCRIPTOR_HANDLE renderTargetDescriptor);
}
}

class D3D12Binding : public BackendBinding {
    public:
        void SetupGLFWWindowHints() override {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }

        void GetProcAndDevice(nxtProcTable* procs, nxtDevice* device) override {
            uint32_t dxgiFactoryFlags = 0;
#ifdef _DEBUG
            // Enable the debug layer (requires the Graphics Tools "optional feature").
            // NOTE: Enabling the debug layer after device creation will invalidate the active device.
            {
                ComPtr<ID3D12Debug> debugController;
                if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
                {
                    debugController->EnableDebugLayer();

                    // Enable additional debug layers.
                    dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
                }
            }
#endif

            ASSERT_SUCCESS(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));
            if (!GetHardwareAdapter(factory.Get(), &hardwareAdapter)) {
                throw std::exception("Could not get hardware adapter");
            }
            ASSERT_SUCCESS(D3D12CreateDevice(
                hardwareAdapter.Get(),
                D3D_FEATURE_LEVEL_11_0,
                IID_PPV_ARGS(&d3d12Device)
            ));

            backend::d3d12::Init(d3d12Device, procs, device);
            backendDevice = *device;
            commandQueue = backend::d3d12::GetCommandQueue(backendDevice);

            int width, height;
            glfwGetWindowSize(window, &width, &height);
            DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
            swapChainDesc.Width = width;
            swapChainDesc.Height = height;
            swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferCount = FrameCount;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            swapChainDesc.SampleDesc.Count = 1;

            HWND win32Window = glfwGetWin32Window(window);
            ComPtr<IDXGISwapChain1> swapChain1;
            ASSERT_SUCCESS(factory->CreateSwapChainForHwnd(
                commandQueue.Get(),
                win32Window,
                &swapChainDesc,
                nullptr,
                nullptr,
                &swapChain1
            ));
            ASSERT_SUCCESS(swapChain1.As(&swapChain));
            frameIndex = swapChain->GetCurrentBackBufferIndex();

            // Describe and create a render target view (RTV) descriptor heap.
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
            rtvHeapDesc.NumDescriptors = FrameCount;
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            ASSERT_SUCCESS(d3d12Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&renderTargetViewHeap)));

            rtvDescriptorSize = d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

            // Create a RTV for each frame.
            D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle = renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
            for (uint32_t n = 0; n < FrameCount; ++n) {
                ASSERT_SUCCESS(swapChain->GetBuffer(n, IID_PPV_ARGS(&renderTargetResources[n])));
                d3d12Device->CreateRenderTargetView(renderTargetResources[n].Get(), nullptr, renderTargetViewHandle);
                renderTargetViewHandle.ptr += rtvDescriptorSize;
            }

            ASSERT_SUCCESS(d3d12Device->CreateFence(fenceValues[frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
            fenceValues[frameIndex]++;
            fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (fenceEvent == nullptr) {
                ASSERT_SUCCESS(HRESULT_FROM_WIN32(GetLastError()));
            }

            // Schedule a Signal command in the queue.
            ASSERT_SUCCESS(commandQueue->Signal(fence.Get(), fenceValues[frameIndex]));

            // Wait until the fence has been processed.
            ASSERT_SUCCESS(fence->SetEventOnCompletion(fenceValues[frameIndex], fenceEvent));
            WaitForSingleObjectEx(fenceEvent, INFINITE, FALSE);

            // Increment the fence value for the current frame.
            fenceValues[frameIndex]++;

            SetNextRenderTarget();
        }
        void SwapBuffers() override {
            // Present the rendered frame
            ASSERT_SUCCESS(swapChain->Present(1, 0));

            const uint64_t currentFenceValue = fenceValues[frameIndex];
            ASSERT_SUCCESS(commandQueue->Signal(fence.Get(), currentFenceValue));

            frameIndex = swapChain->GetCurrentBackBufferIndex();

            SetNextRenderTarget();

            // If the next frame is not ready to be rendered yet, wait until it is ready.
            if (fence->GetCompletedValue() < fenceValues[frameIndex]) {
                ASSERT_SUCCESS(fence->SetEventOnCompletion(fenceValues[frameIndex], fenceEvent));
                WaitForSingleObjectEx(fenceEvent, INFINITE, FALSE);
            }

            // Set the fence value for the next frame.
            fenceValues[frameIndex] = currentFenceValue + 1;
        }

    private:
        nxtDevice backendDevice = nullptr;

        static const int FrameCount = 2;
        uint32_t frameIndex = 0;
        uint32_t rtvDescriptorSize;
        ComPtr<IDXGIFactory4> factory;
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        ComPtr<ID3D12Device> d3d12Device;
        ComPtr<ID3D12CommandQueue> commandQueue;
        ComPtr<IDXGISwapChain3> swapChain;
        ComPtr<ID3D12DescriptorHeap> renderTargetViewHeap;
        ComPtr<ID3D12Resource> renderTargetResources[FrameCount];
        uint64_t fenceValues[FrameCount] = { 0, 0 };
        ComPtr<ID3D12Fence> fence;
        HANDLE fenceEvent;

        void SetNextRenderTarget() {
            D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle = renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
            renderTargetViewHandle.ptr += rtvDescriptorSize * frameIndex;
            backend::d3d12::SetNextRenderTarget(backendDevice, renderTargetResources[frameIndex], renderTargetViewHandle);
        }

        static void ASSERT_SUCCESS(HRESULT hr) {
            assert(SUCCEEDED(hr));
        }

        static bool GetHardwareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter1** ppAdapter) {
            *ppAdapter = nullptr;
            for (UINT adapterIndex = 0; ; ++adapterIndex) {
                IDXGIAdapter1* pAdapter = nullptr;
                if (DXGI_ERROR_NOT_FOUND == pFactory->EnumAdapters1(adapterIndex, &pAdapter)) {
                    // No more adapters to enumerate.
                    break;
                }

                // Check to see if the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
                    *ppAdapter = pAdapter;
                    return true;
                }
                pAdapter->Release();
            }
            return false;
        }
};

BackendBinding* CreateD3D12Binding() {
    return new D3D12Binding;
}
