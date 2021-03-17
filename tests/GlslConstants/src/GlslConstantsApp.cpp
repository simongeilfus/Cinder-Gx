#include "cinder/app/App.h"
#include "cinder/app/RendererGx.h"
#include "cinder/graphics/Mesh.h"
#include "cinder/graphics/Batch.h"
#include "cinder/graphics/PipelineState.h"
#include "cinder/CameraUi.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class GlslConstantsApp : public App {
public:
	GlslConstantsApp();

	void resize() override;
	void draw() override;

	gx::Batch mBatch;
	gx::BufferRef mConstants;

	CameraPersp mCamera;
	CameraUi mCameraUi;
};

GlslConstantsApp::GlslConstantsApp()
	: mCameraUi( &mCamera, getWindow() )
{
	mConstants = gx::createBuffer( gx::BufferDesc()
		.name( "Constant Buffer" )
		.sizeInBytes( sizeof( mat4 ) )
		.usage( gx::USAGE_DYNAMIC )
		.bindFlags( gx::BIND_UNIFORM_BUFFER )
		.cpuAccessFlags( gx::CPU_ACCESS_WRITE )
	);

	const string vertexShader = R"( #line 308
		uniform VertexConstants {
			mat4 transform;
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
	mBatch.setStaticVariable( gx::SHADER_TYPE_VERTEX, "VertexConstants", mConstants );
}

void GlslConstantsApp::resize()
{
	mCamera = CameraPersp( getWindowWidth(), getWindowHeight(), 45.0f, 0.1f, 1000.0f );
	mCamera.lookAt( vec3( 0.0f, 1.0f, 3.0f ), vec3( 0.0f ) );
}

namespace {
	mat4 getMatrices( const CameraPersp &camera )
	{
		const vec3 invertZ = vec3( 1, 1, -1 );
		mat4 view = glm::lookAt( camera.getEyePoint() * invertZ, ( camera.getEyePoint() + camera.getViewDirection() ) * invertZ, camera.getWorldUp() * invertZ );
		mat4 proj = glm::perspective( glm::radians( camera.getFov() ), getWindowAspectRatio(), 0.1f, 100.f );
		return proj * view;
	}
}

void GlslConstantsApp::draw()
{
	gx::clear( ColorA( 0.350f, 0.350f, 0.350f, 1.0f ) );

	{   // Map the buffer and write the camera matrices
		gx::MapHelper<mat4> constants( getImmediateContext(), mConstants, gx::MAP_WRITE, gx::MAP_FLAG_DISCARD );
		*constants = getMatrices( mCamera );
	}

	mBatch.draw();
}

CINDER_APP( GlslConstantsApp, RendererGx )
