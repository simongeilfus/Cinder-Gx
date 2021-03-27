#include "cinder/app/App.h"
#include "cinder/app/RendererGx.h"

#include "cinder/graphics/wrapper.h"
#include "cinder/graphics/Buffer.h"
#include "cinder/graphics/DeviceContext.h"
#include "cinder/graphics/PipelineState.h"
#include "cinder/graphics/Shader.h"
#include "cinder/graphics/Texture.h"
#include "cinder/graphics/Query.h"

#include "cinder/CinderDiligentImGui.h"

#include "TexturedCube.h"
#include "Utils.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class QueriesApp : public App {
public:
    void setup() override;
    void update() override;
    void draw() override;

    void createCubePSO();
    void updateUI();

    gx::PipelineStateRef         mCubePSO;
    gx::ShaderResourceBindingRef mCubeSRB;
    gx::BufferRef                mCubeVertexBuffer;
    gx::BufferRef                mCubeIndexBuffer;
    gx::BufferRef                mCubeVSConstants;
    gx::TextureViewRef           mCubeTextureSRV;

    std::unique_ptr<gx::ScopedQueryHelper>   mPipelineStatsQuery;
    std::unique_ptr<gx::ScopedQueryHelper>   mOcclusionQuery;
    std::unique_ptr<gx::ScopedQueryHelper>   mDurationQuery;
    std::unique_ptr<gx::DurationQueryHelper> mDurationFromTimestamps;

    gx::QueryDataPipelineStatistics mPipelineStatsData;
    gx::QueryDataOcclusion          mOcclusionData;
    gx::QueryDataDuration           mDurationData;

    double                      mDuration = 0;

    glm::mat4 mWorldViewProjMatrix;
};

void QueriesApp::setup()
{
    ImGui::DiligentInitialize();
    createCubePSO();

    // Load textured cube
    mCubeVertexBuffer = TexturedCube::createVertexBuffer( getRenderDevice() );
    mCubeIndexBuffer = TexturedCube::createIndexBuffer( getRenderDevice() );
    mCubeTextureSRV = TexturedCube::loadTexture( getRenderDevice(), getAssetPath( "DGLogo.png" ) )->GetDefaultView( gx::TEXTURE_VIEW_SHADER_RESOURCE );

    // Set cube texture SRV in the SRB
    mCubeSRB->GetVariableByName( gx::SHADER_TYPE_PIXEL, "g_Texture" )->Set( mCubeTextureSRV );

    // Check query support
    const auto& features = getRenderDevice()->GetDeviceCaps().Features;
    if( features.PipelineStatisticsQueries ) {
        mPipelineStatsQuery.reset( new gx::ScopedQueryHelper{ getRenderDevice(), gx::QueryDesc()
            .type( gx::QUERY_TYPE_PIPELINE_STATISTICS )
            .name( "Pipeline statistics query" ), 2 } );
    }

    if( features.OcclusionQueries ) {
        mOcclusionQuery.reset( new gx::ScopedQueryHelper{ getRenderDevice(), gx::QueryDesc()
            .type( gx::QUERY_TYPE_OCCLUSION )
            .name( "Occlusion query" ), 2 } );
    }

    if( features.DurationQueries ) {
        mDurationQuery.reset( new gx::ScopedQueryHelper{ getRenderDevice(), gx::QueryDesc()
            .type( gx::QUERY_TYPE_DURATION )
            .name( "Duration query" ), 2 } );
    }

    if( features.TimestampQueries ) {
        mDurationFromTimestamps.reset( new gx::DurationQueryHelper{ getRenderDevice(), 2 } );
    }
}

