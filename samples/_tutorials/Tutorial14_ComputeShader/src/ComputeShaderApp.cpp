#include "cinder/app/App.h"
#include "cinder/app/RendererGx.h"

#include "cinder/graphics/Buffer.h"
#include "cinder/graphics/PipelineState.h"
#include "cinder/graphics/Texture.h"
#include "cinder/CinderDiligentImGui.h"

#include <random>

#include "Utils.h"

using namespace ci;
using namespace ci::app;
using namespace std;

struct ParticleAttribs
{
    vec2 f2Pos;
    vec2 f2NewPos;

    vec2 f2Speed;
    vec2 f2NewSpeed;

    float fSize;
    float fTemperature;
    int   iNumCollisions;
    float fPadding0;
};

class ComputeShaderApp : public App {
  public:
	void setup() override;
	void update() override;
	void draw() override;

    void createRenderParticlePSO();
    void createUpdateParticlePSO();
    void createParticleBuffers();
    void createConsantBuffer();
    void updateUI();

    static void prepareEngine( gx::RENDER_DEVICE_TYPE, gx::EngineCreateInfo*, gx::SwapChainDesc* );

    int mNumParticles    = 2000;
    int mThreadGroupSize = 256;

    gx::PipelineStateRef         mRenderParticlePSO;
    gx::ShaderResourceBindingRef mRenderParticleSRB;
    gx::PipelineStateRef         mResetParticleListsPSO;
    gx::ShaderResourceBindingRef mResetParticleListsSRB;
    gx::PipelineStateRef         mMoveParticlesPSO;
    gx::ShaderResourceBindingRef mMoveParticlesSRB;
    gx::PipelineStateRef         mCollideParticlesPSO;
    gx::ShaderResourceBindingRef mCollideParticlesSRB;
    gx::PipelineStateRef         mUpdateParticleSpeedPSO;
    gx::BufferRef                mConstants;
    gx::BufferRef                mParticleAttribsBuffer;
    gx::BufferRef                mParticleListsBuffer;
    gx::BufferRef                mParticleListHeadsBuffer;
    gx::ResourceMappingRef       mResMapping;

    float mTimeDelta       = 0;
    float mSimulationSpeed = 1;
};

void ComputeShaderApp::prepareEngine( gx::RENDER_DEVICE_TYPE deviceType, gx::EngineCreateInfo* engineCreateInfo, gx::SwapChainDesc* swapChainDesc )
{
    engineCreateInfo->Features.ComputeShaders = gx::DEVICE_FEATURE_STATE_ENABLED;
}

void ComputeShaderApp::setup()
{
    disableFrameRate();
    ImGui::DiligentInitialize();

    createConsantBuffer();
    createRenderParticlePSO();
    createUpdateParticlePSO();
    createParticleBuffers();
}

void ComputeShaderApp::createRenderParticlePSO()
{
    gx::GraphicsPipelineDesc psoCreateInfo = gx::GraphicsPipelineDesc()
        // Pipeline state name is used by the engine to report issues.
        .name( "Render particles PSO" )
        // Primitive topology defines what kind of primitives will be rendered by this pipeline state
        .primitiveTopology( gx::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP )
        // Disable back face culling
        .cullMode( gx::CULL_MODE_NONE )
        // Disable depth testing
        .depthEnable( false )
        // Set the Blending mode
        .renderTargetBlendDesc( 0, gx::RenderTargetBlendDesc().blendEnable( true ).srcBlend( gx::BLEND_FACTOR_SRC_ALPHA ).destBlend( gx::BLEND_FACTOR_INV_SRC_ALPHA ) )
        // Create particle vertex shader
        .vertexShader( gx::ShaderCreateInfo()
            .name( "Particle VS" )
            // Tell the system that the shader source code is in HLSL. For OpenGL, the engine will convert this into GLSL under the hood.
            .sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
            .filePath( getAssetPath( "particle.vsh" ) )
        )
        .pixelShader( gx::ShaderCreateInfo()
            .name( "Particle PS" )
            // Tell the system that the shader source code is in HLSL. For OpenGL, the engine will convert this into GLSL under the hood.
            .sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
            // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
            .useCombinedTextureSamplers( true )
            .filePath( getAssetPath( "particle.psh" ) )
        )
        // Shader variables should typically be mutable, which means they are expected to change on a per-instance basis
        .variables( { { gx::SHADER_TYPE_VERTEX, "g_Particles", gx::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE } } );

    mRenderParticlePSO = gx::createGraphicsPipelineState( psoCreateInfo );
    mRenderParticlePSO->GetStaticVariableByName( gx::SHADER_TYPE_VERTEX, "Constants" )->Set( mConstants );
}

