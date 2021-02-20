#pragma once

#include "cinder/app/App.h"
#include "cinder/Timer.h"

#include <sstream>

namespace utils {

void updateWindowTitle()
{
    static std::string title = ci::app::App::get()->getWindow()->getTitle();
    static ci::Timer timer;
    static double filteredFrameTime = 0.0;
    timer.stop();
    double filterScale = 0.2;
    double elapsedTime = timer.getSeconds();
    filteredFrameTime = filteredFrameTime * ( 1.0 - filterScale ) + filterScale * elapsedTime;
    std::stringstream fpsCounterSS;
    fpsCounterSS << title << " - " << std::fixed << std::setprecision( 1 ) << filteredFrameTime * 1000;
    fpsCounterSS << " ms (" << 1.0 / filteredFrameTime << " fps)";
    timer.start();
    ci::app::getWindow()->setTitle( fpsCounterSS.str() );
}

} // namespace utils