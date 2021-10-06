#include "cinder/app/App.h"
#include "cinder/app/RendererGx.h"
#include "cinder/graphics/Mesh.h"
#include "cinder/graphics/Batch.h"
#include "cinder/graphics/PipelineState.h"
#include "cinder/graphics/Sampler.h"
#include "cinder/CinderDiligentImGui.h"

#include "TexturedCube.h"
#include "Utils.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class ShadowMapApp : public App {
  public:
	void setup() override;
	void update() override;
	void draw() override;

    static void prepareEngine( gx::RENDER_DEVICE_TYPE, gx::EngineCreateInfo*, gx::SwapChainDesc* );

    void createCubePSO();
    void createPlanePSO();
    void createShadowMapVisPSO();
    void createVertexBuffer();
    void updateUI();
    void createShadowMap();
    void renderShadowMap();
    void renderCube( const mat4& CameraViewProj, bool IsShadowPass );
    void renderPlane();
    void renderShadowMapVis();

    gx::PipelineStateRef         mCubePSO;
    gx::PipelineStateRef         mCubeShadowPSO;
    gx::PipelineStateRef         mPlanePSO;
    gx::PipelineStateRef         mShadowMapVisPSO;
    gx::BufferRef                mCubeVertexBuffer;
    gx::BufferRef                mCubeIndexBuffer;
    gx::BufferRef                mVSConstants;
    gx::TextureViewRef           mTextureSRV;
    gx::ShaderResourceBindingRef mCubeSRB;
    gx::ShaderResourceBindingRef mCubeShadowSRB;
    gx::ShaderResourceBindingRef mPlaneSRB;
    gx::ShaderResourceBindingRef mShadowMapVisSRB;
    gx::TextureViewRef           mShadowMapDSV;
    gx::TextureViewRef           mShadowMapSRV;

    mat4                mCubeWorldMatrix;
    mat4                mCameraViewProjMatrix;
    mat4                mWorldToShadowMapUVDepthMatr;
    vec3                mLightDirection  = normalize( vec3( -0.49f, -0.60f, 0.64f ) );
    uint32_t            mShadowMapSize   = 512;
    gx::TEXTURE_FORMAT  mShadowMapFormat = gx::TEX_FORMAT_D16_UNORM;
};

void ShadowMapApp::prepareEngine( gx::RENDER_DEVICE_TYPE deviceType, gx::EngineCreateInfo* engineCreateInfo, gx::SwapChainDesc* swapChainDesc )
{
    engineCreateInfo->Features.DepthClamp = gx::DEVICE_FEATURE_STATE_OPTIONAL;
}

void ShadowMapApp::setup()
{
    disableFrameRate();
    ImGui::DiligentInitialize();

    // Create dynamic uniform buffer that will store our transformation matrices
    // Dynamic buffers can be frequently updated by the CPU
    mVSConstants = gx::createBuffer( gx::BufferDesc()
        .name( "VS constants CB" )
        .size( sizeof( mat4 ) * 2 + sizeof( vec4 ) )
        .usage( gx::USAGE_DYNAMIC )
        .bindFlags( gx::BIND_UNIFORM_BUFFER )
        .cpuAccessFlags( gx::CPU_ACCESS_WRITE )
    );
    
    std::vector<gx::StateTransitionDesc> barriers;
    barriers.emplace_back( mVSConstants, gx::RESOURCE_STATE_UNKNOWN, gx::RESOURCE_STATE_CONSTANT_BUFFER, gx::STATE_TRANSITION_FLAG_UPDATE_STATE );

    createCubePSO();
    createPlanePSO();
    createShadowMapVisPSO();

    // Load cube
    // In this tutorial we need vertices with normals
    createVertexBuffer();
    // Load index buffer
    mCubeIndexBuffer = TexturedCube::createIndexBuffer( getRenderDevice() );
    // Explicitly transition vertex and index buffers to required states
    barriers.emplace_back( mCubeVertexBuffer, gx::RESOURCE_STATE_UNKNOWN, gx::RESOURCE_STATE_VERTEX_BUFFER, gx::STATE_TRANSITION_FLAG_UPDATE_STATE );
    barriers.emplace_back( mCubeIndexBuffer, gx::RESOURCE_STATE_UNKNOWN, gx::RESOURCE_STATE_INDEX_BUFFER, gx::STATE_TRANSITION_FLAG_UPDATE_STATE );
    // Load texture
    auto CubeTexture = TexturedCube::loadTexture( getRenderDevice(), getAssetPath( "DGLogo.png" ) );
    mCubeSRB->GetVariableByName( gx::SHADER_TYPE_PIXEL, "g_Texture" )->Set( CubeTexture->GetDefaultView( gx::TEXTURE_VIEW_SHADER_RESOURCE ) );
    // Transition the texture to shader resource state
    barriers.emplace_back( CubeTexture, gx::RESOURCE_STATE_UNKNOWN, gx::RESOURCE_STATE_SHADER_RESOURCE, gx::STATE_TRANSITION_FLAG_UPDATE_STATE );

    createShadowMap();

    gx::transitionResourceStates( static_cast<uint32_t>( barriers.size() ), barriers.data() );
}

