#include "cinder/app/App.h"
#include "cinder/app/RendererGx.h"

#include "cinder/graphics/wrapper.h"
#include "cinder/graphics/Buffer.h"
#include "cinder/graphics/DeviceContext.h"
#include "cinder/graphics/PipelineState.h"
#include "cinder/graphics/Shader.h"
#include "cinder/graphics/Texture.h"

#include "cinder/CinderDiligentImGui.h"

#include "Utils.h"

using namespace ci;
using namespace ci::app;
using namespace std;

struct GlobalConstants {
    unsigned int numHorzBlocks; // Number of blocks along the horizontal edge
    unsigned int numVertBlocks; // Number of blocks along the horizontal edge
    float        fnumHorzBlocks;
    float        fnumVertBlocks;

    float   fBlockSize;
    float   LengthScale;
    float   HeightScale;
    float   LineWidth;

    float   TessDensity;
    int     AdaptiveTessellation;
    vec2    Dummy2;

    mat4    WorldView;
    mat4    WorldViewProj;
    vec4    ViewportSize;
};

class TessellationApp : public App {
  public:
	void setup() override;
	void update() override;
	void draw() override;

    void createPipelineStates();
    void loadTextures();
    void updateUI();

    static void prepareEngine( gx::RENDER_DEVICE_TYPE, gx::EngineCreateInfo*, gx::SwapChainDesc* );

    gx::PipelineStateRef         mPSO[2];
    gx::ShaderResourceBindingRef mSRB[2];
    gx::BufferRef                mShaderConstants;
    gx::TextureViewRef           mHeightMapSRV;
    gx::TextureViewRef           mColorMapSRV;

    mat4 mWorldViewProjMatrix;
    mat4 mWorldViewMatrix;

    bool  mAnimate              = true;
    bool  mWireframe            = false;
    float mRotationAngle        = 0;
    float mTessDensity          = 32;
    float mDistance             = 10.f;
    bool  mAdaptiveTessellation = true;
    int   mBlockSize            = 32;

    unsigned int mHeightMapWidth  = 0;
    unsigned int mHeightMapHeight = 0;
};

void TessellationApp::setup()
{
    ImGui::DiligentInitialize();
    disableFrameRate();

    createPipelineStates();
    loadTextures();
}

