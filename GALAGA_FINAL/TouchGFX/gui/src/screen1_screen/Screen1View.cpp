#include <gui/screen1_screen/Screen1View.hpp>
extern uint16_t scores;
Screen1View::Screen1View()
{

}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

void Screen1View::handleTickEvent()
{
	tickCounter += 1;
	updatePoint(scores);

}
void Screen1View::updatePoint(int newValue)
{
    Unicode::snprintf(pointBuffer, POINT_SIZE, "%d", newValue);
    point.setWildcard(pointBuffer);
    point.invalidate();
}
