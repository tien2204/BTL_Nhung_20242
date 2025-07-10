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
#define BULLET_SPEED    3
#define FIRE_COOLDOWN_TICKS 15

struct Bullet {
    int x, y;
    bool active;
};

struct Enemy {
    int x, y;
    bool alive;
    bool chasePlayer; // Thêm cờ phân biệt enemy random
};

static const int MAX_ACTIVE_BULLETS = 50;
static const int MAX_TOTAL_SHOTS   = 1000000000;

class Screen2View : public Screen2ViewBase
{
public:
    Screen2View();
    virtual ~Screen2View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();
    virtual void ExitFromScreen2();

    virtual void updatePoint(int newValue);
    virtual void updateHearts();
    virtual void resetGame();
    bool checkCollision(const touchgfx::Image& img1, const touchgfx::Image& img2);
    void updateJoyX(uint16_t value);
    void buzz(uint16_t duration_ms);
    static inline int clamp(int value, int minVal, int maxVal) {
        if (value < minVal) return minVal;
        if (value > maxVal) return maxVal;
        return value;
    }

    int getScore() const;

    void spawnRandomEnemies(int n);
    void moveEnemiesTowardSpaceship();

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
    int totalShotsFired;

    static const int POINT_SIZE = 8;
    Unicode::UnicodeChar txtBuffer[POINT_SIZE];

    Enemy enemies[MAX_ENEMIES];
    Image enemyImages[MAX_ENEMIES];

    int spaceshipX, spaceshipY;

    int fireCooldown = 0;

    int flickerCount = 0;
    bool flickering = false;
    int lives = 3;
    int scores = 0;

    int totalEnemiesDefeated = 0;
    bool playerHasWon = false;
};

#endif // SCREEN2VIEW_HPP