void ShadowMapApp::createCubePSO()
{
    // Define vertex shader input layout
    vector<gx::LayoutElement> layoutElems = {
        // Attribute 0 - vertex position
        { 0, 0, 3, gx::VT_FLOAT32, false },
        // Attribute 1 - texture coordinates
        { 1, 0, 2, gx::VT_FLOAT32, false },
        // Attribute 2 - normal
        { 2, 0, 3, gx::VT_FLOAT32, false },
    };
    mCubePSO = TexturedCube::createPipelineState( getRenderDevice(), getSwapChainColorFormat(), getSwapChainDepthFormat(), getAssetPath( "cube.vsh" ), getAssetPath( "cube.psh" ), layoutElems );

    // Since we did not explcitly specify the type for 'Constants' variable, default
    // type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables never
    // change and are bound directly through the pipeline state object.
    mCubePSO->GetStaticVariableByName( gx::SHADER_TYPE_VERTEX, "Constants" )->Set( mVSConstants );

    // Since we are using mutable variable, we must create a shader resource binding object
    // http://diligentgraphics.com/2016/03/23/resource-binding-model-in-diligent-engine-2-0/
    mCubePSO->CreateShaderResourceBinding( &mCubeSRB, true );

    // Create shadow pass PSO
    gx::GraphicsPipelineCreateInfo psoCreateInfo = gx::GraphicsPipelineCreateInfo()
        .name( "Cube shadow PSO" )
        // Shadow pass doesn't use any render target outputs
        .numRenderTargets( 0 )
        .rtvFormat( 0, gx::TEX_FORMAT_UNKNOWN )
        // The DSV format is the shadow map format
        .dsvFormat( mShadowMapFormat )
        .primitiveTopology( gx::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST )
        // Cull back faces
        .cullMode( gx::CULL_MODE_BACK )
        // Enable depth testing
        .depthEnable( true )
         // Create a vertex shader
        .vertexShader( gx::ShaderCreateInfo()
            .name( "Cube Shadow VS" )
            // Tell the system that the shader source code is in HLSL. For OpenGL, the engine will convert this into GLSL under the hood.
            .sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
            // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
            .useCombinedTextureSamplers( true )
            .filePath( getAssetPath( "cube_shadow.vsh" ) )
        )
        // We don't use pixel shader as we are only interested in populating the depth buffer
        //.pixelShader( nullptr )
        .inputLayout( layoutElems );

    if( getRenderDevice()->GetDeviceCaps().Features.DepthClamp ) {
        // Disable depth clipping to render objects that are closer than near
        // clipping plane. This is not required for this tutorial, but real applications
        // will most likely want to do this.
        psoCreateInfo.depthClipEnable( false );
    }

    mCubeShadowPSO = gx::createGraphicsPipelineState( psoCreateInfo );
    mCubeShadowPSO->GetStaticVariableByName( gx::SHADER_TYPE_VERTEX, "Constants" )->Set( mVSConstants );
    mCubeShadowPSO->CreateShaderResourceBinding( &mCubeShadowSRB, true );
}

