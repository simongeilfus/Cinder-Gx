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

#include "cinder/graphics/Batch.h"
#include "cinder/graphics/wrapper.h"
#include "cinder/app/RendererGx.h"

using namespace std;
using namespace ci::app;

namespace cinder { namespace graphics {

Batch::Batch( const PipelineStateRef &pipelineState )
	: Batch( app::getRenderDevice(), pipelineState )
{
}

Batch::Batch( const GraphicsPipelineStateCreateInfo &pipelineCreateInfo )
	: Batch( app::getRenderDevice(), pipelineCreateInfo )
{
}

Batch::Batch( const Mesh &mesh, const PipelineStateRef &pipelineState )
	: Batch( app::getRenderDevice(), mesh, pipelineState )
{
}

Batch::Batch( const Mesh &mesh, const GraphicsPipelineStateCreateInfo &pipelineCreateInfo )
	: Batch( app::getRenderDevice(), mesh, pipelineCreateInfo )
{
}

Batch::Batch( const std::vector<Mesh> &meshes, const PipelineStateRef &pipelineState )
	: Batch( app::getRenderDevice(), meshes, pipelineState )
{
}

Batch::Batch( const std::vector<Mesh> &meshes, const GraphicsPipelineStateCreateInfo &pipelineCreateInfo )
	: Batch( app::getRenderDevice(), meshes, pipelineCreateInfo )
{
}

namespace {
	PipelineStateRef createPipeline( RenderDevice* device, const GraphicsPipelineStateCreateInfo &pipelineCreateInfo )
	{
		return createGraphicsPipelineState( device, pipelineCreateInfo );
	}

	PipelineStateRef createPipeline( RenderDevice* device, GraphicsPipelineStateCreateInfo pipelineCreateInfo, const Mesh &mesh )
	{
		if( pipelineCreateInfo.getLayoutElements().empty() ) {
			pipelineCreateInfo.inputLayout( mesh.getVertexLayoutElements() );
		}
		return createGraphicsPipelineState( device, pipelineCreateInfo );
	}
}

Batch::Batch( RenderDevice* device, const PipelineStateRef &pipelineState )
	: mDevice( device ),
	mPso( pipelineState )
{
}

Batch::Batch( RenderDevice* device, const GraphicsPipelineStateCreateInfo &pipelineCreateInfo )
	: Batch( device, createPipeline( device, pipelineCreateInfo ) )
{
}

Batch::Batch( RenderDevice* device, const Mesh &mesh, const PipelineStateRef &pipelineState )
	: Batch( device, std::vector<Mesh>{ mesh }, pipelineState )
{
}

Batch::Batch( RenderDevice* device, const Mesh &mesh, const GraphicsPipelineStateCreateInfo &pipelineCreateInfo )
	: Batch( device, std::vector<Mesh>{ mesh }, createPipeline( device, pipelineCreateInfo, mesh ) )
{
}

Batch::Batch( RenderDevice* device, const std::vector<Mesh> &meshes, const PipelineStateRef &pipelineState )
	: mDevice( device ),
	mPso( pipelineState ),
	mMeshes( meshes )
{
}

Batch::Batch( RenderDevice* device, const std::vector<Mesh> &meshes, const GraphicsPipelineStateCreateInfo &pipelineCreateInfo )
	: Batch( device, meshes, createPipeline( device, pipelineCreateInfo, meshes.front() ) ) // should all the meshes participate in choosing the input layout (instead of meshes.front()) ?
{
}

void Batch::setStaticVariable( SHADER_TYPE shaderType, const char* name, IDeviceObject* pObject )
{
	if( ShaderResourceVariable* variable = mPso->GetStaticVariableByName( shaderType, name ) ) {
		variable->Set( pObject );
	}
}

void Batch::setStaticVariable( SHADER_TYPE shaderType, uint32_t index, IDeviceObject* pObject )
{
	if( ShaderResourceVariable* variable = mPso->GetStaticVariableByIndex( shaderType, index ) ) {
		variable->Set( pObject );
	}
}

uint32_t Batch::getStaticVariableCount( SHADER_TYPE shaderType ) const
{
	return mPso->GetStaticVariableCount( shaderType );
}

ShaderResourceVariable* Batch::getStaticVariable( SHADER_TYPE shaderType, const char* name )
{
	return mPso->GetStaticVariableByName( shaderType, name );
}

ShaderResourceVariable* Batch::getStaticVariable( SHADER_TYPE shaderType, uint32_t index )
{
	return mPso->GetStaticVariableByIndex( shaderType, index );
}

void Batch::setVariable( SHADER_TYPE shaderType, const char* name, IDeviceObject* pObject )
{
	if( ShaderResourceVariable* variable = getShaderResourceBinding()->GetVariableByName( shaderType, name ) ) {
		variable->Set( pObject );
	}
}

void Batch::setVariable( SHADER_TYPE shaderType, uint32_t index, IDeviceObject* pObject )
{
	if( ShaderResourceVariable* variable = getShaderResourceBinding()->GetVariableByIndex( shaderType, index ) ) {
		variable->Set( pObject );
	}
}

uint32_t Batch::getVariableCount( SHADER_TYPE shaderType )
{
	return getShaderResourceBinding()->GetVariableCount( shaderType );
}

ShaderResourceVariable* Batch::getVariable( SHADER_TYPE shaderType, const char* name )
{
	return getShaderResourceBinding()->GetVariableByName( shaderType, name );
}

ShaderResourceVariable* Batch::getVariable( SHADER_TYPE shaderType, uint32_t index )
{
	return getShaderResourceBinding()->GetVariableByIndex( shaderType, index );
}

ShaderResourceBindingRef Batch::getShaderResourceBinding()
{
	if( ! mSrb ) {
		mPso->CreateShaderResourceBinding( &mSrb, true );
	}
	return mSrb;
}

void Batch::draw()
{
	draw( app::getImmediateContext() );
}

void Batch::draw( DeviceContext* context )
{
	context->SetPipelineState( mPso );
	context->CommitShaderResources( getShaderResourceBinding(), RESOURCE_STATE_TRANSITION_MODE_TRANSITION );

	for( const Mesh &mesh : mMeshes ) {
		mesh.draw( context );
	}
}

}

namespace gx = graphics;
} // namespace cinder::graphics