void ComputeShaderApp::createUpdateParticlePSO()
{
    gx::ComputePipelineDesc computePipelineDesc = gx::ComputePipelineDesc()
        .variables( { { gx::SHADER_TYPE_COMPUTE, "Constants", gx::SHADER_RESOURCE_VARIABLE_TYPE_STATIC } } )
        .defaultVariableType( gx::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE );

    gx::ShaderCreateInfo shaderCreateInfo = gx::ShaderCreateInfo()
        .macro( "THREAD_GROUP_SIZE", mThreadGroupSize )
        .useCombinedTextureSamplers( true )
        .sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL );

    mResetParticleListsPSO = gx::createComputePipelineState( computePipelineDesc.shader( shaderCreateInfo.name( "Reset particle lists CS" ).filePath( getAssetPath( "reset_particle_lists.csh" ) ) ) );
    mResetParticleListsPSO->GetStaticVariableByName( gx::SHADER_TYPE_COMPUTE, "Constants" )->Set( mConstants );

    mMoveParticlesPSO = gx::createComputePipelineState( computePipelineDesc.shader( shaderCreateInfo.name( "Move particle CS" ).filePath( getAssetPath( "move_particles.csh" ) ) ) );
    mMoveParticlesPSO->GetStaticVariableByName( gx::SHADER_TYPE_COMPUTE, "Constants" )->Set( mConstants );

    mCollideParticlesPSO = gx::createComputePipelineState( computePipelineDesc.shader( shaderCreateInfo.name( "Collide particle CS" ).filePath( getAssetPath( "collide_particles.csh" ) ) ) );
    mCollideParticlesPSO->GetStaticVariableByName( gx::SHADER_TYPE_COMPUTE, "Constants" )->Set( mConstants );

    mUpdateParticleSpeedPSO = gx::createComputePipelineState( computePipelineDesc.shader( shaderCreateInfo.name( "Update particle speed CS" ).filePath( getAssetPath( "collide_particles.csh" ) ).macro( "UDPATE_SPEED", 1 ) ) );
    mUpdateParticleSpeedPSO->GetStaticVariableByName( gx::SHADER_TYPE_COMPUTE, "Constants" )->Set( mConstants );
}