void ShadowMapApp::createPlanePSO()
{
    // Define immutable comparison sampler for g_ShadowMap. Immutable samplers should be used whenever possible
    gx::SamplerDesc comparsionSampler = gx::SamplerDesc()
        .comparisonFunc( gx::COMPARISON_FUNC_LESS )
        .minFilter( gx::FILTER_TYPE_COMPARISON_LINEAR )
        .magFilter( gx::FILTER_TYPE_COMPARISON_LINEAR )
        .mipFilter( gx::FILTER_TYPE_COMPARISON_LINEAR );

    gx::ShaderCreateInfo shaderCreateInfo = gx::ShaderCreateInfo()
         // Tell the system that the shader source code is in HLSL. For OpenGL, the engine will convert this into GLSL under the hood.
        .sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
        // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
        .useCombinedTextureSamplers( true );

    // Create shadow pass PSO
    mPlanePSO = gx::createGraphicsPipelineState( gx::GraphicsPipelineCreateInfo()
        // Pipeline state name is used by the engine to report issues.
        // It is always a good idea to give objects descriptive names.
        .name( "Plane PSO" )
        // Primitive topology defines what kind of primitives will be rendered by this pipeline state
        .primitiveTopology( gx::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP )
        // No cull
        .cullMode( gx::CULL_MODE_NONE )
        // Enable depth testing
        .depthEnable( true )
         // Create a plane vertex shader
        .vertexShader( shaderCreateInfo.name( "Plane VS" ).filePath( getAssetPath( "plane.vsh" ) ) )
        .pixelShader( shaderCreateInfo.name( "Plane PS" ).filePath( getAssetPath( "plane.psh" ) ) )
        .variables( { { gx::SHADER_TYPE_PIXEL, "g_ShadowMap", gx::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE } } )
        .immutableSamplers( { { gx::SHADER_TYPE_PIXEL, "g_ShadowMap", comparsionSampler } } )
    );

    // Since we did not explcitly specify the type for 'Constants' variable, default
    // type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables never
    // change and are bound directly through the pipeline state object.
    mPlanePSO->GetStaticVariableByName( gx::SHADER_TYPE_VERTEX, "Constants" )->Set( mVSConstants );
}

void ShadowMapApp::createShadowMapVisPSO()
{
    gx::SamplerDesc samLinearClampDesc {
        gx::FILTER_TYPE_LINEAR, gx::FILTER_TYPE_LINEAR, gx::FILTER_TYPE_LINEAR,
        gx::TEXTURE_ADDRESS_CLAMP, gx::TEXTURE_ADDRESS_CLAMP, gx::TEXTURE_ADDRESS_CLAMP
    };

    gx::ShaderCreateInfo shaderCreateInfo = gx::ShaderCreateInfo()
         // Tell the system that the shader source code is in HLSL. For OpenGL, the engine will convert this into GLSL under the hood.
        .sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
        // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
        .useCombinedTextureSamplers( true );

    mShadowMapVisPSO = gx::createGraphicsPipelineState( gx::GraphicsPipelineCreateInfo()
        // Pipeline state name is used by the engine to report issues.
        // It is always a good idea to give objects descriptive names.
        .name( "Shadow Map Vis PSO" )
        // Primitive topology defines what kind of primitives will be rendered by this pipeline state
        .primitiveTopology( gx::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP )
        // No cull
        .cullMode( gx::CULL_MODE_NONE )
        // Disable depth testing
        .depthEnable( false )
        // Create shadow map visualization vertex shader
        .vertexShader( shaderCreateInfo.name( "Shadow Map Vis VS" ).filePath( getAssetPath( "shadow_map_vis.vsh" ) ) )
        // Create shadow map visualization pixel shader
        .pixelShader( shaderCreateInfo.name( "Shadow Map Vis PS" ).filePath( getAssetPath( "shadow_map_vis.psh" ) ) )
        // Define variable type that will be used by default
        .defaultVariableType( gx::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE )
        .immutableSamplers( { { gx::SHADER_TYPE_PIXEL, "g_ShadowMap", samLinearClampDesc } } )
    );
}