void QueriesApp::createCubePSO()
{
    mCubePSO = TexturedCube::createPipelineState( getRenderDevice(), getSwapChainColorFormat(), getSwapChainDepthFormat(), getAssetPath( "cube.vsh" ), getAssetPath( "cube.psh" ) );
 
    // Create dynamic uniform buffer that will store our transformation matrix
    // Dynamic buffers can be frequently updated by the CPU
    mCubeVSConstants = gx::createBuffer( gx::BufferDesc()
        .name( "VS constants CB" )
        .sizeInBytes( sizeof( mat4 ) )
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

void QueriesApp::updateUI()
{
    ImGui::SetNextWindowPos( ImVec2( 10, 10 ), ImGuiCond_FirstUseEver );
    if( ImGui::Begin( "Query data", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
        if( mPipelineStatsQuery || mOcclusionQuery || mDurationQuery || mDurationFromTimestamps ) {
            std::stringstream params_ss, values_ss;
            if( mPipelineStatsQuery ) {
                params_ss << "Input vertices" << std::endl
                    << "Input primitives" << std::endl
                    << "VS Invocations" << std::endl
                    << "Clipping Invocations" << std::endl
                    << "Rasterized Primitives" << std::endl
                    << "PS Invocations" << std::endl;

                values_ss << mPipelineStatsData.InputVertices << std::endl
                    << mPipelineStatsData.InputPrimitives << std::endl
                    << mPipelineStatsData.VSInvocations << std::endl
                    << mPipelineStatsData.ClippingInvocations << std::endl
                    << mPipelineStatsData.ClippingPrimitives << std::endl
                    << mPipelineStatsData.PSInvocations << std::endl;
            }

            if( mOcclusionQuery ) {
                params_ss << "Samples rendered" << std::endl;
                values_ss << mOcclusionData.NumSamples << std::endl;
            }

            if( mDurationQuery ) {
                if( mDurationData.Frequency > 0 ) {
                    params_ss << "Duration (mus)" << std::endl;
                    values_ss << std::fixed << std::setprecision( 0 )
                        << static_cast<float>( mDurationData.Duration ) / static_cast<float>( mDurationData.Frequency ) * 1000000.f << std::endl;
                }
                else {
                    params_ss << "Duration unavailable" << std::endl;
                }
            }

            if( mDurationFromTimestamps ) {
                params_ss << "Duration from TS (mus)" << std::endl;
                values_ss << static_cast<int>( mDuration * 1000000 ) << std::endl;
            }

            ImGui::TextDisabled( "%s", params_ss.str().c_str() );
            ImGui::SameLine();
            ImGui::TextDisabled( "%s", values_ss.str().c_str() );
        }
        else {
            ImGui::TextDisabled( "Queries are not supported by this device" );
        }
    }
    ImGui::End();
}

void QueriesApp::update()
{
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

void QueriesApp::draw()
{
    // Clear the back buffer
	gx::clear( ColorA( 0.350f, 0.350f, 0.350f, 1.0f ) );

    {   // Map the buffer and write current world-view-projection matrix
        gx::MapHelper<mat4> constants( getImmediateContext(), mCubeVSConstants, gx::MAP_WRITE, gx::MAP_FLAG_DISCARD );
        *constants = glm::transpose( mWorldViewProjMatrix );
    }

    // Bind vertex and index buffers
    gx::setVertexBuffer( mCubeVertexBuffer, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, gx::SET_VERTEX_BUFFERS_FLAG_RESET );
    gx::setIndexBuffer( mCubeIndexBuffer, 0, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );

    // Set the cube's pipeline state
    gx::setPipelineState( mCubePSO );

    // Commit the cube shader's resources
    gx::commitShaderResources( mCubeSRB, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );

    // Begin supported queries
    if( mPipelineStatsQuery )
        mPipelineStatsQuery->Begin( getImmediateContext() );
    if( mOcclusionQuery )
        mOcclusionQuery->Begin( getImmediateContext() );
    if( mDurationFromTimestamps )
        mDurationFromTimestamps->Begin( getImmediateContext() );
    if( mDurationQuery )
        mDurationQuery->Begin( getImmediateContext() );

    // Draw the cube
    gx::drawIndexed( gx::DrawIndexedAttribs().indexType( gx::VT_UINT32 ).numIndices( 36 ).flags( gx::DRAW_FLAG_VERIFY_ALL ) );

    // End queries
    if( mDurationFromTimestamps )
        mDurationFromTimestamps->End( getImmediateContext(), mDuration );
    // Note that recording the query itself may take measurable amount of time, so
    // if mDurationFromTimestamps and mDurationQuery queries are nested, the results
    // may noticeably differ.
    if( mDurationQuery )
        mDurationQuery->End( getImmediateContext(), &mDurationData, sizeof( mDurationData ) );
    if( mOcclusionQuery )
        mOcclusionQuery->End( getImmediateContext(), &mOcclusionData, sizeof( mOcclusionData ) );
    if( mPipelineStatsQuery )
        mPipelineStatsQuery->End( getImmediateContext(), &mPipelineStatsData, sizeof( mPipelineStatsData ) );
}

CINDER_APP( QueriesApp, RendererGx( RendererGx::Options()
	.deviceFeatures( gx::DeviceFeatures()
		.occlusionQueries( gx::DEVICE_FEATURE_STATE_OPTIONAL )
		.binaryOcclusionQueries( gx::DEVICE_FEATURE_STATE_OPTIONAL )
		.timestampQueries( gx::DEVICE_FEATURE_STATE_OPTIONAL )
		.pipelineStatisticsQueries( gx::DEVICE_FEATURE_STATE_OPTIONAL )
		.durationQueries( gx::DEVICE_FEATURE_STATE_OPTIONAL )
	) 
) )
