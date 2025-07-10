#include <gui/screen1_screen/Screen1View.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>

Screen1Presenter::Screen1Presenter(Screen1View& v)
    : view(v)
{

}

void Screen1Presenter::activate()
{
    int best = model->getBestScore();   // lấy kỷ lục đã lưu
    view.updatePoint(best);              // đẩy ra view
}

void Screen1Presenter::deactivate()
{

}
