#ifndef SCREEN2VIEW_HPP
#define SCREEN2VIEW_HPP

#include <gui_generated/screen2_screen/Screen2ViewBase.hpp>
#include <gui/screen2_screen/Screen2Presenter.hpp>

#include <touchgfx/widgets/TextAreaWithWildcard.hpp>
#include <touchgfx/Unicode.hpp>

// từ 5 -> 25 enemy (5x5)
#define ENEMY_ROWS      5
#define ENEMY_COLS      5
#define MAX_ENEMIES     (ENEMY_ROWS * ENEMY_COLS)

// từ 10 -> 1000 missile
#define BULLET_SPEED    4
#define FIRE_COOLDOWN_TICKS 10

struct Bullet {
    int x, y;
    bool active;
};

struct Enemy {
    int x, y;
    bool alive;
};

static const int MAX_ACTIVE_BULLETS = 50;
static const int MAX_TOTAL_SHOTS   = 1000;


class Screen2View : public Screen2ViewBase
{
public:
    Screen2View();
    virtual ~Screen2View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();
    virtual void ExitFromScreen2() override;

    virtual void updatePoint(int newValue);
    virtual void updateHearts();
    virtual void resetGame();
    bool checkCollision(const touchgfx::Image& img1, const touchgfx::Image& img2);
    void updateJoyX(uint16_t value);

    static inline int mapClamped(int v, int screenMax)
        {
    		if (v < 20) v = 0;
    	    else if (v > 4075) v = 4095;
    	    return (v * screenMax) / 4095;              // scale sang kích thước màn
        }


protected:
    uint16_t oldJoyX;
    uint16_t oldJoyY;
    static const uint16_t JOY_BUF_SIZE = 12;
    Unicode::UnicodeChar joyXBuffer[JOY_BUF_SIZE];

    int16_t localImageX;
    int16_t localImageY;
    uint32_t tickCount;
    Bullet   bullets[MAX_ACTIVE_BULLETS];
    Image    bulletImages[MAX_ACTIVE_BULLETS];
//    bool     bulletsActive[MAX_ACTIVE_BULLETS];
    int      totalShotsFired;

    // kích thước mang chứa chữ số điểm (tối đa 6 chữ số + terminate)
    static const int POINT_SIZE = 8;
    Unicode::UnicodeChar txtBuffer[POINT_SIZE];
    // trùng với tên widget bạn kéo từ Designer
    touchgfx::TextAreaWithOneWildcard point;

    Enemy enemies[MAX_ENEMIES];
    Image enemyImages[MAX_ENEMIES]; // Kéo từ Designer

    int spaceshipX, spaceshipY;

    int fireCooldown = 0;

    int flickerCount = 0;
    bool flickering = false;
    int lives = 3;

};

#endif // SCREEN2VIEW_HPP
