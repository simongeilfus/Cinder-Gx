#include "cinder/app/App.h"
#include "cinder/app/RendererGx.h"

#include "cinder/graphics/Buffer.h"
#include "cinder/graphics/PipelineState.h"
#include "cinder/graphics/DrawContext.h"
#include "cinder/graphics/Texture.h"
#include "cinder/CinderDiligentImGui.h"
#include "cinder/Rand.h"

#include "Utils.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class SolidRectApp : public App {
  public:
	void setup() override;
	void update() override;
	void draw() override;
	static void prepareEngine( gx::RENDER_DEVICE_TYPE, gx::EngineCreateInfo*, gx::SwapChainDesc* );

	gx::TextureViewRef mTexture[4];
	gx::DrawContext mDrawContext;
	gx::DrawContext mSecondaryDrawContext;
};

void SolidRectApp::prepareEngine( gx::RENDER_DEVICE_TYPE deviceType, gx::EngineCreateInfo* engineCreateInfo, gx::SwapChainDesc* swapChainDesc )
{
#if VULKAN_SUPPORTED
	if( deviceType == gx::RENDER_DEVICE_TYPE_VULKAN ) {
		gx::EngineVkCreateInfo* vkAttrs = static_cast<gx::EngineVkCreateInfo*>( engineCreateInfo );
	}
#endif
}

void SolidRectApp::setup()
{
	disableFrameRate();
	ImGui::DiligentInitialize();

	mTexture[0] = gx::createTexture( loadImage( loadAsset( "DGLogo0.png" ) ) )->GetDefaultView( gx::TEXTURE_VIEW_SHADER_RESOURCE );
	mTexture[1] = gx::createTexture( loadImage( loadAsset( "DGLogo1.png" ) ) )->GetDefaultView( gx::TEXTURE_VIEW_SHADER_RESOURCE );
	mTexture[2] = gx::createTexture( loadImage( loadAsset( "DGLogo2.png" ) ) )->GetDefaultView( gx::TEXTURE_VIEW_SHADER_RESOURCE );
	mTexture[3] = gx::createTexture( loadImage( loadAsset( "DGLogo3.png" ) ) )->GetDefaultView( gx::TEXTURE_VIEW_SHADER_RESOURCE );
}

void SolidRectApp::update()
{
	utils::updateWindowTitle();
}

