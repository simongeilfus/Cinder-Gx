#include "cinder/app/App.h"
#include "cinder/app/RendererGx.h"

#include "cinder/graphics/wrapper.h"
#include "cinder/graphics/Buffer.h"
#include "cinder/graphics/DeviceContext.h"
#include "cinder/graphics/PipelineState.h"
#include "cinder/graphics/Shader.h"
#include "cinder/graphics/Texture.h"

#include "TexturedCube.h"
#include "Utils.h"

// TODO / NOTES: Still need to add the pre-transform process for mobile. See lines 239-140 

using namespace ci;
using namespace ci::app;
using namespace std;

class RenderTargetApp : public App {
  public:
	void setup() override;
	void update() override;
	void draw() override;
	void resize() override;

	void createCubePSO();
	void createRenderTargetPSO();

    static void prepareEngine( gx::RENDER_DEVICE_TYPE, gx::EngineCreateInfo*, gx::SwapChainDesc* );

    static constexpr gx::TEXTURE_FORMAT sRenderTargetFormat = gx::TEX_FORMAT_RGBA8_UNORM;
    static constexpr gx::TEXTURE_FORMAT sDepthBufferFormat  = gx::TEX_FORMAT_D32_FLOAT;

    // Cube resources
    gx::PipelineStateRef         mCubePSO;
    gx::ShaderResourceBindingRef mCubeSRB;
    gx::BufferRef                mCubeVertexBuffer;
    gx::BufferRef                mCubeIndexBuffer;
    gx::BufferRef                mCubeVSConstants;
    gx::TextureViewRef           mCubeTextureSRV;

    // Offscreen render target and depth-stencil
    gx::TextureViewRef mColorRTV;
    gx::TextureViewRef mDepthDSV;

    gx::BufferRef                mRTPSConstants;
    gx::PipelineStateRef         mRTPSO;
    gx::ShaderResourceBindingRef mRTSRB;
    mat4                         mWorldViewProjMatrix;
};

void RenderTargetApp::setup()
{
	disableFrameRate();

    createCubePSO();
    createRenderTargetPSO();
    // Load textured cube
    mCubeVertexBuffer = TexturedCube::createVertexBuffer( getRenderDevice() );
    mCubeIndexBuffer  = TexturedCube::createIndexBuffer( getRenderDevice() );
    mCubeTextureSRV   = TexturedCube::loadTexture( getRenderDevice(), getAssetPath( "DGLogo.png" ) )->GetDefaultView( gx::TEXTURE_VIEW_SHADER_RESOURCE );
    // Set cube texture SRV in the SRB
    mCubeSRB->GetVariableByName( gx::SHADER_TYPE_PIXEL, "g_Texture" )->Set( mCubeTextureSRV );
}

void RenderTargetApp::resize()
{
    // Create window-size offscreen render target
    gx::TextureRef pRTColor = gx::createTexture( gx::TextureDesc()
        .name( "Offscreen render target" )
        .type( gx::RESOURCE_DIM_TEX_2D )
        .size( getWindowSize() )
        .mipLevels( 1 )
        .format( sRenderTargetFormat )
        // The render target can be bound as a shader resource and as a render target
        .bindFlags( gx::BIND_SHADER_RESOURCE | gx::BIND_RENDER_TARGET )
        // Define optimal clear value
        .clearValue( { sRenderTargetFormat, { 0.35f, 0.35f, 0.35f, 1.0f } } ) 
    );
    // Store the render target view
    mColorRTV = pRTColor->GetDefaultView( gx::TEXTURE_VIEW_RENDER_TARGET );

    // Create window-size depth buffer
    gx::TextureRef pRTDepth = gx::createTexture( gx::TextureDesc()
        .name( "Offscreen depth buffer" )
        .type( gx::RESOURCE_DIM_TEX_2D )
        .size( getWindowSize() )
        .mipLevels( 1 )
        .format( sDepthBufferFormat )
        .bindFlags( gx::BIND_DEPTH_STENCIL )
        // Define optimal clear value
        .clearValue( { sDepthBufferFormat, { 0.35f, 0.35f, 0.35f, 1.0f } } )
    );

    // Store the depth-stencil view
    mDepthDSV = pRTDepth->GetDefaultView( gx::TEXTURE_VIEW_DEPTH_STENCIL );

    // We need to release and create a new SRB that references new off-screen render target SRV
    mRTSRB.Release();
    mRTPSO->CreateShaderResourceBinding( &mRTSRB, true );

    // Set render target color texture SRV in the SRB
    mRTSRB->GetVariableByName( gx::SHADER_TYPE_PIXEL, "g_Texture" )->Set( pRTColor->GetDefaultView( gx::TEXTURE_VIEW_SHADER_RESOURCE ) );
}

