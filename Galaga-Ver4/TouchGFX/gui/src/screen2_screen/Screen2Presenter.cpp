#include <gui/screen2_screen/Screen2View.hpp>
#include <gui/screen2_screen/Screen2Presenter.hpp>

Screen2Presenter::Screen2Presenter(Screen2View& v)
    : Presenter(),
	  view(v),
	  imageX(0),
	  imageY(0),
	  lastScore(0)
{

}

void Screen2Presenter::activate()
{
	view.setupScreen();
}

void Screen2Presenter::deactivate()
{

}

