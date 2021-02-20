#include "cinder/app/App.h"
#include "cinder/app/RendererGx.h"
#include "cinder/FileWatcher.h"

#include "cinder/graphics/wrapper.h"
#include "cinder/graphics/Buffer.h"
#include "cinder/graphics/PipelineState.h"
#include "cinder/graphics/Shader.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class HotReloadApp : public App {
public:
    void setup() override;
    void update() override;
    void draw() override;

    void createPipelineState();
    void createVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffer();

    gx::PipelineStateRef         mPipelineState;
    gx::ShaderResourceBindingRef mSRB;
    gx::BufferRef                mCubeVertexBuffer;
    gx::BufferRef                mCubeIndexBuffer;
    gx::BufferRef                mVSConstants;
    mat4                         mWorldViewProjMatrix;
};

void HotReloadApp::setup()
{
    createUniformBuffer();
    createPipelineState();
    createVertexBuffer();
    createIndexBuffer();
}

void HotReloadApp::createUniformBuffer()
{
    // Create dynamic uniform buffer that will store our transformation matrix
    // Dynamic buffers can be frequently updated by the CPU
    mVSConstants = gx::createBuffer( gx::BufferDesc()
        .name( "VS constants CB" )
        .sizeInBytes( sizeof( mat4 ) )
        .usage( gx::USAGE_DYNAMIC )
        .bindFlags( gx::BIND_UNIFORM_BUFFER )
        .cpuAccessFlags( gx::CPU_ACCESS_WRITE )
    );
}

void HotReloadApp::createPipelineState()
{
    FileWatcher::instance().watch( { fs::path( "cube.psh" ), "cube.vsh " }, [this]( const WatchEvent &event ) {
        // Pipeline state object encompasses configuration of all GPU stages
        mPipelineState = gx::createGraphicsPipelineState( gx::GraphicsPipelineStateCreateInfo()
            // Pipeline state name is used by the engine to report issues. It is always a good idea to give objects descriptive names.
            .name( "Cube PSO" )
            // Define vertex shader input layout
            .inputLayout( {
                // Attribute 0 - vertex position
                gx::LayoutElement{ 0, 0, 3, gx::VT_FLOAT32, false },
                // Attribute 1 - vertex color
                gx::LayoutElement{ 1, 0, 4, gx::VT_FLOAT32, false }
                } )
                // Create a vertex shader
            .vertexShader( gx::createShader( gx::ShaderCreateInfo()
                .name( "Cube VS" )
                // Tell the system that the shader source code is in HLSL. For OpenGL, the engine will convert this into GLSL under the hood.
                .sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
                // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
                .useCombinedTextureSamplers( true )
                .shaderType( gx::SHADER_TYPE_VERTEX )
                .filePath( getAssetPath( "cube.vsh" ) )
            ) )
            // Create a pixel shader
            .pixelShader( gx::createShader( gx::ShaderCreateInfo()
                .name( "Cube PS" )
                // Tell the system that the shader source code is in HLSL. For OpenGL, the engine will convert this into GLSL under the hood.
                .sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
                // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
                .useCombinedTextureSamplers( true )
                .shaderType( gx::SHADER_TYPE_PIXEL )
                .filePath( getAssetPath( "cube.psh" ) )
            ) )
            // Define variable type that will be used by default
            .resourceLayout( gx::PipelineResourceLayoutDesc().defaultVariableType( gx::SHADER_RESOURCE_VARIABLE_TYPE_STATIC ) )
        );

        // Since we did not explcitly specify the type for 'Constants' variable, default
        // type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables never
        // change and are bound directly through the pipeline state object.
        mPipelineState->GetStaticVariableByName( gx::SHADER_TYPE_VERTEX, "Constants" )->Set( mVSConstants );
        // Create a shader resource binding object and bind all static resources in it
        mPipelineState->CreateShaderResourceBinding( &mSRB, true );
    } );
}

