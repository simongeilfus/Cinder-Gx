#include "cinder/app/App.h"
#include "cinder/app/RendererGx.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class _TBOX_PREFIX_App : public App {
  public:
	void setup() override;
	void update() override;
	void draw() override;
};

void _TBOX_PREFIX_App::setup()
{
}

void _TBOX_PREFIX_App::update()
{
}

void _TBOX_PREFIX_App::draw()
{
	gx::clear( ColorA( 0.350f, 0.350f, 0.350f, 1.0f ) );
}

CINDER_APP( _TBOX_PREFIX_App, RendererGx )