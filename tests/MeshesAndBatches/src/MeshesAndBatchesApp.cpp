#include "cinder/app/App.h"
#include "cinder/app/RendererGx.h"
#include "cinder/CinderDiligentImGui.h"
#include "cinder/graphics/Mesh.h"
#include "cinder/graphics/Batch.h"
#include "cinder/graphics/PipelineState.h"
#include "cinder/CameraUi.h"

#include "Utils.h"

using namespace ci;
using namespace ci::app;
using namespace std;

const string vertexShader = R"( #line 15
	cbuffer Constants {
		float4x4 g_WorldViewProj;
	};

	struct VSInput {
		float3 position   : ATTRIB0;
		float3 color : ATTRIB1;
	};

	struct PSInput { 
		float4 position : SV_POSITION; 
		float4 color : COLOR0; 
	};

	void main( in VSInput VSIn, out PSInput PSIn ) 
	{
		PSIn.position = mul( float4( VSIn.position, 1.0 ), g_WorldViewProj );
		PSIn.color  = float4( VSIn.color, 1.0 );
	}
)";

const string pixelShader = R"( #line 37
	struct PSInput { 
		float4 position : SV_POSITION; 
		float4 color : COLOR0; 
	};

	struct PSOutput	{
		float4 color : SV_TARGET;
	};

	void main( in PSInput PSIn, out PSOutput PSOut )
	{
		PSOut.color = PSIn.color; 
	}
)";

class MeshesAndBatchesApp : public App {
  public:
	void setup() override;
	void update() override;
	void draw() override;

	void initializeMeshSource();
	void initializeMeshBuffers();
	void initializeMeshIndexedVertices();
	void initializeMeshVertices();
	void initializeBatch();
	void initializeConstantsBuffer();

	gx::Mesh mMesh;
	gx::BufferRef mMeshConstants;
	gx::PipelineStateRef mMeshPipeline;
	gx::ShaderResourceBindingRef mMeshSRB;

	gx::Batch mBatch;

	CameraPersp mCamera;
	CameraUi mCameraUi;

	bool mDrawBatch;
};

void MeshesAndBatchesApp::setup()
{
	disableFrameRate();
	ImGui::DiligentInitialize();

	mCamera = CameraPersp( getWindowWidth(), getWindowHeight(), 45.0f, 0.1f, 1000.0f );
	mCamera.lookAt( vec3( 0.0f, 1.0f, 3.0f ), vec3( 0.0f ) );
	mCameraUi = CameraUi( &mCamera, getWindow() );

	initializeConstantsBuffer();
	initializeMeshSource();
	initializeBatch();

	mDrawBatch = true;
}

void MeshesAndBatchesApp::initializeConstantsBuffer()
{
	mMeshConstants = gx::createBuffer( gx::BufferDesc()
		.name( "Mesh constants CB" )
		.sizeInBytes( sizeof( mat4 ) )
		.usage( gx::USAGE_DYNAMIC )
		.bindFlags( gx::BIND_UNIFORM_BUFFER )
		.cpuAccessFlags( gx::CPU_ACCESS_WRITE )
	);
}

void MeshesAndBatchesApp::initializeMeshSource()
{
	mMesh = gx::Mesh( geom::TorusKnot().colors(), { geom::POSITION, geom::COLOR } );

	mMeshPipeline = gx::createGraphicsPipelineState( gx::GraphicsPipelineStateCreateInfo()
		.vertexShader( gx::createShader( gx::ShaderCreateInfo()
			.name( "Mesh vertex shader" )
			.shaderType( gx::SHADER_TYPE_VERTEX )
			.sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
			.source( vertexShader ) )
		)
		.pixelShader( gx::createShader( gx::ShaderCreateInfo()
			.name( "Mesh pixel shader" )
			.shaderType( gx::SHADER_TYPE_PIXEL )
			.sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
			.source( pixelShader ) )
		)
		.inputLayout( mMesh.getVertexLayoutElements() )
		.primitiveTopology( mMesh.getPrimitiveTopology() )
	);

	mMeshPipeline->GetStaticVariableByName( gx::SHADER_TYPE_VERTEX, "Constants" )->Set( mMeshConstants );
	mMeshPipeline->CreateShaderResourceBinding( &mMeshSRB, true );
}