void HotReloadApp::createVertexBuffer()
{
    // Layout of this structure matches the one we defined in the pipeline state
    struct Vertex {
        vec3 pos;
        vec4 color;
    };

    // Cube vertices
    //    5-(-1,+1,+1)________________6-(+1,+1,+1)
    //               /|              /|
    //              / |             / |
    //             /  |            /  |
    //            /   |           /   |
    //4(-1,-1,+1)/____|__________/7-(+1,-1,+1)
    //           |    |__________|____|
    //           |   /1(-1,+1,-1)|    /2-(+1,+1,-1)
    //           |  /            |   /
    //           | /             |  /
    //           |/              | /
    //           /_______________|/
    //        0-(-1,-1,-1)    3-(+1,-1,-1)
    //

    Vertex cubeVerts[] = {
        { vec3( -1,-1,-1 ), vec4( 1,0,0,1 ) },
        { vec3( -1,+1,-1 ), vec4( 0,1,0,1 ) },
        { vec3( +1,+1,-1 ), vec4( 0,0,1,1 ) },
        { vec3( +1,-1,-1 ), vec4( 1,1,1,1 ) },

        { vec3( -1,-1,+1 ), vec4( 1,1,0,1 ) },
        { vec3( -1,+1,+1 ), vec4( 0,1,1,1 ) },
        { vec3( +1,+1,+1 ), vec4( 1,0,1,1 ) },
        { vec3( +1,-1,+1 ), vec4( 0.2f,0.2f,0.2f,1 ) },
    };

    // Create a vertex buffer that stores cube vertices
    gx::BufferData data = { cubeVerts, sizeof( cubeVerts ) };
    mCubeVertexBuffer = gx::createBuffer( gx::BufferDesc()
        .name( "Cube vertex buffer" )
        .usage( gx::USAGE_IMMUTABLE )
        .bindFlags( gx::BIND_VERTEX_BUFFER )
        .sizeInBytes( sizeof( cubeVerts ) ),
        &data
    );
}

void HotReloadApp::createIndexBuffer()
{
    uint32_t indices[] = {
        2,0,1, 2,3,0,
        4,6,5, 4,7,6,
        0,7,4, 0,3,7,
        1,0,4, 1,4,5,
        1,5,2, 5,6,2,
        3,6,7, 3,2,6
    };

    gx::BufferData data = { indices, sizeof( indices ) };
    mCubeIndexBuffer = gx::createBuffer( gx::BufferDesc()
        .name( "Cube index buffer" )
        .usage( gx::USAGE_IMMUTABLE )
        .bindFlags( gx::BIND_INDEX_BUFFER )
        .sizeInBytes( sizeof( indices ) ),
        &data
    );
}

void HotReloadApp::update()
{
    // Apply rotation
    mat4 model = glm::rotate( glm::pi<float>() * 0.1f, vec3( 1.0f, 0.0f, 0.0f ) ) * glm::rotate( (float) getElapsedSeconds(), vec3( 0.0f, 1.0f, 0.0f ) );
    // Camera is at (0, 0, -5) looking along the Z axis
    mat4 view = glm::lookAt( glm::vec3( 0.f, 0.0f, 5.0f ), vec3( 0.0f ), vec3( 0.0f, 1.0f, 0.0f ) );
    // Get projection matrix adjusted to the current screen orientation
    mat4 proj = glm::perspective( glm::pi<float>() / 4.0f, getWindowAspectRatio(), 0.1f, 100.f );
    // Compute world-view-projection matrix
    mWorldViewProjMatrix = proj * view * model;
}

void HotReloadApp::draw()
{
    // Clear the back buffer
    gx::clear( ColorA( 0.350f, 0.350f, 0.350f, 1.0f ) );

    {   // Map the buffer and write current world-view-projection matrix
        gx::MapHelper<mat4> constants( getImmediateContext(), mVSConstants, gx::MAP_WRITE, gx::MAP_FLAG_DISCARD );
        *constants = glm::transpose( mWorldViewProjMatrix );
    }

    // Bind vertex and index buffers
    gx::setVertexBuffer( mCubeVertexBuffer, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, gx::SET_VERTEX_BUFFERS_FLAG_RESET );
    gx::setIndexBuffer( mCubeIndexBuffer, 0, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
    // Set the pipeline state
    gx::setPipelineState( mPipelineState );
    // Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode makes sure that resources are transitioned to required states.
    gx::commitShaderResources( mSRB, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
    // This is an indexed draw call
    // Verify the state of vertex and index buffers
    gx::drawIndexed( gx::DrawIndexedAttribs().indexType( gx::VT_UINT32 ).numIndices( 36 ).flags( gx::DRAW_FLAG_VERIFY_ALL ) );
}

CINDER_APP( HotReloadApp, RendererGx )
