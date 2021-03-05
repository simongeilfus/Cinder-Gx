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

#include "cinder/graphics/Shader.h"
#include "cinder/app/RendererGx.h"

using namespace std;
using namespace ci::app;

namespace cinder { namespace graphics {

ShaderCreateInfo::ShaderCreateInfo()
	: mMacros( make_shared<ShaderMacroHelper>() )
{
	pShaderSourceStreamFactory = app::getDefaultShaderSourceStreamFactory();
}

ShaderCreateInfo::ShaderCreateInfo( const ShaderCreateInfo &other )
	: mFilePath( other.mFilePath ),
	mSource( other.mSource ),
	mEntryPoint( other.mEntryPoint ),
	mCombinedSamplerSuffix( other.mCombinedSamplerSuffix ),
	mName( other.mName ),
	mMacros( other.mMacros )
{
    FilePath = other.FilePath;
    pShaderSourceStreamFactory = other.pShaderSourceStreamFactory;
    ppConversionStream = other.ppConversionStream;
    Source = other.Source;
    ByteCode = other.ByteCode;
    ByteCodeSize = other.ByteCodeSize;
    EntryPoint = other.EntryPoint;
    Macros = other.Macros;
    UseCombinedTextureSamplers = other.UseCombinedTextureSamplers;
    CombinedSamplerSuffix = other.CombinedSamplerSuffix;
    Desc = other.Desc;
    SourceLanguage = other.SourceLanguage;
    ShaderCompiler = other.ShaderCompiler;
    HLSLVersion = other.HLSLVersion;
    GLSLVersion = other.GLSLVersion;
    GLESSLVersion = other.GLESSLVersion;
    ppCompilerOutput = other.ppCompilerOutput;
	
	updatePtrs();
}

ShaderCreateInfo::ShaderCreateInfo( ShaderCreateInfo &&other ) noexcept
	: ShaderCreateInfo()
{
	other.swap( *this );
	updatePtrs();
}

ShaderCreateInfo& ShaderCreateInfo::operator=( const ShaderCreateInfo &other )
{
	ShaderCreateInfo( other ).swap( *this );
	updatePtrs();
	return *this;
}

ShaderCreateInfo& ShaderCreateInfo::operator=( ShaderCreateInfo &&other ) noexcept
{
	other.swap( *this );
	updatePtrs();
	return *this;
}

void ShaderCreateInfo::swap( ShaderCreateInfo &other ) noexcept
{
	std::swap( mFilePath, other.mFilePath );
	std::swap( mSource, other.mSource );
	std::swap( mEntryPoint, other.mEntryPoint );
	std::swap( mCombinedSamplerSuffix, other.mCombinedSamplerSuffix );
	std::swap( mName, other.mName );
	std::swap( mMacros, other.mMacros );
	
    std::swap( FilePath, other.FilePath );
    std::swap( pShaderSourceStreamFactory, other.pShaderSourceStreamFactory );
    std::swap( ppConversionStream, other.ppConversionStream );
    std::swap( Source, other.Source );
    std::swap( ByteCode, other.ByteCode );
    std::swap( ByteCodeSize, other.ByteCodeSize );
    std::swap( EntryPoint, other.EntryPoint );
    std::swap( Macros, other.Macros );
    std::swap( UseCombinedTextureSamplers, other.UseCombinedTextureSamplers );
    std::swap( CombinedSamplerSuffix, other.CombinedSamplerSuffix );
    std::swap( Desc, other.Desc );
    std::swap( SourceLanguage, other.SourceLanguage );
    std::swap( ShaderCompiler, other.ShaderCompiler );
    std::swap( HLSLVersion, other.HLSLVersion );
    std::swap( GLSLVersion, other.GLSLVersion );
    std::swap( GLESSLVersion, other.GLESSLVersion );
    std::swap( ppCompilerOutput, other.ppCompilerOutput );
}

void ShaderCreateInfo::updatePtrs() noexcept
{
	if( ! mFilePath.empty() ) FilePath = mFilePath.c_str();
	if( ! mSource.empty() ) Source = mSource.c_str();
	if( ! mEntryPoint.empty() ) EntryPoint = mEntryPoint.c_str();
	if( ! mCombinedSamplerSuffix.empty() ) CombinedSamplerSuffix = mCombinedSamplerSuffix.c_str();
	if( ! mName.empty() ) Desc.Name = mName.c_str();
	if( mMacros ) Macros = *mMacros;
}

ShaderRef createShader( const Diligent::ShaderCreateInfo &shaderCI )
{
	ShaderRef shader;
	getRenderDevice()->CreateShader( shaderCI, &shader );
	return shader;
}

}

namespace gx = graphics;
} // namespace cinder::graphics