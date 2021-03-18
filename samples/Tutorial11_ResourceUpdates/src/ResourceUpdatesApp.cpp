#include "cinder/app/App.h"
#include "cinder/app/RendererGx.h"

#include "cinder/graphics/wrapper.h"
#include "cinder/graphics/Buffer.h"
#include "cinder/graphics/PipelineState.h"
#include "cinder/graphics/Shader.h"
#include "cinder/graphics/Texture.h"

#include "cinder/CinderDiligentImGui.h"

#include "Utils.h"

#include <array>
#include <random>

using namespace ci;
using namespace ci::app;
using namespace std;

// Layout of this structure matches the one we defined in the pipeline state
struct Vertex {
    vec3 pos;
    vec2 uv;
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
const Vertex cubeVerts[] = {
    { vec3( -1,-1,-1 ), vec2( 0,1 ) },
    { vec3( -1,+1,-1 ), vec2( 0,0 ) },
    { vec3( +1,+1,-1 ), vec2( 1,0 ) },
    { vec3( +1,-1,-1 ), vec2( 1,1 ) },

    { vec3( -1,-1,-1 ), vec2( 0,1 ) },
    { vec3( -1,-1,+1 ), vec2( 0,0 ) },
    { vec3( +1,-1,+1 ), vec2( 1,0 ) },
    { vec3( +1,-1,-1 ), vec2( 1,1 ) },

    { vec3( +1,-1,-1 ), vec2( 0,1 ) },
    { vec3( +1,-1,+1 ), vec2( 1,1 ) },
    { vec3( +1,+1,+1 ), vec2( 1,0 ) },
    { vec3( +1,+1,-1 ), vec2( 0,0 ) },

    { vec3( +1,+1,-1 ), vec2( 0,1 ) },
    { vec3( +1,+1,+1 ), vec2( 0,0 ) },
    { vec3( -1,+1,+1 ), vec2( 1,0 ) },
    { vec3( -1,+1,-1 ), vec2( 1,1 ) },
    
    { vec3( -1,+1,-1 ), vec2( 1,0 ) },
    { vec3( -1,+1,+1 ), vec2( 0,0 ) },
    { vec3( -1,-1,+1 ), vec2( 0,1 ) },
    { vec3( -1,-1,-1 ), vec2( 1,1 ) },

    { vec3( -1,-1,+1 ), vec2( 1,1 ) },
    { vec3( +1,-1,+1 ), vec2( 0,1 ) },
    { vec3( +1,+1,+1 ), vec2( 0,0 ) },
    { vec3( -1,+1,+1 ), vec2( 1,0 ) }
};

class ResourceUpdatesApp : public App {
  public:
	void setup() override;
	void update() override;
	void draw() override;

	static void prepareEngine( gx::RENDER_DEVICE_TYPE deviceType, gx::EngineCreateInfo* engineCreateInfo, gx::SwapChainDesc* swapChainDesc );

    void createPipelineStates();
    void createVertexBuffers();
    void createIndexBuffer();
    void loadTextures();

    void writeStripPattern( uint8_t*, uint32_t Width, uint32_t Height, uint32_t Stride );
    void writeDiamondPattern( uint8_t*, uint32_t Width, uint32_t Height, uint32_t Stride );

    void updateTexture( uint32_t TexIndex );
    void mapTexture( uint32_t TexIndex, bool MapEntireTexture );
    void updateBuffer( uint32_t BufferIndex );
    void mapDynamicBuffer( uint32_t BufferIndex );

    gx::PipelineStateRef mPSO, mPSONoCull;
    gx::BufferRef        mCubeVertexBuffer[3];
    gx::BufferRef        mCubeIndexBuffer;
    gx::BufferRef        mVSConstants;
    gx::BufferRef        mTextureupdateBuffer;

    void drawCube( const mat4 &WVPMatrix, gx::Buffer* pVertexBuffer, gx::ShaderResourceBinding* pSRB );

    static constexpr const size_t   sNumTextures         = 4;
    static constexpr const uint32_t sMaxUpdateRegionSize = 128;
    static constexpr const uint32_t sMaxMapRegionSize    = 128;

    std::array<gx::TextureRef,sNumTextures>               mTextures;
    std::array<gx::ShaderResourceBindingRef,sNumTextures> mSRBs;