void RenderTargetApp::createCubePSO()
{
    mCubePSO = TexturedCube::createPipelineState( getRenderDevice(), sRenderTargetFormat, sDepthBufferFormat, getAssetPath( "cube.vsh" ), getAssetPath( "cube.psh" ) );

    // Create dynamic uniform buffer that will store our transformation matrix
    // Dynamic buffers can be frequently updated by the CPU
    mCubeVSConstants = gx::createBuffer( gx::BufferDesc()
        .name( "VS constants CB" )
        .size( sizeof( mat4 ) )
        .usage( gx::USAGE_DYNAMIC )
        .bindFlags( gx::BIND_UNIFORM_BUFFER )
        .cpuAccessFlags( gx::CPU_ACCESS_WRITE )
    );

    // Since we did not explcitly specify the type for 'Constants' variable, default
    // type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables never
    // change and are bound directly through the pipeline state object.
    mCubePSO->GetStaticVariableByName( gx::SHADER_TYPE_VERTEX, "Constants" )->Set( mCubeVSConstants );

    // Since we are using mutable variable, we must create a shader resource binding object
    // http://diligentgraphics.com/2016/03/23/resource-binding-model-in-diligent-engine-2-0/
    mCubePSO->CreateShaderResourceBinding( &mCubeSRB, true );
}

void RenderTargetApp::createRenderTargetPSO()
{  
    // Create dynamic uniform buffer that will store our transformation matrix
    // Dynamic buffers can be frequently updated by the CPU
    mRTPSConstants = gx::createBuffer( gx::BufferDesc()
        .name( "RTPS constants CB" )
        .size( sizeof( mat4 ) + sizeof( mat2 ) * 2 )
        .usage( gx::USAGE_DYNAMIC )
        .bindFlags( gx::BIND_UNIFORM_BUFFER )
        .cpuAccessFlags( gx::CPU_ACCESS_WRITE )
    );

#if PLATFORM_ANDROID
    // Vulkan on mobile platforms may require handling surface pre-transforms
    const bool transformUVCoords = getRenderDevice()->GetDeviceCaps().IsVulkanDevice();
#else
    constexpr bool transformUVCoords = false;
#endif

    mRTPSO = gx::createGraphicsPipelineState( gx::GraphicsPipelineCreateInfo()
        // Pipeline state name is used by the engine to report issues.
        // It is always a good idea to give objects descriptive names.
        .name( "Render Target PSO" )
        // Define vertex shader input layout
        .inputLayout( {
            // Attribute 0 - vertex position
            gx::LayoutElement{ 0, 0, 3, gx::VT_FLOAT32, false },
            // Attribute 1 - vertex uv
            gx::LayoutElement{ 1, 0, 2, gx::VT_FLOAT32, false }
            } )
        .vertexShader( gx::createShader( gx::ShaderCreateInfo()
            .name( "Render Target VS" )
            .sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
            .shaderType( gx::SHADER_TYPE_VERTEX )
            .useCombinedTextureSamplers( true )
            .filePath( getAssetPath( "rendertarget.vsh" ) )
        ) )
        .pixelShader( gx::createShader( gx::ShaderCreateInfo()
            .name( "Render Target PS" )
            .sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
            .shaderType( gx::SHADER_TYPE_PIXEL )
            .useCombinedTextureSamplers( true )
            .macro( "TRANSFORM_UV", transformUVCoords )
            .filePath( getAssetPath( "rendertarget.psh" ) )
        ) )
        // Shader variables should typically be mutable, which means they are expected
        // to change on a per-instance basis
        .variables( {
            { gx::SHADER_TYPE_PIXEL, "g_Texture", gx::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE }
            } )
        // Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
        .immutableSamplers( { { gx::SHADER_TYPE_PIXEL, "g_Texture", gx::Sam_LinearClamp } } )
        // Primitive topology defines what kind of primitives will be rendered by this pipeline state
        .primitiveTopology( gx::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP )
        // Enable depth testing
        .depthStencilDesc( gx::DepthStencilStateDesc().depthEnable( true ) )
        // Cull back faces
        .rasterizerStateDesc( gx::RasterizerStateDesc().cullMode( gx::CULL_MODE_BACK ) )
    );

    // Since we did not explcitly specify the type for Constants, default type
    // (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables never change and are bound directly
    // to the pipeline state object.
    mRTPSO->GetStaticVariableByName( gx::SHADER_TYPE_PIXEL, "Constants" )->Set( mRTPSConstants );
}