void SolidRectApp::draw()
{
	static vec4 rect0( 100, 100, 200, 200 );
	static vec4 rect1( 150, 150, 250, 250 );
	static ColorA color = ColorA::white();
	static vec2 viewportUpperLeft( 0 );
	static vec2 viewportSize( getWindowSize() );
	static float z = 0.0f;
	static bool depth = false;
	static int blending = 1;
	const char* blendingName[4] = { "Disabled", "Alpha", "Premult", "Additive" };

	ImGui::DragFloat4( "rect0", &rect0[0] );
	ImGui::DragFloat4( "rect1", &rect1[0] );
	ImGui::DragFloat2( "viewportUpperLeft", &viewportUpperLeft );
	ImGui::DragFloat2( "viewportSize", &viewportSize );
	ImGui::ColorEdit4( "color", &color[0] );
	ImGui::DragFloat( "z", &z );
	ImGui::Checkbox( "depth", &depth );
	if( ImGui::BeginCombo( "Blending", blendingName[blending] ) ) {
		for( size_t i = 0; i < 4; ++i ) {
			if( ImGui::Selectable( blendingName[i], i == blending ) ) {
				blending = i;
			}
		}
		ImGui::EndCombo();
	}


	gx::clear( ColorA( 0.350f, 0.350f, 0.350f, 1.0f ) );

#if 0
	static gx::CommandListRef commandList[1];
	if( ! commandList[0] ) {
		gx::DrawContext ctx;
		ctx.setMatricesWindow( getWindowSize() );
		ctx.viewport( getWindowSize() );
		ctx.enableDepthRead( false );
		ctx.enableAlphaBlending();

		ctx.color( ColorA::gray( 1.0f, 0.4f ) );
		ctx.drawSolidRect( Rectf( rect0.x, rect0.y, rect0.z, rect0.w ) );
		ctx.bindTexture( mTexture );
		ctx.drawSolidRect( Rectf( rect1.x, rect1.y, rect1.z, rect1.w ) );
		ctx.unbindTexture();

		commandList[0] = ctx.bake( getRenderDevice(), getImmediateContext() );
	}

	gx::executeCommandLists( 1, &commandList[0] );
#elif 1
#if 0
	// ! warning smart code ahead
	{ // start a thread to render two rectangles
		std::thread thread = std::thread( [&]() {
			gx::DrawContext& ctx = mSecondaryDrawContext;
			ctx.enableDepthRead( false );
			ctx.enableDepthWrite( false );
			ctx.enableAlphaBlending();
			ctx.setMatricesWindow( app::getWindowSize() );
			ctx.viewport( app::getWindowSize() );

			ctx.color( ColorA::white() );
			ctx.draw( geom::RoundedRect( Rectf( 100, 100, 200, 200 ), 5.0f ) );
			ctx.color( ColorA( 1.0f, 0.0f, 0.0f, 0.4f ) );
			ctx.drawSolidRect( Rectf( 150, 150, 250, 250 ) );
			} );

		thread.join();
	}

	// submit secondary context
	mSecondaryDrawContext.submit();
#endif

	static bool redraw = false;
	ImGui::Checkbox( "redraw", &redraw );

	gx::DrawContext& ctx = mDrawContext;
	if( ctx.empty() || redraw ) {
		ctx.enableDepthRead( false );
		ctx.enableDepthWrite( false );
		ctx.enableAlphaBlending();
		ctx.setMatricesWindow( getWindowSize() );
		ctx.viewport( getWindowSize() );


		ctx.color( ColorA::white() );
		ctx.bindTexture( mTexture[1] );
		ctx.drawSolidRect( Rectf( 0, 0, 100, 100 ) );
		ctx.bindTexture( mTexture[2] );
		ctx.drawSolidRect( Rectf( 50, 50, 150, 150 ) );
		ctx.bindTexture( mTexture[3] );
		ctx.drawSolidRect( Rectf( 100, 100, 200, 200 ) );
		ctx.unbindTexture();

		ctx.color( ColorA::white() );
		ctx.draw( geom::RoundedRect( Rectf( 300, 50, 400, 150 ), 5.0f ) );
		ctx.color( ColorA( 1.0f, 0.0f, 0.0f, 0.4f ) );
		ctx.drawSolidRect( Rectf( 150, 150, 250, 250 ) );

		ctx.enableDepthRead( true );
		ctx.enableDepthWrite( true );
		ctx.setMatricesWindowPersp( getWindowSize() );
		ctx.pushModelMatrix();
		ctx.translate( getWindowCenter() );
		ctx.rotate( getElapsedSeconds(), 0.3f, 0.8f, 0.0f );
		ctx.scale( 50.0f );
		ctx.color( ColorA::white() );
		ctx.bindTexture( mTexture[0] );
		ctx.draw( geom::Cube() );
		ctx.unbindTexture();
		ctx.popModelMatrix();
	}


	int pipelines = mDrawContext.mPipelines.size();
	int drawCalls = mDrawContext.mCommands.size();

	ImGui::Value( "Pipelines", pipelines );
	ImGui::Value( "drawCalls", drawCalls );
	ImGui::Value( "ConstantCount", mDrawContext.mConstantCount );
	ImGui::Checkbox( "GeomBuffersImmutable", &mDrawContext.mGeomBuffersImmutable );
	ctx.submit( redraw );



#elif 1
	gx::DrawContext& ctx = mDrawContext;
	ctx.enableDepthRead( depth );
	ctx.enableDepthWrite( depth );
	switch( blending ) {
		case 0: ctx.disableBlending(); break;
		case 1: ctx.enableAlphaBlending(); break;
		case 2: ctx.enableAlphaBlendingPremult(); break;
		case 3: ctx.enableAdditiveBlending(); break;
	}
	ctx.setMatricesWindow( getWindowSize() );
	ctx.viewport( getWindowSize() );

	ctx.color( color );
	ctx.drawSolidRect( Rectf( rect0.x, rect0.y, rect0.z, rect0.w ) );

	ctx.bindTexture( mTexture );
	//ctx.translate( vec2( 50 ) );
	ctx.drawSolidRect( Rectf( rect1.x, rect1.y, rect1.z, rect1.w ) );
	ctx.unbindTexture();
	/*ctx.color( ColorA( 1, 0, 0, 0.05f ) );
	ctx.translate( vec3( 0, 0, z ) );
	ctx.drawSolidRect( Rectf( rectMin, rectMax ) + vec2( 30.0f ) );*/

	//Rand rnd;
	//for( int i = 0; i < 100; ++i ) {
	//	ctx.color( Color::gray( rnd.nextFloat() ) );
	//	vec2 min( rnd.nextFloat( 0.0f, getWindowWidth() ), rnd.nextFloat( 0.0f, getWindowHeight() ) );
	//	ctx.drawSolidRect( Rectf( min, min + vec2( rnd.nextFloat( 10.0f, 60.0f ) ) ) );
	//}
	int pipelines = mDrawContext.mPipelines.size();
	int drawCalls = mDrawContext.mCommands.size();

	ImGui::Value( "Pipelines", pipelines );
	ImGui::Value( "drawCalls", drawCalls );
	ImGui::Value( "ConstantCount", mDrawContext.mConstantCount );

	ctx.debugSubmit( "DrawContext" );
	ctx.submit();
#endif

}

CINDER_APP( SolidRectApp, RendererGx( RendererGx::Options()/*.deviceType( gx::RENDER_DEVICE_TYPE_D3D12 )*/.prepareEngineFn( SolidRectApp::prepareEngine ) ) )