void ComputeShaderApp::createParticleBuffers()
{
    mParticleAttribsBuffer.Release();
    mParticleListHeadsBuffer.Release();
    mParticleListsBuffer.Release();

    std::vector<ParticleAttribs> particleData( mNumParticles );

    std::mt19937 gen; // Standard mersenne_twister_engine. Use default seed
                      // to generate consistent distribution.

    std::uniform_real_distribution<float> pos_distr( -1.f, +1.f );
    std::uniform_real_distribution<float> size_distr( 0.5f, 1.f );

    constexpr float fMaxParticleSize = 0.05f;
    float           fSize            = 0.7f / std::sqrt( static_cast<float>( mNumParticles ) );
    fSize                            = std::min( fMaxParticleSize, fSize );
    for( auto& particle : particleData ) {
        particle.f2NewPos.x   = pos_distr( gen );
        particle.f2NewPos.y   = pos_distr( gen );
        particle.f2NewSpeed.x = pos_distr( gen ) * fSize * 5.f;
        particle.f2NewSpeed.y = pos_distr( gen ) * fSize * 5.f;
        particle.fSize        = fSize * size_distr( gen );
    }

    mParticleAttribsBuffer = gx::createBuffer( gx::BufferDesc()
            .name( "Particle attribs buffer" )
            .usage( gx::USAGE_DEFAULT )
            .bindFlags( gx::BIND_SHADER_RESOURCE | gx::BIND_UNORDERED_ACCESS )
            .mode( gx::BUFFER_MODE_STRUCTURED )
            .elementByteStride( sizeof( ParticleAttribs ) )
            .sizeInBytes( sizeof( ParticleAttribs ) * mNumParticles ), 
        particleData.data(), sizeof( ParticleAttribs ) * static_cast<uint32_t>( particleData.size() ) );

    gx::BufferView* pParticleAttribsBufferSRV = mParticleAttribsBuffer->GetDefaultView( gx::BUFFER_VIEW_SHADER_RESOURCE );
    gx::BufferView* pParticleAttribsBufferUAV = mParticleAttribsBuffer->GetDefaultView( gx::BUFFER_VIEW_UNORDERED_ACCESS );

    mParticleListHeadsBuffer = gx::createBuffer( gx::BufferDesc()
        .name( "Particle heads buffer" )
        .usage( gx::USAGE_DEFAULT )
        .bindFlags( gx::BIND_SHADER_RESOURCE | gx::BIND_UNORDERED_ACCESS )
        .mode( gx::BUFFER_MODE_FORMATTED )
        .elementByteStride( sizeof( int ) )
        .sizeInBytes( sizeof( int ) * mNumParticles ), nullptr );

    mParticleListsBuffer = gx::createBuffer( gx::BufferDesc()
        .name( "Particle lists buffer" )
        .usage( gx::USAGE_DEFAULT )
        .bindFlags( gx::BIND_SHADER_RESOURCE | gx::BIND_UNORDERED_ACCESS )
        .mode( gx::BUFFER_MODE_FORMATTED )
        .elementByteStride( sizeof( int ) )
        .sizeInBytes( sizeof( int ) * mNumParticles ), nullptr );

    gx::BufferViewRef pParticleListHeadsBufferUAV;
    gx::BufferViewRef pParticleListsBufferUAV;
    gx::BufferViewRef pParticleListHeadsBufferSRV;
    gx::BufferViewRef pParticleListsBufferSRV;
    {
        gx::BufferViewDesc viewDesc = gx::BufferViewDesc()
            .viewType( gx::BUFFER_VIEW_UNORDERED_ACCESS )
            .valueType( gx::VT_INT32 )
            .numComponents( 1 );

        mParticleListHeadsBuffer->CreateView( viewDesc, &pParticleListHeadsBufferUAV );
        mParticleListsBuffer->CreateView( viewDesc, &pParticleListsBufferUAV );

        viewDesc.ViewType = gx::BUFFER_VIEW_SHADER_RESOURCE;
        mParticleListHeadsBuffer->CreateView( viewDesc, &pParticleListHeadsBufferSRV );
        mParticleListsBuffer->CreateView( viewDesc, &pParticleListsBufferSRV );
    }

    mResetParticleListsSRB.Release();
    mResetParticleListsPSO->CreateShaderResourceBinding( &mResetParticleListsSRB, true );
    mResetParticleListsSRB->GetVariableByName( gx::SHADER_TYPE_COMPUTE, "g_ParticleListHead" )->Set( pParticleListHeadsBufferUAV );

    mRenderParticleSRB.Release();
    mRenderParticlePSO->CreateShaderResourceBinding( &mRenderParticleSRB, true );
    mRenderParticleSRB->GetVariableByName( gx::SHADER_TYPE_VERTEX, "g_Particles" )->Set( pParticleAttribsBufferSRV );

    mMoveParticlesSRB.Release();
    mMoveParticlesPSO->CreateShaderResourceBinding( &mMoveParticlesSRB, true );
    mMoveParticlesSRB->GetVariableByName( gx::SHADER_TYPE_COMPUTE, "g_Particles" )->Set( pParticleAttribsBufferUAV );
    mMoveParticlesSRB->GetVariableByName( gx::SHADER_TYPE_COMPUTE, "g_ParticleListHead" )->Set( pParticleListHeadsBufferUAV );
    mMoveParticlesSRB->GetVariableByName( gx::SHADER_TYPE_COMPUTE, "g_ParticleLists" )->Set( pParticleListsBufferUAV );

    mCollideParticlesSRB.Release();
    mCollideParticlesPSO->CreateShaderResourceBinding( &mCollideParticlesSRB, true );
    mCollideParticlesSRB->GetVariableByName( gx::SHADER_TYPE_COMPUTE, "g_Particles" )->Set( pParticleAttribsBufferUAV );
    mCollideParticlesSRB->GetVariableByName( gx::SHADER_TYPE_COMPUTE, "g_ParticleListHead" )->Set( pParticleListHeadsBufferSRV );
    mCollideParticlesSRB->GetVariableByName( gx::SHADER_TYPE_COMPUTE, "g_ParticleLists" )->Set( pParticleListsBufferSRV );
}

