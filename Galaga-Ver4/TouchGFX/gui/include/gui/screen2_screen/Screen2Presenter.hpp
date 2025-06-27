#ifndef SCREEN2PRESENTER_HPP
#define SCREEN2PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Screen2View;

class Screen2Presenter : public touchgfx::Presenter, public ModelListener
{
public:
    Screen2Presenter(Screen2View& v);
    virtual ~Screen2Presenter() {}

    virtual void activate()   override;
    virtual void deactivate() override;

    // --- Thêm các phương thức sau ---
    void    updateImageX(int x)   { imageX = x; }
    void    updateImageY(int y)   { imageY = y; }
    int     getImageX() const     { return imageX; }
    int     getImageY() const     { return imageY; }

    void    saveLastScore(uint16_t s) { lastScore = s; }
    uint16_t getLastScore() const     { return lastScore; }

private:
    Screen2Presenter();  // không cho gọi mặc định

    Screen2View& view;

    // biến nội bộ để lưu trữ
    int        imageX    = 0;
    int        imageY    = 0;
    uint16_t   lastScore = 0;
};

#endif // SCREEN2PRESENTER_HPP