void MeshesAndBatchesApp::initializeMeshBuffers()
{
	struct Vertex {
		vec3 position;
		vec4 color;
	};

	Vertex vertices[] = {
		{ vec3( -1,-1,-1 ), vec4( 1,0,0,1 ) },
		{ vec3( -1,+1,-1 ), vec4( 0,1,0,1 ) },
		{ vec3( +1,+1,-1 ), vec4( 0,0,1,1 ) },
		{ vec3( +1,-1,-1 ), vec4( 1,1,1,1 ) },

		{ vec3( -1,-1,+1 ), vec4( 1,1,0,1 ) },
		{ vec3( -1,+1,+1 ), vec4( 0,1,1,1 ) },
		{ vec3( +1,+1,+1 ), vec4( 1,0,1,1 ) },
		{ vec3( +1,-1,+1 ), vec4( 0.2f,0.2f,0.2f,1 ) },
	};

	gx::BufferRef vertexBuffer = gx::createBuffer( gx::BufferDesc()
		.name( "Cube vertex buffer" )
		.usage( gx::USAGE_IMMUTABLE )
		.bindFlags( gx::BIND_VERTEX_BUFFER )
		.sizeInBytes( sizeof( vertices ) ),
		&vertices, sizeof( vertices )
	);

	uint32_t indices[] = {
        2,0,1, 2,3,0,
        4,6,5, 4,7,6,
        0,7,4, 0,3,7,
        1,0,4, 1,4,5,
        1,5,2, 5,6,2,
        3,6,7, 3,2,6
    };

	gx::BufferRef indexBuffer = gx::createBuffer( gx::BufferDesc()
		.name( "Cube index buffer" )
		.usage( gx::USAGE_IMMUTABLE )
		.bindFlags( gx::BIND_INDEX_BUFFER )
		.sizeInBytes( sizeof( indices ) ),
		&indices, sizeof( indices )
	);

	vector<geom::AttribInfo> attribs = {
		{ geom::POSITION, 3, sizeof( Vertex ), 0 },
		{ geom::COLOR, 4, sizeof( Vertex ), offsetof( Vertex, color ) }
	};

	std::vector<std::pair<gx::Mesh::BufferInfo, gx::BufferRef>> vertexBuffers = {
		{ gx::Mesh::BufferInfo( attribs ), vertexBuffer }
	};

	mMesh = gx::Mesh( vertexBuffers, 36, indexBuffer, gx::VT_UINT32, geom::TRIANGLES );

	mMeshPipeline = gx::createGraphicsPipelineState( gx::GraphicsPipelineStateCreateInfo()
		.vertexShader( gx::createShader( gx::ShaderCreateInfo()
			.name( "Mesh vertex shader" )
			.shaderType( gx::SHADER_TYPE_VERTEX )
			.sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
			.source( vertexShader ) )
		)
		.pixelShader( gx::createShader( gx::ShaderCreateInfo()
			.name( "Mesh pixel shader" )
			.shaderType( gx::SHADER_TYPE_PIXEL )
			.sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
			.source( pixelShader ) )
		)
		.inputLayout( mMesh.getVertexLayoutElements() )
		.primitiveTopology( mMesh.getPrimitiveTopology() )
	);

	mMeshPipeline->GetStaticVariableByName( gx::SHADER_TYPE_VERTEX, "Constants" )->Set( mMeshConstants );
	mMeshPipeline->CreateShaderResourceBinding( &mMeshSRB, true );
}

void MeshesAndBatchesApp::initializeMeshIndexedVertices()
{
	struct Vertex {
		vec3 position;
		vec4 color;
	};

	Vertex vertices[] = {
		{ vec3( -1,-1,-1 ), vec4( 1,1,1,1 ) },
		{ vec3( -1,+1,-1 ), vec4( 1,1,1,1 ) },
		{ vec3( +1,+1,-1 ), vec4( 1,1,1,1 ) },
		{ vec3( +1,-1,-1 ), vec4( 1,1,1,1 ) },

		{ vec3( -1,-1,+1 ), vec4( 1,1,1,1 ) },
		{ vec3( -1,+1,+1 ), vec4( 1,1,1,1 ) },
		{ vec3( +1,+1,+1 ), vec4( 1,1,1,1 ) },
		{ vec3( +1,-1,+1 ), vec4( 1,1,1,1 ) },
	};

	uint16_t indices[] = {
		2,0,1, 2,3,0,
		4,6,5, 4,7,6,
		0,7,4, 0,3,7,
		1,0,4, 1,4,5,
		1,5,2, 5,6,2,
		3,6,7, 3,2,6
	};

	vector<geom::AttribInfo> attribs = {
		{ geom::POSITION, 3, sizeof( Vertex ), 0 },
		{ geom::COLOR, 4, sizeof( Vertex ), offsetof( Vertex, color ) }
	};

	mMesh = gx::Mesh( &vertices, sizeof( vertices ), attribs, 36, &indices );

	mMeshPipeline = gx::createGraphicsPipelineState( gx::GraphicsPipelineStateCreateInfo()
		.vertexShader( gx::createShader( gx::ShaderCreateInfo()
			.name( "Mesh vertex shader" )
			.shaderType( gx::SHADER_TYPE_VERTEX )
			.sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
			.source( vertexShader ) )
		)
		.pixelShader( gx::createShader( gx::ShaderCreateInfo()
			.name( "Mesh pixel shader" )
			.shaderType( gx::SHADER_TYPE_PIXEL )
			.sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
			.source( pixelShader ) )
		)
		.inputLayout( mMesh.getVertexLayoutElements() )
		.primitiveTopology( mMesh.getPrimitiveTopology() )
	);

	mMeshPipeline->GetStaticVariableByName( gx::SHADER_TYPE_VERTEX, "Constants" )->Set( mMeshConstants );
	mMeshPipeline->CreateShaderResourceBinding( &mMeshSRB, true );
}

