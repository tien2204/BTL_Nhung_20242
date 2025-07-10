#ifndef SCREEN1VIEW_HPP
#define SCREEN1VIEW_HPP

#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>

#include <touchgfx/widgets/TextAreaWithWildcard.hpp>
#include <touchgfx/Unicode.hpp>

class Screen1View : public Screen1ViewBase
{
public:
    Screen1View();
    virtual ~Screen1View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();
    virtual void updatePoint(int newValue);
protected:
    long tickCounter = 0;

    static const int POINT_SIZE = 8;                      // số ký tự tối đa bạn cần hiển thị
    Unicode::UnicodeChar pointBuffer[POINT_SIZE];
};

#endif // SCREEN1VIEW_HPP