    double       mLastTextureUpdateTime = 0;
    double       mLastBufferUpdateTime  = 0;
    double       mLastMapTime           = 0;
    std::mt19937 mGen{ 0 }; //Use 0 as the seed to always generate the same sequence
};

void ResourceUpdatesApp::prepareEngine( gx::RENDER_DEVICE_TYPE deviceType, gx::EngineCreateInfo* engineCreateInfo, gx::SwapChainDesc* swapChainDesc )
{
}

void ResourceUpdatesApp::setup()
{
    disableFrameRate();
    ImGui::DiligentInitialize();

    createPipelineStates();
    createVertexBuffers();
    createIndexBuffer();
    loadTextures();

    mTextureupdateBuffer = gx::createBuffer( gx::BufferDesc()
        .name( "Texture update buffer" )
        .usage( gx::USAGE_DYNAMIC )
        // We do not really bind the buffer, but D3D11 wants at least one bind flag bit
        .bindFlags( gx::BIND_VERTEX_BUFFER )
        .cpuAccessFlags( gx::CPU_ACCESS_WRITE )
        .sizeInBytes( sMaxUpdateRegionSize * sMaxUpdateRegionSize * 4 )
    );
}

void ResourceUpdatesApp::createPipelineStates()
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

    // Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
    gx::SamplerDesc samLinearClampDesc {
        gx::FILTER_TYPE_LINEAR, gx::FILTER_TYPE_LINEAR, gx::FILTER_TYPE_LINEAR,
        gx::TEXTURE_ADDRESS_CLAMP, gx::TEXTURE_ADDRESS_CLAMP, gx::TEXTURE_ADDRESS_CLAMP
    };

    gx::ShaderCreateInfo shaderCreateInfo = gx::ShaderCreateInfo()
         // Tell the system that the shader source code is in HLSL. For OpenGL, the engine will convert this into GLSL under the hood.
        .sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
        // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
        .useCombinedTextureSamplers( true );

    // Pipeline state object encompasses configuration of all GPU stages
    gx::GraphicsPipelineDesc pipelineDesc = gx::GraphicsPipelineDesc()
        // Pipeline state name is used by the engine to report issues.
        // It is always a good idea to give objects descriptive names.
        .name( "Cube PSO" )
        // Primitive topology defines what kind of primitives will be rendered by this pipeline state
        .primitiveTopology( gx::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST )
        // Cull back faces
        .cullMode( gx::CULL_MODE_BACK )
        // Enable depth testing
        .depthEnable( true )
         // Create a plane vertex shader
        .vertexShader( shaderCreateInfo.name( "Cube VS" ).filePath( getAssetPath( "cube.vsh" ) ) )
        .pixelShader( shaderCreateInfo.name( "Cube PS" ).filePath( getAssetPath( "cube.psh" ) ) )
        // Define vertex shader input layout
        .inputLayout( {
            // Attribute 0 - vertex position
            { 0, 0, 3, gx::VT_FLOAT32, false },
            // Attribute 1 - texture coordinates
            { 1, 0, 2, gx::VT_FLOAT32, false }
            } )
        .variables( { { gx::SHADER_TYPE_PIXEL, "g_Texture", gx::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE } } )
        .immutableSamplers( { { gx::SHADER_TYPE_PIXEL, "g_Texture", samLinearClampDesc } } );
    mPSO = gx::createGraphicsPipelineState( pipelineDesc );

    // Since we did not explcitly specify the type for 'Constants' variable, default
    // type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables never
    // change and are bound directly to the pipeline state object.
    mPSO->GetStaticVariableByName( gx::SHADER_TYPE_VERTEX, "Constants" )->Set( mVSConstants );

    // same pipeline but with disabled cull mode
    mPSONoCull = gx::createGraphicsPipelineState( pipelineDesc.cullMode( gx::CULL_MODE_NONE ) );
    mPSONoCull->GetStaticVariableByName( gx::SHADER_TYPE_VERTEX, "Constants" )->Set( mVSConstants );
}

void ResourceUpdatesApp::createVertexBuffers()
{
    for( uint32_t i = 0; i < _countof( mCubeVertexBuffer ); ++i ) {
        // Create vertex buffer that stores cube vertices
        mCubeVertexBuffer[i] = gx::createBuffer( gx::BufferDesc()
            .name( "Cube vertex buffer" )
            .usage( i == 0 ? gx::USAGE_IMMUTABLE : i == 1 ? gx::USAGE_DEFAULT : gx::USAGE_DYNAMIC )
            .bindFlags( gx::BIND_VERTEX_BUFFER )
            .cpuAccessFlags( i > 1 ? gx::CPU_ACCESS_WRITE : gx::CPU_ACCESS_NONE )
            .sizeInBytes( sizeof( cubeVerts ) ),
            i < 2 ? &cubeVerts : nullptr, i < 2 ? sizeof( cubeVerts ) : 0
        );
    }
}