void ComputeShaderApp::createConsantBuffer()
{ 
    mConstants = gx::createBuffer( gx::BufferDesc()
        .name( "Constants buffer" )
        .usage( gx::USAGE_DYNAMIC )
        .bindFlags( gx::BIND_UNIFORM_BUFFER )
        .cpuAccessFlags( gx::CPU_ACCESS_WRITE )
        .sizeInBytes( sizeof( mat4 ) * 2 ), nullptr );
}

void ComputeShaderApp::updateUI()
{
    ImGui::SetNextWindowPos( ImVec2( 10, 10 ), ImGuiCond_FirstUseEver );
    if( ImGui::Begin( "Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
        if( ImGui::InputInt( "Num Particles", &mNumParticles, 100, 1000, ImGuiInputTextFlags_EnterReturnsTrue ) ) {
            mNumParticles = std::min( std::max( mNumParticles, 100 ), 1000000 );
            createParticleBuffers();
        }
        ImGui::SliderFloat( "Simulation Speed", &mSimulationSpeed, 0.1f, 5.f );
    }
    ImGui::End();
}


void ComputeShaderApp::update()
{
    utils::updateWindowTitle();

    updateUI();
    
    static double prevElapsedSeconds = getElapsedSeconds();
    mTimeDelta = static_cast<float>( getElapsedSeconds() - prevElapsedSeconds );
    prevElapsedSeconds = getElapsedSeconds();
}

void ComputeShaderApp::draw()
{
    // Clear the back buffer
	gx::clear( ColorA( 0.350f, 0.350f, 0.350f, 1.0f ) );

    {
        struct Constants
        {
            uint32_t    uiNumParticles;
            float       fDeltaTime;
            float       fDummy0;
            float       fDummy1;

            vec2        f2Scale;
            ivec2       i2ParticleGridSize;
        };
        // Map the buffer and write current world-view-projection matrix
        gx::MapHelper<Constants> constData( getImmediateContext(), mConstants, gx::MAP_WRITE, gx::MAP_FLAG_DISCARD );
        constData->uiNumParticles = static_cast<uint32_t>( mNumParticles );
        constData->fDeltaTime     = std::min( mTimeDelta, 1.f / 60.f ) * mSimulationSpeed;

        float  AspectRatio = getWindowAspectRatio();
        vec2 f2Scale     = vec2( std::sqrt( 1.f / AspectRatio ), std::sqrt( AspectRatio ) );
        constData->f2Scale = f2Scale;

        int iParticleGridWidth          = static_cast<int>( std::sqrt( static_cast<float>( mNumParticles ) ) / f2Scale.x );
        constData->i2ParticleGridSize.x = iParticleGridWidth;
        constData->i2ParticleGridSize.y = mNumParticles / iParticleGridWidth;
    }


    gx::DispatchComputeAttribs dispatchAttribs;
    dispatchAttribs.ThreadGroupCountX = ( mNumParticles + mThreadGroupSize - 1 ) / mThreadGroupSize;

    gx::setPipelineState( mResetParticleListsPSO );
    gx::commitShaderResources( mResetParticleListsSRB, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
    gx::dispatchCompute( dispatchAttribs );

    gx::setPipelineState( mMoveParticlesPSO );
    gx::commitShaderResources( mMoveParticlesSRB, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
    gx::dispatchCompute( dispatchAttribs );

    gx::setPipelineState( mCollideParticlesPSO );
    gx::commitShaderResources( mCollideParticlesSRB, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
    gx::dispatchCompute( dispatchAttribs );

    gx::setPipelineState( mUpdateParticleSpeedPSO );
    // Use the same SRB
    gx::commitShaderResources( mCollideParticlesSRB, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
    gx::dispatchCompute( dispatchAttribs );

    gx::setPipelineState( mRenderParticlePSO );
    gx::commitShaderResources( mRenderParticleSRB, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );

    gx::draw( gx::DrawAttribs().numVertices( 4 ).numInstances( static_cast<uint32_t>( mNumParticles ) ) );
}

CINDER_APP( ComputeShaderApp, RendererGx( RendererGx::Options().prepareEngineFn( ComputeShaderApp::prepareEngine ) ) )