void ShadowMapApp::createVertexBuffer()
{
    // Layout of this structure matches the one we defined in pipeline state
    struct Vertex {
        vec3 pos;
        vec2 uv;
        vec3 normal;
    };

    // Cube vertices

    //      (-1,+1,+1)________________(+1,+1,+1)
    //               /|              /|
    //              / |             / |
    //             /  |            /  |
    //            /   |           /   |
    //(-1,-1,+1) /____|__________/(+1,-1,+1)
    //           |    |__________|____|
    //           |   /(-1,+1,-1) |    /(+1,+1,-1)
    //           |  /            |   /
    //           | /             |  /
    //           |/              | /
    //           /_______________|/
    //        (-1,-1,-1)       (+1,-1,-1)
    //

    // clang-format off
    Vertex cubeVerts[] = {
        {vec3( -1,-1,-1 ), vec2( 0,1 ), vec3( 0, 0, -1 )},
        {vec3( -1,+1,-1 ), vec2( 0,0 ), vec3( 0, 0, -1 )},
        {vec3( +1,+1,-1 ), vec2( 1,0 ), vec3( 0, 0, -1 )},
        {vec3( +1,-1,-1 ), vec2( 1,1 ), vec3( 0, 0, -1 )},

        {vec3( -1,-1,-1 ), vec2( 0,1 ), vec3( 0, -1, 0 )},
        {vec3( -1,-1,+1 ), vec2( 0,0 ), vec3( 0, -1, 0 )},
        {vec3( +1,-1,+1 ), vec2( 1,0 ), vec3( 0, -1, 0 )},
        {vec3( +1,-1,-1 ), vec2( 1,1 ), vec3( 0, -1, 0 )},

        {vec3( +1,-1,-1 ), vec2( 0,1 ), vec3( +1, 0, 0 )},
        {vec3( +1,-1,+1 ), vec2( 1,1 ), vec3( +1, 0, 0 )},
        {vec3( +1,+1,+1 ), vec2( 1,0 ), vec3( +1, 0, 0 )},
        {vec3( +1,+1,-1 ), vec2( 0,0 ), vec3( +1, 0, 0 )},

        {vec3( +1,+1,-1 ), vec2( 0,1 ), vec3( 0, +1, 0 )},
        {vec3( +1,+1,+1 ), vec2( 0,0 ), vec3( 0, +1, 0 )},
        {vec3( -1,+1,+1 ), vec2( 1,0 ), vec3( 0, +1, 0 )},
        {vec3( -1,+1,-1 ), vec2( 1,1 ), vec3( 0, +1, 0 )},

        {vec3( -1,+1,-1 ), vec2( 1,0 ), vec3( -1, 0, 0 )},
        {vec3( -1,+1,+1 ), vec2( 0,0 ), vec3( -1, 0, 0 )},
        {vec3( -1,-1,+1 ), vec2( 0,1 ), vec3( -1, 0, 0 )},
        {vec3( -1,-1,-1 ), vec2( 1,1 ), vec3( -1, 0, 0 )},

        {vec3( -1,-1,+1 ), vec2( 1,1 ), vec3( 0, 0, +1 )},
        {vec3( +1,-1,+1 ), vec2( 0,1 ), vec3( 0, 0, +1 )},
        {vec3( +1,+1,+1 ), vec2( 0,0 ), vec3( 0, 0, +1 )},
        {vec3( -1,+1,+1 ), vec2( 1,0 ), vec3( 0, 0, +1 )}
    };

    mCubeVertexBuffer = gx::createBuffer( gx::BufferDesc()
        .name( "Cube vertex buffer" )
        .usage( gx::USAGE_IMMUTABLE )
        .bindFlags( gx::BIND_VERTEX_BUFFER )
        .size( sizeof( cubeVerts ) ),
        &cubeVerts, sizeof( cubeVerts )
    );
}