void TessellationApp::createPipelineStates()
{
    const bool bWireframeSupported = getRenderDevice()->GetDeviceCaps().Features.GeometryShaders;
    
    // Create a vertex shader
    gx::ShaderRef pVS = gx::createShader( gx::ShaderCreateInfo()
        .name( "Terrain VS" )
        .sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
        .shaderType( gx::SHADER_TYPE_VERTEX )
        .filePath( getAssetPath( "terrain.vsh" ) )
        .entryPoint( "TerrainVS" )
    );
    // Create a geometry shader
    gx::ShaderRef pGS;
    if( bWireframeSupported ) {
        pGS = gx::createShader( gx::ShaderCreateInfo()
            .name( "Terrain GS" )
            .sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
            .shaderType( gx::SHADER_TYPE_GEOMETRY )
            .filePath( getAssetPath( "terrain.gsh" ) )
            .entryPoint( "TerrainGS" )
        );
    }
    // Create a hull shader
    gx::ShaderRef pHS = gx::createShader( gx::ShaderCreateInfo()
        .name( "Terrain HS" )
        .sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
        .shaderType( gx::SHADER_TYPE_HULL )
        .useCombinedTextureSamplers( true )
        .filePath( getAssetPath( "terrain.hsh" ) )
        .entryPoint( "TerrainHS" )
    );
    // Create a domain shader
    gx::ShaderRef pDS = gx::createShader( gx::ShaderCreateInfo()
        .name( "Terrain DS" )
        .sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
        .shaderType( gx::SHADER_TYPE_DOMAIN )
        .useCombinedTextureSamplers( true )
        .filePath( getAssetPath( "terrain.dsh" ) )
        .entryPoint( "TerrainDS" )
    );
    // Create a pixel shader
    gx::ShaderRef pPS, pWirePS;
    pPS = gx::createShader( gx::ShaderCreateInfo()
        .name( "Terrain PS" )
        .sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
        .shaderType( gx::SHADER_TYPE_PIXEL )
        .useCombinedTextureSamplers( true )
        .filePath( getAssetPath( "terrain.psh" ) )
        .entryPoint( "TerrainPS" )
    );
    if( bWireframeSupported ) {
        pWirePS = gx::createShader( gx::ShaderCreateInfo()
            .name( "Wireframe Terrain PS" )
            .sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
            .shaderType( gx::SHADER_TYPE_PIXEL )
            .useCombinedTextureSamplers( true )
            .filePath( getAssetPath( "terrain_wire.psh" ) )
            .entryPoint( "WireTerrainPS" )
        );
    }
    // Sampler description used in the immutable sampler bellow
    gx::SamplerDesc samLinearClampDesc {
        gx::FILTER_TYPE_LINEAR, gx::FILTER_TYPE_LINEAR, gx::FILTER_TYPE_LINEAR,
        gx::TEXTURE_ADDRESS_CLAMP, gx::TEXTURE_ADDRESS_CLAMP, gx::TEXTURE_ADDRESS_CLAMP
    };

    // Pipeline state object encompasses configuration of all GPU stages
    gx::GraphicsPipelineDesc psoCreateInfo = gx::GraphicsPipelineDesc()
        // Pipeline state name is used by the engine to report issues.
        // It is always a good idea to give objects descriptive names.
        .name( "Terrain PSO" )
        // Create a vertex shader
        .vertexShader( pVS )
        // Create a hull shader
        .hullShader( pHS )
        // Create a domain shader
        .domainShader( pDS )
        // Create a pixel shader
        .pixelShader( pPS )
        // Shader variables should typically be mutable, which means they are expected
        // to change on a per-instance basis
        .variables( {
            { gx::SHADER_TYPE_HULL | gx::SHADER_TYPE_DOMAIN,  "g_HeightMap", gx::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE },
            { gx::SHADER_TYPE_PIXEL,                          "g_Texture",   gx::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE }
        } )
        // Define immutable sampler for g_HeightMap and g_Texture. Immutable samplers should be used whenever possible
        .immutableSamplers( {
            { gx::SHADER_TYPE_HULL | gx::SHADER_TYPE_DOMAIN, "g_HeightMap", samLinearClampDesc},
            { gx::SHADER_TYPE_PIXEL,                         "g_Texture",   samLinearClampDesc}
        } )
        // Enable depth testing
        .depthStencilDesc( gx::DepthStencilStateDesc().depthEnable( true ) )
        // Cull back faces
        .rasterizerStateDesc( gx::RasterizerStateDesc().cullMode( getRenderDevice()->GetDeviceCaps().IsGLDevice() ? gx::CULL_MODE_FRONT : gx::CULL_MODE_BACK ) )
        // Primitive topology type defines what kind of primitives will be rendered by this pipeline state
        .primitiveTopology( gx::PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST );

    // Create dynamic uniform buffer that will store shader constants
    mShaderConstants = gx::createBuffer( gx::BufferDesc()
        .name( "Global shader constants CB" )
        .sizeInBytes( sizeof( GlobalConstants ) )
        .usage( gx::USAGE_DYNAMIC )
        .bindFlags( gx::BIND_UNIFORM_BUFFER )
        .cpuAccessFlags( gx::CPU_ACCESS_WRITE )
    );


    mPSO[0] = gx::createGraphicsPipelineState( psoCreateInfo );
    if( bWireframeSupported ) {
        mPSO[1] = gx::createGraphicsPipelineState( psoCreateInfo.geometryShader( pGS ).pixelShader( pWirePS ) );
    }

    for( uint32_t i = 0; i < 2; ++i ) {
        if( mPSO[i] ) {
            mPSO[i]->GetStaticVariableByName( gx::SHADER_TYPE_VERTEX, "VSConstants" )->Set( mShaderConstants );
            mPSO[i]->GetStaticVariableByName( gx::SHADER_TYPE_HULL, "HSConstants" )->Set( mShaderConstants );
            mPSO[i]->GetStaticVariableByName( gx::SHADER_TYPE_DOMAIN, "DSConstants" )->Set( mShaderConstants );
        }
    }
    if( mPSO[1] ) {
        mPSO[1]->GetStaticVariableByName( gx::SHADER_TYPE_GEOMETRY, "GSConstants" )->Set( mShaderConstants );
        mPSO[1]->GetStaticVariableByName( gx::SHADER_TYPE_PIXEL, "PSConstants" )->Set( mShaderConstants );
    }
}

void TessellationApp::loadTextures()
{
    // Load heightmap texture
    gx::TextureRef heightmap = gx::createTexture( loadImage( loadAsset( "ps_height_1k.png" ) ), gx::TextureDesc().name( "Terrain height map" ) );
    // Get shader resource view from the texture
    mHeightMapSRV = heightmap->GetDefaultView( gx::TEXTURE_VIEW_SHADER_RESOURCE );
    // Load color texture
    gx::TextureRef colormap = gx::createTexture( loadImage( loadAsset( "ps_texture_2k.png" ) ), gx::TextureDesc().name( "Terrain color map" ) );
    // Get shader resource view from the texture
    mColorMapSRV = colormap->GetDefaultView( gx::TEXTURE_VIEW_SHADER_RESOURCE );

    const auto hmDesc = heightmap->GetDesc();
    mHeightMapWidth  = hmDesc.Width;
    mHeightMapHeight = hmDesc.Height;

    // Since we are using mutable variable, we must create a shader resource binding object
    // http://diligentgraphics.com/2016/03/23/resource-binding-model-in-diligent-engine-2-0/
    for( size_t i = 0; i < _countof( mPSO ); ++i ) {
        if( mPSO[i] ) {
            mPSO[i]->CreateShaderResourceBinding( &mSRB[i], true );
            // Set texture SRV in the SRB
            mSRB[i]->GetVariableByName( gx::SHADER_TYPE_PIXEL, "g_Texture" )->Set( mColorMapSRV );
            mSRB[i]->GetVariableByName( gx::SHADER_TYPE_DOMAIN, "g_HeightMap" )->Set( mHeightMapSRV );
            mSRB[i]->GetVariableByName( gx::SHADER_TYPE_HULL, "g_HeightMap" )->Set( mHeightMapSRV );
        }
    }
}