void RenderTargetApp::update()
{
	utils::updateWindowTitle();

    // Apply rotation
    mat4 cubeModelTransform = glm::rotate( glm::pi<float>() * 0.1f, vec3( 1.0f, 0.0f, 0.0f ) ) * glm::rotate( (float) getElapsedSeconds(), vec3( 0.0f, 1.0f, 0.0f ) );
    // Camera is at (0, 0, -5) looking along the Z axis
    mat4 view = glm::lookAt( glm::vec3( 0.f, 0.0f, 5.0f ), vec3( 0.0f ), vec3( 0.0f, 1.0f, 0.0f ) );
    // Get projection matrix adjusted to the current screen orientation
    mat4 proj = glm::perspective( glm::pi<float>() / 4.0f, getWindowAspectRatio(), 0.1f, 100.f );
    // Compute world-view-projection matrix
    mWorldViewProjMatrix = proj * view * cubeModelTransform;
}

void RenderTargetApp::draw()
{

    // Clear the offscreen render target and depth buffer
    const float clearColor[] ={ 0.350f, 0.350f, 0.350f, 1.0f };
    gx::setRenderTargets( 1, &mColorRTV, mDepthDSV, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
    gx::clearRenderTarget( mColorRTV, clearColor, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
    gx::clearDepthStencil( mDepthDSV, gx::CLEAR_DEPTH_FLAG, 1.0f, 0, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );

    {
        // Map the cube's constant buffer and fill it in with its model-view-projection matrix
        gx::MapHelper<mat4> constants( getImmediateContext(), mCubeVSConstants, gx::MAP_WRITE, gx::MAP_FLAG_DISCARD );
        *constants = glm::transpose( mWorldViewProjMatrix );
    }

    {
        struct VSConstants {
            float Time;
            float Padding0;
            float Padding1;
            float Padding2;

            mat2 UVPreTransform;
            mat2 UVPreTransformInv;
        };
        // Map the render target PS constant buffer and fill it in with current time
        gx::MapHelper<VSConstants> cbConstants( getImmediateContext(), mRTPSConstants, gx::MAP_WRITE, gx::MAP_FLAG_DISCARD );
        cbConstants->Time              = static_cast<float>( getElapsedSeconds() );
        //cbConstants->UVPreTransform    = mUVPreTransformMatrix;
        //cbConstants->UVPreTransformInv = mUVPreTransformMatrix.Inverse();
    }

    // Bind vertex and index buffers
    gx::setVertexBuffer( mCubeVertexBuffer, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, gx::SET_VERTEX_BUFFERS_FLAG_RESET );
    gx::setIndexBuffer( mCubeIndexBuffer, 0, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
    // Set the cube's pipeline state
    gx::setPipelineState( mCubePSO );
    // Commit the cube shader's resources
    gx::commitShaderResources( mCubeSRB, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
    // Draw the cube
    gx::drawIndexed( gx::DrawIndexedAttribs().indexType( gx::VT_UINT32 ).numIndices( 36 ).flags( gx::DRAW_FLAG_VERIFY_ALL ) );

    
    // Clear the default render target
    auto* pRTV = getSwapChain()->GetCurrentBackBufferRTV();
    const float zero[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    gx::setRenderTargets( 1, &pRTV, getSwapChain()->GetDepthBufferDSV(), gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
    gx::clearRenderTarget( pRTV, zero, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
    // Set the render target pipeline state
    gx::setPipelineState( mRTPSO );
    // Commit the render target shader's resources
    gx::commitShaderResources( mRTSRB, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
    // Draw the render target's vertices
    gx::draw( gx::DrawAttribs().numVertices( 4 ).flags( gx::DRAW_FLAG_VERIFY_ALL ) );
}

void RenderTargetApp::prepareEngine( gx::RENDER_DEVICE_TYPE deviceType, gx::EngineCreateInfo* engineCreateInfo, gx::SwapChainDesc* swapChainDesc )
{
    // In this tutorial we will be using off-screen depth-stencil buffer, so
    // we do not need the one in the swap chain.
    swapChainDesc->DepthBufferFormat = gx::TEX_FORMAT_UNKNOWN;
}

CINDER_APP( RenderTargetApp, RendererGx( RendererGx::Options().prepareEngineFn( RenderTargetApp::prepareEngine ) ) )
