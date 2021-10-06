#include "cinder/app/App.h"
#include "cinder/app/RendererGx.h"

#include "cinder/graphics/wrapper.h"
#include "cinder/graphics/Buffer.h"
#include "cinder/graphics/DeviceContext.h"
#include "cinder/graphics/PipelineState.h"
#include "cinder/graphics/Shader.h"
#include "cinder/graphics/Texture.h"

#include "cinder/CinderDiligentImGui.h"

#include "TexturedCube.h"
#include "Utils.h"

using namespace ci;
using namespace ci::app;
using namespace std;

namespace ui = ImGui;

struct Constants {
    ci::mat4 WorldViewProj;
    ci::vec4 ViewportSize;
    float    LineWidth;
};

class GeometryShaderApp : public App {
  public:
	void setup() override;
	void update() override;
	void draw() override;

    void createPipelineState();
    void updateUI();

    static void prepareEngine( gx::RENDER_DEVICE_TYPE, gx::EngineCreateInfo*, gx::SwapChainDesc* );

    gx::PipelineStateRef         mPSO;
    gx::BufferRef                mCubeVertexBuffer;
    gx::BufferRef                mCubeIndexBuffer;
    gx::BufferRef                mShaderConstants;
    gx::TextureViewRef           mTextureSRV;
    gx::ShaderResourceBindingRef mSRB;

    mat4    mWorldViewProjMatrix;
    float   mLineWidth = 3.f;
};

void GeometryShaderApp::setup()
{
    ui::DiligentInitialize();
    disableFrameRate();

    createPipelineState();

    // Load textured cube
    mCubeVertexBuffer = TexturedCube::createVertexBuffer( getRenderDevice() );
    mCubeIndexBuffer  = TexturedCube::createIndexBuffer( getRenderDevice() );
    mTextureSRV       = TexturedCube::loadTexture( getRenderDevice(), getAssetPath( "DGLogo.png" ) )->GetDefaultView( gx::TEXTURE_VIEW_SHADER_RESOURCE );
    mSRB->GetVariableByName( gx::SHADER_TYPE_PIXEL, "g_Texture" )->Set( mTextureSRV );
}

void GeometryShaderApp::createPipelineState()
{
    // Create dynamic uniform buffer that will store shader constants
    mShaderConstants = gx::createBuffer( gx::BufferDesc()
        .name( "Shader constants CB" )
        .size( sizeof( Constants ) )
        .usage( gx::USAGE_DYNAMIC )
        .bindFlags( gx::BIND_UNIFORM_BUFFER )
        .cpuAccessFlags( gx::CPU_ACCESS_WRITE )
    );

    // Pipeline state object encompasses configuration of all GPU stages
    mPSO = gx::createGraphicsPipelineState( gx::GraphicsPipelineCreateInfo()
        // Pipeline state name is used by the engine to report issues.
        // It is always a good idea to give objects descriptive names.
		.name( "Cube PSO" )
        // Define vertex shader input layout
		.inputLayout( {
			// Attribute 0 - vertex position
			gx::LayoutElement{ 0, 0, 3, gx::VT_FLOAT32, false },
			// Attribute 1 - vertex uv
			gx::LayoutElement{ 1, 0, 2, gx::VT_FLOAT32, false }
			} )
		.vertexShader( gx::createShader( gx::ShaderCreateInfo()
			.name( "Cube VS" )
			.sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
			.shaderType( gx::SHADER_TYPE_VERTEX )
            .useCombinedTextureSamplers( true )
			.filePath( getAssetPath( "cube.vsh" ) )
		) )
        .geometryShader( gx::createShader( gx::ShaderCreateInfo()
            .name( "Cube GS" )
            .sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
            .shaderType( gx::SHADER_TYPE_GEOMETRY )
            .useCombinedTextureSamplers( true )
            .filePath( getAssetPath( "cube.gsh" ) )
        ) )
		.pixelShader( gx::createShader( gx::ShaderCreateInfo()
			.name( "Cube PS" )
			.sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
			.shaderType( gx::SHADER_TYPE_PIXEL )
			.useCombinedTextureSamplers( true )
            .filePath( getAssetPath( "cube.psh" ) )
		) )
        // Shader variables should typically be mutable, which means they are expected
        // to change on a per-instance basis
		.variables( {
			{ gx::SHADER_TYPE_PIXEL, "g_Texture", gx::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE }
			} )
        // Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
		.immutableSamplers( { { gx::SHADER_TYPE_PIXEL, "g_Texture", Diligent::SamplerDesc() } } )
        // Enable depth testing
        .depthStencilDesc( gx::DepthStencilStateDesc().depthEnable( true ) )
        // Cull back faces
        .rasterizerStateDesc( gx::RasterizerStateDesc().cullMode( gx::CULL_MODE_BACK ) ) 
    );

    // Since we did not explcitly specify the type for 'VSConstants', 'GSConstants', 
    // and 'PSConstants' variables, default type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used.
    // Static variables never change and are bound directly to the pipeline state object.
    mPSO->GetStaticVariableByName( gx::SHADER_TYPE_VERTEX, "VSConstants" )->Set( mShaderConstants );
    mPSO->GetStaticVariableByName( gx::SHADER_TYPE_GEOMETRY, "GSConstants" )->Set( mShaderConstants );
    mPSO->GetStaticVariableByName( gx::SHADER_TYPE_PIXEL, "PSConstants" )->Set( mShaderConstants );
    
    // Since we are using mutable variable, we must create a shader resource binding object
    // http://diligentgraphics.com/2016/03/23/resource-binding-model-in-diligent-engine-2-0/
    mPSO->CreateShaderResourceBinding( &mSRB, true );
}