void ResourceUpdatesApp::createIndexBuffer()
{
    uint32_t indices[] = {
        2,0,1,     2,3,0,
        4,6,5,     4,7,6,
        8,10,9,    8,11,10,
        12,14,13,  12,15,14,
        16,18,17,  16,19,18,
        20,21,22,  20,22,23
    };

    // Create index buffer
    mCubeIndexBuffer = gx::createBuffer( gx::BufferDesc()
        .name( "Cube index buffer" )
        .usage( gx::USAGE_IMMUTABLE )
        .bindFlags( gx::BIND_INDEX_BUFFER )
        .sizeInBytes( sizeof( indices ) ),
        &indices, sizeof( indices )
    );
}

void ResourceUpdatesApp::loadTextures()
{
    for( size_t i = 0; i < mTextures.size(); ++i ) {
        // Load texture
        std::stringstream fileNameSS;
        fileNameSS << "DGLogo" << i << ".png";
        std::string fileName = fileNameSS.str();
        gx::TextureDesc textureDesc = gx::TextureDesc().usage( gx::USAGE_IMMUTABLE );

        if( i == 2 ) {
            textureDesc.usage( gx::USAGE_DEFAULT );
            // Disable mipmapping for simplicity as we will only update mip level 0
            textureDesc.mipLevels( 1 );
        }
        else if( i == 3 ) {
            // Disable mipmapping
            textureDesc.mipLevels( 1 );
            textureDesc.usage( gx::USAGE_DYNAMIC );
            textureDesc.cpuAccessFlags( gx::CPU_ACCESS_WRITE );
        }

        mTextures[i] = gx::createTexture( loadImage( loadAsset( fileName ) ), textureDesc );
        // Get shader resource view from the texture
        auto TextureSRV = mTextures[i]->GetDefaultView( gx::TEXTURE_VIEW_SHADER_RESOURCE );

        // Since we are using mutable variable, we must create shader resource binding object
        // http://diligentgraphics.com/2016/03/23/resource-binding-model-in-diligent-engine-2-0/
        mPSO->CreateShaderResourceBinding( &( mSRBs[i] ), true );
        // Set texture SRV in the SRB
        mSRBs[i]->GetVariableByName( gx::SHADER_TYPE_PIXEL, "g_Texture" )->Set( TextureSRV );
    }
}

void ResourceUpdatesApp::writeStripPattern( uint8_t* pData, uint32_t Width, uint32_t Height, uint32_t Stride )
{
    auto x_scale = std::uniform_int_distribution<uint32_t>{ 1, 8 }( mGen );
    auto y_scale = std::uniform_int_distribution<uint32_t>{ 1, 8 }( mGen );
    auto c_scale = std::uniform_int_distribution<uint32_t>{ 1, 64 }( mGen );
    for( uint32_t j = 0; j < Height; ++j ) {
        for( uint32_t i = 0; i < Width; ++i ) {
            for( int c = 0; c < 4; ++c )
                pData[i * 4 + c + j * Stride] = ( i * x_scale + j * y_scale + c * c_scale ) & 0xFF;
        }
    }
}

void ResourceUpdatesApp::writeDiamondPattern( uint8_t* pData, uint32_t Width, uint32_t Height, uint32_t Stride )
{
    auto x_scale = std::uniform_int_distribution<uint32_t>{ 1, 8 }( mGen );
    auto y_scale = std::uniform_int_distribution<uint32_t>{ 1, 8 }( mGen );
    auto c_scale = std::uniform_int_distribution<uint32_t>{ 1, 64 }( mGen );
    for( uint32_t j = 0; j < Height; ++j ) {
        for( uint32_t i = 0; i < Width; ++i ) {
            for( int c = 0; c < 4; ++c )
                pData[i * 4 + c + j * Stride] = ( ::abs( static_cast<int>( i ) - static_cast<int>( Width / 2 ) ) * x_scale + ::abs( static_cast<int>( j ) - static_cast<int>( Height / 2 ) ) * y_scale + c * c_scale ) & 0xFF;
        }
    }
}