void MeshesAndBatchesApp::initializeMeshVertices()
{
	struct Vertex {
		vec3 position;
		vec4 color;
	};

	Vertex vertices[] = {
		{ vec3( -0.5,+0,+0 ), vec4( 1,0,0,1 ) },
		{ vec3( +0.0,+1,+0 ), vec4( 0,1,0,1 ) },
		{ vec3( +0.5,+0,+0 ), vec4( 0,0,1,1 ) },
	};

	vector<geom::AttribInfo> attribs = {
		{ geom::POSITION, 3, sizeof( Vertex ), 0 },
		{ geom::COLOR, 4, sizeof( Vertex ), offsetof( Vertex, color ) }
	};

	mMesh = gx::Mesh( 3, &vertices, sizeof( vertices ), attribs );

	mMeshPipeline = gx::createGraphicsPipelineState( gx::GraphicsPipelineStateCreateInfo()
		.vertexShader( gx::createShader( gx::ShaderCreateInfo()
			.name( "Mesh vertex shader" )
			.shaderType( gx::SHADER_TYPE_VERTEX )
			.sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
			.source( vertexShader ) )
		)
		.pixelShader( gx::createShader( gx::ShaderCreateInfo()
			.name( "Mesh pixel shader" )
			.shaderType( gx::SHADER_TYPE_PIXEL )
			.sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
			.source( pixelShader ) )
		)
		.inputLayout( mMesh.getVertexLayoutElements() )
		.primitiveTopology( mMesh.getPrimitiveTopology() )
		.rasterizerStateDesc( gx::RasterizerStateDesc().cullMode( gx::CULL_MODE_NONE ) )
	);

	mMeshPipeline->GetStaticVariableByName( gx::SHADER_TYPE_VERTEX, "Constants" )->Set( mMeshConstants );
	mMeshPipeline->CreateShaderResourceBinding( &mMeshSRB, true );
}

void MeshesAndBatchesApp::initializeBatch()
{
	const string vertexShader = R"( #line 308
		layout(set = 0, binding = 0, std140) uniform ConstantBuffer
		{
			layout(row_major) mat4 transform;
		} vConstants;

		layout(location = 0) in vec3 aPosition;
		layout(location = 1) in vec3 aColor;
		layout(location = 0) out vec4 vColor;

		void main()
		{
			gl_Position = vConstants.transform * vec4( aPosition, 1.0 );
			vColor = vec4( aColor, 1.0 );
		}
	)";

	const string pixelShader = R"( #line 325
		layout(location = 0) in vec4 vColor;
		layout(location = 0) out vec4 oColor;

		void main()
		{
			oColor = vColor;
		}
	)";

	gx::Mesh mesh( geom::TorusKnot().colors(), { geom::POSITION, geom::COLOR } );
	mBatch = gx::Batch( mesh, gx::GraphicsPipelineStateCreateInfo()
		.vertexShader( gx::ShaderCreateInfo().source( vertexShader ) )
		.pixelShader( gx::ShaderCreateInfo().source( pixelShader ) ) 
	);
	mBatch.setStaticVariable( gx::SHADER_TYPE_VERTEX, "ConstantBuffer", mMeshConstants );
}

void MeshesAndBatchesApp::update()
{
	utils::updateWindowTitle();

	ImGui::Checkbox( "DrawBatch", &mDrawBatch );
	if( ImGui::Button( "initializeMeshSource" ) ) initializeMeshSource();
	if( ImGui::Button( "initializeMeshBuffers" ) ) initializeMeshBuffers();
	if( ImGui::Button( "initializeMeshIndexedVertices" ) ) initializeMeshIndexedVertices();
	if( ImGui::Button( "initializeMeshVertices" ) ) initializeMeshVertices();
}

void MeshesAndBatchesApp::draw()
{
	gx::clear( ColorA( 0.350f, 0.350f, 0.350f, 1.0f ) );

	{   // Map the buffer and write current world-view-projection matrix
		gx::MapHelper<mat4> constants( getImmediateContext(), mMeshConstants, gx::MAP_WRITE, gx::MAP_FLAG_DISCARD );
		// rebuild mCamera matrices as Cinder was not built with GLM_FORCE_LEFT_HANDED
		vec3 invertZ = vec3( 1, 1, -1 );
		mat4 view = glm::lookAt( mCamera.getEyePoint() * invertZ, ( mCamera.getEyePoint() + mCamera.getViewDirection() ) * invertZ, mCamera.getWorldUp() * invertZ );
		mat4 proj = glm::perspective( glm::radians( mCamera.getFov() ), getWindowAspectRatio(), 0.1f, 100.f );
		*constants = glm::transpose( proj * view );
	}

	if( mDrawBatch ) {
		mBatch.draw();
	}
	else {
		// Set the mesh pipeline and resources and draw it
		gx::setPipelineState( mMeshPipeline );
		gx::commitShaderResources( mMeshSRB, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
		mMesh.draw();
	}
}

CINDER_APP( MeshesAndBatchesApp, RendererGx )