void ShadowMapApp::createShadowMap()
{
    gx::TextureRef shadowMap = gx::createTexture( gx::TextureDesc()
        .name( "Shadow map" )
        .type( gx::RESOURCE_DIM_TEX_2D )
        .width( mShadowMapSize )
        .height( mShadowMapSize )
        .format( mShadowMapFormat )
        .bindFlags( gx::BIND_SHADER_RESOURCE | gx::BIND_DEPTH_STENCIL )
    );
    mShadowMapSRV = shadowMap->GetDefaultView( gx::TEXTURE_VIEW_SHADER_RESOURCE );
    mShadowMapDSV = shadowMap->GetDefaultView( gx::TEXTURE_VIEW_DEPTH_STENCIL );

    // Create SRBs that use shadow map as mutable variable
    mPlaneSRB.Release();
    mPlanePSO->CreateShaderResourceBinding( &mPlaneSRB, true );
    mPlaneSRB->GetVariableByName( gx::SHADER_TYPE_PIXEL, "g_ShadowMap" )->Set( mShadowMapSRV );

    mShadowMapVisSRB.Release();
    mShadowMapVisPSO->CreateShaderResourceBinding( &mShadowMapVisSRB, true );
    mShadowMapVisSRB->GetVariableByName( gx::SHADER_TYPE_PIXEL, "g_ShadowMap" )->Set( mShadowMapSRV );
}