void ResourceUpdatesApp::updateTexture( uint32_t TexIndex )
{
    auto& Texture = *mTextures[TexIndex];

    static constexpr const uint32_t NumUpdates = 3;
    for( uint32_t update = 0; update < NumUpdates; ++update ) {
        const auto& TexDesc = Texture.GetDesc();

        uint32_t Width  = std::uniform_int_distribution<uint32_t>{ 2, sMaxUpdateRegionSize }( mGen );
        uint32_t Height = std::uniform_int_distribution<uint32_t>{ 2, sMaxUpdateRegionSize }( mGen );

        std::vector<uint8_t> Data( Width * Height * 4 );
        writeStripPattern( Data.data(), Width, Height, Width * 4 );

        gx::Box UpdateBox;
        UpdateBox.MinX = std::uniform_int_distribution<uint32_t>{ 0, TexDesc.Width - Width }( mGen );
        UpdateBox.MinY = std::uniform_int_distribution<uint32_t>{ 0, TexDesc.Height - Height }( mGen );
        UpdateBox.MaxX = UpdateBox.MinX + Width;
        UpdateBox.MaxY = UpdateBox.MinY + Height;

        gx::TextureSubResData SubresData;
        SubresData.Stride = Width * 4;
        SubresData.pData  = Data.data();
        uint32_t MipLevel   = 0;
        uint32_t ArraySlice = 0;
        gx::updateTexture( &Texture, MipLevel, ArraySlice, UpdateBox, SubresData, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
    }
}

void ResourceUpdatesApp::mapTexture( uint32_t TexIndex, bool MapEntireTexture )
{
    auto&       Texture = *mTextures[TexIndex];
    const auto& TexDesc = mTextures[2]->GetDesc();

    gx::MappedTextureSubresource MappedSubres;
    gx::Box                      MapRegion;
    if( MapEntireTexture ) {
        MapRegion.MaxX = TexDesc.Width;
        MapRegion.MaxY = TexDesc.Height;
    }
    else {
        uint32_t Width   = std::uniform_int_distribution<uint32_t>{ 2, sMaxMapRegionSize }( mGen );
        uint32_t Height  = std::uniform_int_distribution<uint32_t>{ 2, sMaxMapRegionSize }( mGen );
        MapRegion.MinX = std::uniform_int_distribution<uint32_t>{ 0, TexDesc.Width - Width }( mGen );
        MapRegion.MinY = std::uniform_int_distribution<uint32_t>{ 0, TexDesc.Height - Height }( mGen );
        MapRegion.MaxX = MapRegion.MinX + Width;
        MapRegion.MaxY = MapRegion.MinY + Height;
    }
    uint32_t MipLevel   = 0;
    uint32_t ArraySlice = 0;
    gx::mapTextureSubresource( &Texture, MipLevel, ArraySlice, gx::MAP_WRITE, gx::MAP_FLAG_DISCARD, MapEntireTexture ? nullptr : &MapRegion, MappedSubres );
    writeDiamondPattern( (uint8_t*) MappedSubres.pData, MapRegion.MaxX - MapRegion.MinX, MapRegion.MaxY - MapRegion.MinY, MappedSubres.Stride );
    gx::unmapTextureSubresource( &Texture, 0, 0 );
}

void ResourceUpdatesApp::updateBuffer( uint32_t BufferIndex )
{
    uint32_t NumVertsToUpdate  = std::uniform_int_distribution<uint32_t>{ 2, 8 }( mGen );
    uint32_t FirstVertToUpdate = std::uniform_int_distribution<uint32_t>{ 0, static_cast<uint32_t>( _countof( cubeVerts ) ) - NumVertsToUpdate }( mGen );
    Vertex Vertices[_countof( cubeVerts )];
    for( uint32_t v = 0; v < NumVertsToUpdate; ++v ) {
        auto        SrcInd  = FirstVertToUpdate + v;
        const auto& SrcVert = cubeVerts[SrcInd];
        Vertices[v].uv      = SrcVert.uv;
        Vertices[v].pos     = SrcVert.pos * static_cast<float>( 1 + 0.2 * sin( getElapsedSeconds() * ( 1.0 + SrcInd * 0.2 ) ) );
    }
    gx::updateBuffer(
        mCubeVertexBuffer[BufferIndex],    // Device context to use for the operation
        FirstVertToUpdate * sizeof( Vertex ), // Start offset in bytes
        NumVertsToUpdate * sizeof( Vertex ),  // Data size in bytes
        Vertices,                           // Data pointer
        gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
}

void ResourceUpdatesApp::mapDynamicBuffer( uint32_t BufferIndex )
{
    // Map the buffer and write current world-view-projection matrix
    gx::MapHelper<Vertex> Vertices( getImmediateContext(), mCubeVertexBuffer[BufferIndex], gx::MAP_WRITE, gx::MAP_FLAG_DISCARD );
    for( uint32_t v = 0; v < _countof( cubeVerts ); ++v ) {
        const auto& SrcVert = cubeVerts[v];
        Vertices[v].uv      = SrcVert.uv;
        Vertices[v].pos     = SrcVert.pos * static_cast<float>( 1 + 0.2 * sin( getElapsedSeconds() * ( 1.0 + v * 0.2 ) ) );
    }
}

void ResourceUpdatesApp::update()
{
    utils::updateWindowTitle();

    static constexpr const double updateBufferPeriod = 0.1;
    if( getElapsedSeconds() - mLastBufferUpdateTime > updateBufferPeriod ) {
        mLastBufferUpdateTime = getElapsedSeconds();
        updateBuffer( 1 );
    }

    mapDynamicBuffer( 2 );

    static constexpr const double updateTexturePeriod = 0.5;
    if( getElapsedSeconds() - mLastTextureUpdateTime > updateTexturePeriod ) {
        mLastTextureUpdateTime = getElapsedSeconds();
        updateTexture( 2 );
    }

    static constexpr const double mapTexturePeriod = 0.05;
    const auto&                   deviceType       = getRenderDevice()->GetDeviceCaps().DevType;
    if( getElapsedSeconds() - mLastMapTime > mapTexturePeriod * ( deviceType == gx::RENDER_DEVICE_TYPE_D3D11 ? 10.f : 1.f ) ) {
        mLastMapTime = getElapsedSeconds();
        if( deviceType == gx::RENDER_DEVICE_TYPE_D3D11 ||
            deviceType == gx::RENDER_DEVICE_TYPE_D3D12 ||
            deviceType == gx::RENDER_DEVICE_TYPE_VULKAN ||
            deviceType == gx::RENDER_DEVICE_TYPE_METAL ) {
            mapTexture( 3, deviceType == gx::RENDER_DEVICE_TYPE_D3D11 );
        }
    }
}

void ResourceUpdatesApp::drawCube( const mat4 &WVPMatrix, gx::Buffer* pVertexBuffer, gx::ShaderResourceBinding* pSRB )
{
    // Bind vertex buffer
    gx::setVertexBuffer( pVertexBuffer, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, gx::SET_VERTEX_BUFFERS_FLAG_RESET );
    gx::setIndexBuffer( mCubeIndexBuffer, 0, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );

    // Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode
    // makes sure that resources are transitioned to required states.
    gx::commitShaderResources( pSRB, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );

    {
        // Map the buffer and write current world-view-projection matrix
        gx::MapHelper<mat4> CBConstants( getImmediateContext(), mVSConstants, gx::MAP_WRITE, gx::MAP_FLAG_DISCARD );
        *CBConstants = glm::transpose( WVPMatrix );
    }

    gx::drawIndexed( gx::DrawIndexedAttribs().indexType( gx::VT_UINT32 ).numIndices( 36 ).flags( gx::DRAW_FLAG_VERIFY_ALL ) );
}

void ResourceUpdatesApp::draw()
{
    // Clear the back buffer
    gx::clear( ColorA( 0.350f, 0.350f, 0.350f, 1.0f ) );

    // Set the pipeline state
    gx::setPipelineState( mPSO );

    // Get projection matrix adjusted to the current screen orientation
    mat4 proj = glm::perspective( glm::pi<float>() / 4.0f, getWindowAspectRatio(), 0.1f, 100.f );
    auto ViewProj = proj;
    auto CubeRotation = glm::translate( vec3( 0.f, 0.0f, 12.0f ) ) *  glm::rotate( -glm::pi<float>() * 0.1f, vec3( 1.0f, 0.0f, 0.0f ) ) * glm::rotate( static_cast<float>( getElapsedSeconds() ) * 0.5f, vec3( 0.0f, 1.0f, 0.0f ) );
    
    drawCube( ViewProj * glm::translate( vec3( -2.f, -2.f, 0.f ) ) * CubeRotation, mCubeVertexBuffer[0], mSRBs[2] );
    drawCube( ViewProj * glm::translate( vec3( +2.f, -2.f, 0.f ) ) * CubeRotation, mCubeVertexBuffer[0], mSRBs[3] );

    drawCube( ViewProj * glm::translate( vec3( -4.f, +2.f, 0.f ) ) * CubeRotation, mCubeVertexBuffer[0], mSRBs[0] );

    gx::setPipelineState( mPSONoCull );

    drawCube( ViewProj * glm::translate( vec3( 0.f, +2.f, 0.f ) ) * CubeRotation, mCubeVertexBuffer[1], mSRBs[0] );
    drawCube( ViewProj * glm::translate( vec3( +4.f, +2.f, 0.f ) ) * CubeRotation, mCubeVertexBuffer[2], mSRBs[1] );
}

CINDER_APP( ResourceUpdatesApp, RendererGx( RendererGx::Options()/*.deviceType( gx::RENDER_DEVICE_TYPE_D3D12 )*/.prepareEngineFn( ResourceUpdatesApp::prepareEngine ) ) )
