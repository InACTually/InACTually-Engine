
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "cinder/app/App.h"
#include "main/InACTually.hpp"

#include "cinder/Log.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace act;

class InACTuallyApp : public App {
  public:
		
	  void setup() override;

	  void cleanup() override;
	  void update() override;
	  void draw() override;

	  shared_ptr<input::MouseRawListener> mouseRawListener = make_shared<input::MouseRawListener>();
	  virtual void	mouseUp(ci::app::MouseEvent event)		override { mouseRawListener->mouseRawUp(event); }
	  virtual void	mouseDown(ci::app::MouseEvent event)	override;
	  virtual void	mouseWheel(ci::app::MouseEvent event)	override { mouseRawListener->mouseRawWheel(event); }
	  virtual void	mouseMove(ci::app::MouseEvent event)	override { mouseRawListener->mouseRawMove(event); }
	  virtual void	mouseDrag(ci::app::MouseEvent event)	override { mouseRawListener->mouseRawDrag(event); }

	  shared_ptr<input::TouchRawListener> touchRawListener = make_shared<input::TouchRawListener>();
	  void			touchesBegan(ci::app::TouchEvent event)	override { touchRawListener->touchesRawBegan(event); }
	  void			touchesMoved(ci::app::TouchEvent event)	override { touchRawListener->touchesRawMoved(event); }
	  void			touchesEnded(ci::app::TouchEvent event)	override { touchRawListener->touchesRawEnded(event); }

	  shared_ptr<input::KeyRawListener> keyRawListener = make_shared<input::KeyRawListener>();
	  virtual void	keyDown(ci::app::KeyEvent event)		override { keyRawListener->keyRawDown(event); }
	  virtual void	keyUp(ci::app::KeyEvent event)			override { keyRawListener->keyRawUp(event); }

	  void fileDrop(FileDropEvent event) override;
	  void resize() override;

	  shared_ptr<InACTually> inACTually;
};


#define RegisterModule(module) InACTually::useModule(std::make_shared<module>());

#include "modules/ProcessingModule.hpp"
#include "modules/RoomModule.hpp"
#include "modules/DisplayModule.hpp"
#include "modules/NetworkModule.hpp"

#include "processing/CameraProcNode.hpp"


void InACTuallyApp::setup()
{
	RegisterModule(mod::RoomModule);
	RegisterModule(mod::ProcessingModule);
	RegisterModule(mod::DisplayModule);
	RegisterModule(mod::NetworkModule);

	inACTually = make_shared<InACTually>(this);
	inACTually->setRawListener(mouseRawListener);
	inACTually->setRawListener(touchRawListener);
	inACTually->setRawListener(keyRawListener);

	inACTually->onInitialised([&]() {
		
	});
}   

void InACTuallyApp::cleanup()
{
	inACTually->cleanup();
}

void InACTuallyApp::update()
{
	inACTually->update();
}   

void InACTuallyApp::draw()
{
	inACTually->draw();
}

void InACTuallyApp::mouseDown(MouseEvent event)
{
	mouseRawListener->mouseRawDown(event);
}

void InACTuallyApp::fileDrop(FileDropEvent event)
{
	inACTually->fileDrop(event);
}

void InACTuallyApp::resize()
{
	inACTually->resize();
}

CINDER_APP(InACTuallyApp, RendererGl(RendererGl::Options().msaa(4)), [](App::Settings* settings) {
	settings->setMultiTouchEnabled(true);

	settings->setHighDensityDisplayEnabled(false);
	//settings->setPowerManagementEnabled(true);

	settings->setFrameRate(60);
})