void ShadowMapApp::updateUI()
{
    ImGui::SetNextWindowPos( ImVec2( 10, 10 ), ImGuiCond_FirstUseEver );
    if( ImGui::Begin( "Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
        constexpr int MinShadowMapSize = 256;
        int           ShadowMapComboId = 0;
        while( ( MinShadowMapSize << ShadowMapComboId ) != static_cast<int>( mShadowMapSize ) )
            ++ShadowMapComboId;
        if( ImGui::Combo( "Shadow map size", &ShadowMapComboId,
            "256\0"
            "512\0"
            "1024\0\0" ) ) {
            mShadowMapSize = MinShadowMapSize << ShadowMapComboId;
            createShadowMap();
        }
        if( ImGui::DragFloat3( "LightDirection", &mLightDirection[0], 0.01f, -1.0f, 1.0f ) ) {
            float length = glm::length( mLightDirection );
            if( length > 1.0f ) {
                mLightDirection /= length;
            }
        }
        //ImGui::Gizmo3D( "##LightDirection", mLightDirection, ImGui::GetTextLineHeight() * 10 );
    }
    ImGui::End();
}

void ShadowMapApp::update()
{
    utils::updateWindowTitle();
    updateUI();

    // Animate the cube
    mCubeWorldMatrix = glm::rotate( (float) getElapsedSeconds(), vec3( 0.0f, 1.0f, 0.0f ) );
    mat4 cameraView = glm::rotate( -glm::pi<float>() * 0.2f, vec3( 1.0f, 0.0f, 0.0f ) ) * glm::rotate( glm::pi<float>(), vec3( 0.0f, 1.0f, 0.0f ) ) * glm::translate( vec3( 0.f, -5.0f, -10.0f ) );
    // Get projection matrix adjusted to the current screen orientation
    mat4 proj = glm::perspective( glm::pi<float>() / 4.0f, getWindowAspectRatio(), 0.1f, 100.f );
    // Compute camera view-projection matrix
    mCameraViewProjMatrix = proj * cameraView;
}

void ShadowMapApp::renderShadowMap()
{
    vec3 f3LightSpaceX, f3LightSpaceY, f3LightSpaceZ;
    f3LightSpaceZ = glm::normalize( mLightDirection );

    auto min_cmp = std::min( std::min( std::abs( mLightDirection.x ), std::abs( mLightDirection.y ) ), std::abs( mLightDirection.z ) );
    if( min_cmp == std::abs( mLightDirection.x ) )
        f3LightSpaceX = vec3( 1, 0, 0 );
    else if( min_cmp == std::abs( mLightDirection.y ) )
        f3LightSpaceX = vec3( 0, 1, 0 );
    else
        f3LightSpaceX = vec3( 0, 0, 1 );

    f3LightSpaceY = cross( f3LightSpaceZ, f3LightSpaceX );
    f3LightSpaceX = cross( f3LightSpaceY, f3LightSpaceZ );
    f3LightSpaceX = normalize( f3LightSpaceX );
    f3LightSpaceY = normalize( f3LightSpaceY );

    mat4 WorldToLightViewSpaceMatr = mat4(
        f3LightSpaceX.x, f3LightSpaceY.x, f3LightSpaceZ.x, 0,
        f3LightSpaceX.y, f3LightSpaceY.y, f3LightSpaceZ.y, 0,
        f3LightSpaceX.z, f3LightSpaceY.z, f3LightSpaceZ.z, 0,
        0, 0, 0, 1 );

    // For this tutorial we know that the scene center is at (0,0,0).
    // Real applications will want to compute tight bounds
    vec3 f3SceneCenter = vec3( 0, 0, 0 );
    float  SceneRadius = std::sqrt( 3.f );
    vec3 f3MinXYZ      = f3SceneCenter - vec3( SceneRadius, SceneRadius, SceneRadius );
    vec3 f3MaxXYZ      = f3SceneCenter + vec3( SceneRadius, SceneRadius, SceneRadius * 5 );
    vec3 f3SceneExtent = f3MaxXYZ - f3MinXYZ;

    const auto& DevCaps = getRenderDevice()->GetDeviceCaps();
    const bool  IsGL    = DevCaps.IsGLDevice();
    vec4      f4LightSpaceScale;
    f4LightSpaceScale.x = 2.f / f3SceneExtent.x;
    f4LightSpaceScale.y = 2.f / f3SceneExtent.y;
    f4LightSpaceScale.z = ( IsGL ? 2.f : 1.f ) / f3SceneExtent.z;
    // Apply bias to shift the extent to [-1,1]x[-1,1]x[0,1] for DX or to [-1,1]x[-1,1]x[-1,1] for GL
    // Find bias such that f3MinXYZ -> (-1,-1,0) for DX or (-1,-1,-1) for GL
    vec4 f4LightSpaceScaledBias;
    f4LightSpaceScaledBias.x = -f3MinXYZ.x * f4LightSpaceScale.x - 1.f;
    f4LightSpaceScaledBias.y = -f3MinXYZ.y * f4LightSpaceScale.y - 1.f;
    f4LightSpaceScaledBias.z = -f3MinXYZ.z * f4LightSpaceScale.z + ( IsGL ? -1.f : 0.f );

    mat4 ScaleMatrix      = glm::scale( vec3( f4LightSpaceScale.x, f4LightSpaceScale.y, f4LightSpaceScale.z ) );
    mat4 ScaledBiasMatrix = glm::translate( vec3( f4LightSpaceScaledBias.x, f4LightSpaceScaledBias.y, f4LightSpaceScaledBias.z ) );

    // Note: bias is applied after scaling!
    mat4 ShadowProjMatr = ScaledBiasMatrix * ScaleMatrix;

    // Adjust the world to light space transformation matrix
    mat4 WorldToLightProjSpaceMatr = ShadowProjMatr * WorldToLightViewSpaceMatr;

    const auto& NDCAttribs    = DevCaps.GetNDCAttribs();
    mat4    ProjToUVScale = glm::scale( vec3( 0.5f, NDCAttribs.YtoVScale, NDCAttribs.ZtoDepthScale ) );
    mat4    ProjToUVBias  = glm::translate( vec3( 0.5f, 0.5f, NDCAttribs.GetZtoDepthBias() ) );

    mWorldToShadowMapUVDepthMatr = ProjToUVBias * ProjToUVScale * WorldToLightProjSpaceMatr;

    renderCube( WorldToLightProjSpaceMatr, true );
}

void ShadowMapApp::renderCube( const mat4& CameraViewProj, bool IsShadowPass )
{
    // Update constant buffer
    {
        struct Constants {
            mat4 WorldViewProj;
            mat4 NormalTranform;
            vec4 LightDirection;
        };

        // Map the buffer and write current world-view-projection matrix
        gx::MapHelper<Constants> CBConstants( getImmediateContext(), mVSConstants, gx::MAP_WRITE, gx::MAP_FLAG_DISCARD );
        CBConstants->WorldViewProj = glm::transpose( CameraViewProj * mCubeWorldMatrix );
        mat4 NormalMatrix          = glm::inverse( mat4( mat3( mCubeWorldMatrix ) ) );
        // We need to do inverse-transpose, but we also need to transpose the matrix
        // before writing it to the buffer
        CBConstants->NormalTranform = NormalMatrix;
        CBConstants->LightDirection = vec4( mLightDirection, 0.0f );
    }

    // Bind vertex buffer
    // Note that since resouces have been explicitly transitioned to required states, we use RESOURCE_STATE_TRANSITION_MODE_VERIFY flag
    gx::setVertexBuffer( mCubeVertexBuffer, gx::RESOURCE_STATE_TRANSITION_MODE_VERIFY, gx::SET_VERTEX_BUFFERS_FLAG_RESET );
    gx::setIndexBuffer( mCubeIndexBuffer, 0, gx::RESOURCE_STATE_TRANSITION_MODE_VERIFY );

    // Set pipeline state and commit resources
    if( IsShadowPass ) {
        gx::setPipelineState( mCubeShadowPSO );
        gx::commitShaderResources( mCubeShadowSRB, gx::RESOURCE_STATE_TRANSITION_MODE_VERIFY );
    }
    else {
        gx::setPipelineState( mCubePSO );
        gx::commitShaderResources( mCubeSRB, gx::RESOURCE_STATE_TRANSITION_MODE_VERIFY );
    }

    gx::drawIndexed( gx::DrawIndexedAttribs().indexType( gx::VT_UINT32 ).numIndices( 36 ).flags( gx::DRAW_FLAG_VERIFY_ALL ) );
}

void ShadowMapApp::renderPlane()
{
    {
        struct Constants {
            mat4 CameraViewProj;
            mat4 WorldToShadowMapUVDepth;
            vec4 LightDirection;
        };
        gx::MapHelper<Constants> CBConstants( getImmediateContext(), mVSConstants, gx::MAP_WRITE, gx::MAP_FLAG_DISCARD );
        CBConstants->CameraViewProj          = glm::transpose( mCameraViewProjMatrix );
        CBConstants->WorldToShadowMapUVDepth = glm::transpose( mWorldToShadowMapUVDepthMatr );
        CBConstants->LightDirection          = vec4( mLightDirection, 0.0f );
    }

    gx::setPipelineState( mPlanePSO );
    // Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode
    // makes sure that resources are transitioned to required states.
    // Note that Vulkan requires shadow map to be transitioned to DEPTH_READ state, not SHADER_RESOURCE
    gx::commitShaderResources( mPlaneSRB, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );

    gx::draw( gx::DrawAttribs().numVertices( 4 ).flags( gx::DRAW_FLAG_VERIFY_ALL ) );
}

void ShadowMapApp::renderShadowMapVis()
{
    gx::setPipelineState( mShadowMapVisPSO );
    gx::commitShaderResources( mShadowMapVisSRB, gx::RESOURCE_STATE_TRANSITION_MODE_VERIFY );

    gx::draw( gx::DrawAttribs().numVertices( 4 ).flags( gx::DRAW_FLAG_VERIFY_ALL ) );
}

void ShadowMapApp::draw()
{
    // Render shadow map
    gx::setRenderTargets( 0, nullptr, mShadowMapDSV, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
    gx::clearDepthStencil( mShadowMapDSV, gx::CLEAR_DEPTH_FLAG, 1.f, 0, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
    renderShadowMap();

    // Bind main back buffer
    auto* pRTV = getSwapChain()->GetCurrentBackBufferRTV();
    auto* pDSV = getSwapChain()->GetDepthBufferDSV();
    gx::setRenderTargets( 1, &pRTV, pDSV, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );

    gx::clear( ColorA( 0.350f, 0.350f, 0.350f, 1.0f ) );

    renderCube( mCameraViewProjMatrix, false );
    renderPlane();
    renderShadowMapVis();
}

CINDER_APP( ShadowMapApp, RendererGx( RendererGx::Options().prepareEngineFn( ShadowMapApp::prepareEngine ) ) )