void TessellationApp::updateUI()
{
    ImGui::SetNextWindowPos( ImVec2( 10, 10 ), ImGuiCond_FirstUseEver );
    if( ImGui::Begin( "Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
        ImGui::Checkbox( "Animate", &mAnimate );
        ImGui::Checkbox( "Adaptive tessellation", &mAdaptiveTessellation );
        if( mPSO[1] )
            ImGui::Checkbox( "Wireframe", &mWireframe );
        ImGui::SliderFloat( "Tess density", &mTessDensity, 1.f, 32.f );
        ImGui::SliderFloat( "Distance", &mDistance, 1.f, 20.f );
    }
    ImGui::End();
}

void TessellationApp::update()
{
    utils::updateWindowTitle();
    updateUI();

    // Set world view matrix
    if( mAnimate ) {
        static double lastFrameTime = getElapsedSeconds();
        double elapsedTime = getElapsedSeconds() - lastFrameTime;
        lastFrameTime = getElapsedSeconds();
        mRotationAngle += static_cast<float>( elapsedTime ) * 0.2f;
        if( mRotationAngle > glm::pi<float>() * 2.f )
            mRotationAngle -= glm::pi<float>() * 2.f;
    }

    // Apply rotation
    mat4 modelMatrix = glm::rotate( glm::pi<float>() * 0.1f, vec3( 1.0f, 0.0f, 0.0f ) ) * glm::rotate( mRotationAngle, vec3( 0.0f, 1.0f, 0.0f ) );
    // Camera is at (0, 0, -mDistance) looking along Z axis
    mat4 viewMatrix = glm::lookAt( glm::vec3( 0.f, 0.0f, mDistance ), vec3( 0.0f ), vec3( 0.0f, 1.0f, 0.0f ) );
    // Get projection matrix adjusted to the current screen orientation
    mat4 proj = glm::perspective( glm::pi<float>() / 4.0f, getWindowAspectRatio(), 0.1f, 100.f );

    mWorldViewMatrix = viewMatrix * modelMatrix;
    // Compute world-view-projection matrix
    mWorldViewProjMatrix = proj * mWorldViewMatrix;
}

void TessellationApp::draw()
{
    // Clear the back buffer
	gx::clear( ColorA( 0.350f, 0.350f, 0.350f, 1.0f ) );

    unsigned int numHorzBlocks = mHeightMapWidth / mBlockSize;
    unsigned int numVertBlocks = mHeightMapHeight / mBlockSize;
    {
        // Map the buffer and write rendering data
        gx::MapHelper<GlobalConstants> consts( getImmediateContext(), mShaderConstants, gx::MAP_WRITE, gx::MAP_FLAG_DISCARD );
        consts->fBlockSize     = static_cast<float>( mBlockSize );
        consts->numHorzBlocks  = numHorzBlocks;
        consts->numVertBlocks  = numVertBlocks;
        consts->fnumHorzBlocks = static_cast<float>( numHorzBlocks );
        consts->fnumVertBlocks = static_cast<float>( numVertBlocks );

        consts->LengthScale = 10.f;
        consts->HeightScale = consts->LengthScale / 25.f;

        consts->WorldView     = glm::transpose( mWorldViewMatrix );
        consts->WorldViewProj = glm::transpose( mWorldViewProjMatrix );

        consts->TessDensity          = mTessDensity;
        consts->AdaptiveTessellation = mAdaptiveTessellation ? 1 : 0;
        consts->ViewportSize = vec4( static_cast<float>( getWindowWidth() ), static_cast<float>( getWindowHeight() ), 1.f / static_cast<float>( getWindowWidth() ), 1.f / static_cast<float>( getWindowHeight() ) );

        consts->LineWidth = 3.0f;
    }


    // Set the pipeline state
    gx::setPipelineState( mPSO[mWireframe ? 1 : 0] );
    // Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode
    // makes sure that resources are transitioned to required states.
    gx::commitShaderResources( mSRB[mWireframe ? 1 : 0], gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );

    gx::draw( gx::DrawAttribs().numVertices( numHorzBlocks * numVertBlocks ).flags( gx::DRAW_FLAG_VERIFY_ALL ) );
}

void TessellationApp::prepareEngine( gx::RENDER_DEVICE_TYPE deviceType, gx::EngineCreateInfo* engineCreateInfo, gx::SwapChainDesc* swapChainDesc )
{
    engineCreateInfo->Features.Tessellation      = gx::DEVICE_FEATURE_STATE_ENABLED;
    engineCreateInfo->Features.SeparablePrograms = gx::DEVICE_FEATURE_STATE_ENABLED;
    engineCreateInfo->Features.GeometryShaders   = gx::DEVICE_FEATURE_STATE_OPTIONAL;
}

CINDER_APP( TessellationApp, RendererGx( RendererGx::Options().prepareEngineFn( TessellationApp::prepareEngine ) ) )
