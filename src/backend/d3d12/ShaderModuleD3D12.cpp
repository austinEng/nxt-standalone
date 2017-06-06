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

#include "ShaderModuleD3D12.h"

#include <spirv-cross/spirv_hlsl.hpp>

namespace backend {
namespace d3d12 {

    std::string GetBindingName(uint32_t group, uint32_t binding) {
        std::ostringstream o;
        o << "nxt_binding_" << group << "_" << binding;
        return o.str();
    }

    bool operator < (const BindingLocation& a, const BindingLocation& b) {
        return std::tie(a.group, a.binding) < std::tie(b.group, b.binding);
    }

    bool operator < (const CombinedSampler& a, const CombinedSampler& b) {
        return std::tie(a.samplerLocation, a.textureLocation) < std::tie(b.samplerLocation, b.textureLocation);
    }

    std::string CombinedSampler::GetName() const {
        std::ostringstream o;
        o << "nxt_combined";
        o << "_" << samplerLocation.group << "_" << samplerLocation.binding;
        o << "_with_" << textureLocation.group << "_" << textureLocation.binding;
        return o.str();
    }

    ShaderModule::ShaderModule(Device* device, ShaderModuleBuilder* builder)
        : ShaderModuleBase(builder), device(device) {
        spirv_cross::CompilerHLSL compiler(builder->AcquireSpirv());

        spirv_cross::CompilerHLSL::Options options;
        options.shader_model = 40; 
        options.flip_vert_y = false;
        options.fixup_clipspace = true;

        compiler.set_options(options);

        ExtractSpirvInfo(compiler);

        const auto& bindingInfo = GetBindingInfo();

        // Extract bindings names so that it can be used to get its location in program.
        // Now translate the separate sampler / textures into combined ones and store their info.
        // We need to do this before removing the set and binding decorations.
        compiler.build_combined_image_samplers();

        for (const auto& combined : compiler.get_combined_image_samplers()) {
            combinedInfo.emplace_back();

            auto& info = combinedInfo.back();
            info.samplerLocation.group = compiler.get_decoration(combined.sampler_id, spv::DecorationDescriptorSet);
            info.samplerLocation.binding = compiler.get_decoration(combined.sampler_id, spv::DecorationBinding);
            info.textureLocation.group = compiler.get_decoration(combined.image_id, spv::DecorationDescriptorSet);
            info.textureLocation.binding = compiler.get_decoration(combined.image_id, spv::DecorationBinding);
            compiler.set_name(combined.combined_id, info.GetName());
        }

        // Change binding names to be "nxt_binding_<group>_<binding>".
        // Also unsets the SPIRV "Binding" decoration as it outputs "layout(binding=)".
        for (uint32_t group = 0; group < kMaxBindGroups; ++group) {
            for (uint32_t binding = 0; binding < kMaxBindingsPerGroup; ++binding) {
                const auto& info = bindingInfo[group][binding];
                if (info.used) {
                    compiler.set_name(info.base_type_id, GetBindingName(group, binding));
                    compiler.unset_decoration(info.id, spv::DecorationBinding);
                    compiler.unset_decoration(info.id, spv::DecorationDescriptorSet);
                }
            }
        }

        hlslSource = compiler.compile();
    }

    const std::string& ShaderModule::GetHLSLSource() const {
        return hlslSource;
    }

}
}