void GeometryShaderApp::updateUI()
{
    ImGui::SetNextWindowPos( ImVec2( 10, 10 ), ImGuiCond_FirstUseEver );
    if( ImGui::Begin( "Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
        ImGui::SliderFloat( "Line Width", &mLineWidth, 1.f, 10.f );
    }
    ImGui::End();
}

void GeometryShaderApp::update()
{
    utils::updateWindowTitle();
    updateUI();

    // Apply rotation
    mat4 cubeModelTransform = glm::rotate( glm::pi<float>() * 0.1f, vec3( 1.0f, 0.0f, 0.0f ) ) * glm::rotate( (float) getElapsedSeconds(), vec3( 0.0f, 1.0f, 0.0f ) );
    // Camera is at (0, 0, -5) looking along the Z axis
    mat4 view = glm::lookAt( glm::vec3( 0.f, 0.0f, 5.0f ), vec3( 0.0f ), vec3( 0.0f, 1.0f, 0.0f ) );
    // Get projection matrix adjusted to the current screen orientation
    mat4 proj = glm::perspective( glm::pi<float>() / 4.0f, getWindowAspectRatio(), 0.1f, 100.f );
    // Compute world-view-projection matrix
    mWorldViewProjMatrix = proj * view * cubeModelTransform;
}

void GeometryShaderApp::draw()
{
    // Clear the back buffer
	gx::clear( ColorA( 0.350f, 0.350f, 0.350f, 1.0f ) );

    {
        // Map the buffer and write current world-view-projection matrix
        gx::MapHelper<Constants> constants( getImmediateContext(), mShaderConstants, gx::MAP_WRITE, gx::MAP_FLAG_DISCARD );
        constants->WorldViewProj = glm::transpose( mWorldViewProjMatrix );
        constants->ViewportSize = vec4( static_cast<float>( getWindowWidth() ), static_cast<float>( getWindowHeight() ), 1.f / static_cast<float>( getWindowWidth() ), 1.f / static_cast<float>( getWindowHeight() ) );
        constants->LineWidth = mLineWidth;
    }
    // Bind vertex and index buffers
    gx::setVertexBuffer( mCubeVertexBuffer, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, gx::SET_VERTEX_BUFFERS_FLAG_RESET );
    gx::setIndexBuffer( mCubeIndexBuffer, 0, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );

    // Set the pipeline state
    gx::setPipelineState( mPSO );
    // Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode
    // makes sure that resources are transitioned to required states.
    gx::commitShaderResources( mSRB, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
    // draw
    gx::drawIndexed( gx::DrawIndexedAttribs().indexType( gx::VT_UINT32 ).numIndices( 36 ).flags( gx::DRAW_FLAG_VERIFY_ALL ) );
}

void GeometryShaderApp::prepareEngine( gx::RENDER_DEVICE_TYPE deviceType, gx::EngineCreateInfo* engineCreateInfo, gx::SwapChainDesc* swapChainDesc )
{
    engineCreateInfo->Features.GeometryShaders    = gx::DEVICE_FEATURE_STATE_ENABLED;
    engineCreateInfo->Features.SeparablePrograms = gx::DEVICE_FEATURE_STATE_ENABLED;
}

CINDER_APP( GeometryShaderApp, RendererGx( RendererGx::Options().prepareEngineFn( GeometryShaderApp::prepareEngine ) ) )
