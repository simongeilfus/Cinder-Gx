/*
 Copyright (c) 2021, The Cinder Project, All rights reserved.

 This code is intended for use with the Cinder C++ library: http://libcinder.org

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

	* Redistributions of source code must retain the above copyright notice, this list of conditions and
	   the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	   the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include "cinder/Filesystem.h"
#include "cinder/graphics/wrapper.h"

#include "DiligentCore/Graphics/GraphicsTools/interface/ShaderMacroHelper.hpp"

namespace cinder { namespace graphics {

struct ShaderCreateInfo : public Diligent::ShaderCreateInfo {
    //! Source file path. If source file path is provided, Source and ByteCode members must be null
    ShaderCreateInfo& filePath( const char* filePath ) { FilePath = filePath; return *this; }
    //! Source file path. If source file path is provided, Source and ByteCode members must be null
    ShaderCreateInfo& filePath( const ci::fs::path &filePath ) { mFilePath = filePath.string(); FilePath = mFilePath.c_str(); return *this; }
    //! Pointer to the shader source input stream factory. The factory is used to load the shader source file if FilePath is not null. It is also used to create additional input streams for shader include files
    ShaderCreateInfo& shaderSourceStreamFactory( Diligent::IShaderSourceInputStreamFactory* shaderSourceStreamFactory ) { pShaderSourceStreamFactory = shaderSourceStreamFactory; return *this; }
    /// HLSL->GLSL conversion stream
    ShaderCreateInfo& conversionStream( struct Diligent::IHLSL2GLSLConversionStream** conversionStream ) { ppConversionStream = conversionStream; return *this; }
    //! Shader source. If shader source is provided, FilePath and ByteCode members must be null
    ShaderCreateInfo& source( const char* source ) { Source = source; return *this; }
    //! Compiled shader bytecode.  If shader byte code is provided, FilePath and Source members must be null
    ShaderCreateInfo& byteCode( const void* byteCode ) { ByteCode = byteCode; return *this; }
    //! Size of the compiled shader bytecode. Byte code size (in bytes) must be provided if ByteCode is not null
    ShaderCreateInfo& byteCodeSize( size_t byteCodeSize ) { ByteCodeSize = byteCodeSize; return *this; }
    //! Shader entry point. This member is ignored if ByteCode is not null
    ShaderCreateInfo& entryPoint( const char* entryPoint ) { EntryPoint = entryPoint; return *this; }
    //! Shader macros. This member is ignored if ByteCode is not null
    ShaderCreateInfo& macros( const Diligent::ShaderMacro* macros ) { Macros = macros; return *this; }
    //! If set to true, textures will be combined with texture samplers.
    ShaderCreateInfo& useCombinedTextureSamplers( bool useCombinedTextureSamplers ) { UseCombinedTextureSamplers = useCombinedTextureSamplers; return *this; }
    //! If UseCombinedTextureSamplers is true, defines the suffix added to the texture variable name to get corresponding sampler name. For example, for default value "_sampler", a texture named "tex" will be combined with sampler named "tex_sampler". If UseCombinedTextureSamplers is false, this member is ignored.
    ShaderCreateInfo& combinedSamplerSuffix( const char* combinedSamplerSuffix ) { CombinedSamplerSuffix = combinedSamplerSuffix; return *this; }
    //! Shader type. See Diligent::SHADER_TYPE.
    ShaderCreateInfo& shaderType( SHADER_TYPE shaderType ) { Desc.ShaderType = shaderType; return *this; }
    //! Shader source language. See Diligent::SHADER_SOURCE_LANGUAGE.
    ShaderCreateInfo& sourceLanguage( SHADER_SOURCE_LANGUAGE sourceLanguage ) { SourceLanguage = sourceLanguage; return *this; }
    //! Shader compiler. See Diligent::SHADER_COMPILER.
    ShaderCreateInfo& shaderCompiler( SHADER_COMPILER shaderCompiler ) { ShaderCompiler = shaderCompiler; return *this; }
    //! HLSL shader model to use when compiling the shader. When default value is given (0, 0), the engine will attempt to use the highest HLSL shader model supported by the device. If the shader is created from the byte code, this value has no effect.
    ShaderCreateInfo& hlslVersion( ShaderVersion hlslVersion ) { HLSLVersion = hlslVersion; return *this; }
    //! GLSL version to use when creating the shader. When default value is given (0, 0), the engine will attempt to use the highest GLSL version supported by the device.
    ShaderCreateInfo& glslVersion( ShaderVersion glslVersion ) { GLSLVersion = glslVersion; return *this; }
    //! GLES shading language version to use when creating the shader. When default value is given (0, 0), the engine will attempt to use the highest GLESSL version supported by the device.
    ShaderCreateInfo& glesslVersion( ShaderVersion glesslVersion ) { GLESSLVersion = glesslVersion; return *this; }
    //! Memory address where pointer to the compiler messages data blob will be written
    ShaderCreateInfo& compilerOutput( struct Diligent::IDataBlob** compilerOutput ) { ppCompilerOutput = compilerOutput; return *this; }
    //! Speficies the object's name.
    ShaderCreateInfo& name( const char* name ) { Desc.Name = name; return *this; }

    ShaderCreateInfo();
protected:
    std::string mFilePath;
};

//! Creates a new shader object using the default RenderDevice
CI_API ShaderRef createShader( const Diligent::ShaderCreateInfo &shaderCI );

}

namespace gx = graphics;
} // namespace cinder::graphics