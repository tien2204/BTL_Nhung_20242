#include <gui/screen2_screen/Screen2View.hpp>
#include <images/BitmapDatabase.hpp>
#include "cmsis_os.h"
#include "main.h"
#include <cmath>

extern RNG_HandleTypeDef hrng;

Screen2View::Screen2View()
    : localImageX(0)
    , localImageY(0)
    , tickCount(0)
    , totalShotsFired(0)
    , fireCooldown(0)
    , flickerCount(0)
    , flickering(false)
    , lives(3)
    , oldJoyX(0)
    , totalEnemiesDefeated(0)
    , playerHasWon(false)
{
    for (int i = 0; i < MAX_ACTIVE_BULLETS; ++i) {
        bullets[i].active       = false;
        bulletImages[i].setBitmap(Bitmap(BITMAP_MISSILE_ID));
        bulletImages[i].setVisible(false);
        add(bulletImages[i]);
    }

    for (int i = 0; i < MAX_ENEMIES; ++i) {
        enemyImages[i].setBitmap(Bitmap(BITMAP_ENEMY_ID));
        enemyImages[i].setVisible(false);
        add(enemyImages[i]);
    }
}

void Screen2View::updateJoyX(uint16_t value)
{
    Unicode::snprintf(joyXBuffer, JOY_BUF_SIZE, "%u&%u", JoystickX, JoystickY);
    txtJoystickX.invalidate();
}

void Screen2View::buzz(uint16_t duration_ms)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_Delay(duration_ms);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
}

void Screen2View::setupScreen()
{
    Screen2ViewBase::setupScreen();

    txtJoystickX.setWildcard(joyXBuffer);
    updateJoyX(JoystickX);

    for (int i = 0; i < MAX_ACTIVE_BULLETS; i++) {
        bullets[i].active       = false;
        bulletImages[i].setVisible(false);
    }

    const int marginX = 10;
    const int marginY = 20;
    const int spacingX = (HAL::DISPLAY_WIDTH - 2*marginX) / ENEMY_COLS;
    const int spacingY = 20;
    for (int r = 0; r < ENEMY_ROWS; ++r) {
        for (int c = 0; c < ENEMY_COLS; ++c) {
            int idx = r*ENEMY_COLS + c;
            enemies[idx].alive = true;
            enemies[idx].chasePlayer = false; // 25 con đầu ĐỨNG YÊN
            int x = marginX + c * spacingX;
            int y = marginY + r * spacingY;
            enemies[idx].x = x;
            enemies[idx].y = y;
            enemyImages[idx].moveTo(x, y);
            enemyImages[idx].setVisible(true);
        }
    }

    spaceshipX = HAL::DISPLAY_WIDTH/2  - spaceship.getWidth()/2;
    spaceshipY = HAL::DISPLAY_HEIGHT - spaceship.getHeight() - 10;
    spaceship.moveTo(spaceshipX, spaceshipY);

    tickCount = 0;
    lives = 3;
    scores = 0;
    totalEnemiesDefeated = 0;
    playerHasWon = false;
    updateHearts();
    updatePoint(scores);

    enemy.setVisible(false);
    enemy.invalidate();

    missile.setVisible(false);
    missile.invalidate();
}

void Screen2View::tearDownScreen()
{
    Screen2ViewBase::tearDownScreen();
}

void Screen2View::spawnRandomEnemies(int n)
{
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < MAX_ENEMIES; ++j)
        {
            if (!enemies[j].alive) {
                uint32_t randNumX, randNumY;
                HAL_RNG_GenerateRandomNumber(&hrng, &randNumX);
                HAL_RNG_GenerateRandomNumber(&hrng, &randNumY);
                int x = randNumX % (HAL::DISPLAY_WIDTH - enemyImages[j].getWidth());
                int y = randNumY % ((HAL::DISPLAY_HEIGHT / 2) - enemyImages[j].getHeight());

                enemies[j].alive = true;
                enemies[j].chasePlayer = true; // Enemy random thì bay theo tàu
                enemies[j].x = x;
                enemies[j].y = y;
                enemyImages[j].moveTo(x, y);
                enemyImages[j].setVisible(true);
                enemyImages[j].invalidate();

                break;
            }
        }
    }
}

void Screen2View::moveEnemiesTowardSpaceship()
{
    const int ENEMY_SPEED = 2;
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (enemies[i].alive && enemies[i].chasePlayer) { // Chỉ enemy random di chuyển
            int dx = spaceshipX + spaceship.getWidth()/2 - (enemies[i].x + enemyImages[i].getWidth()/2);
            int dy = spaceshipY + spaceship.getHeight()/2 - (enemies[i].y + enemyImages[i].getHeight()/2);
            float length = sqrtf(dx*dx + dy*dy);

            if (length > 1.0f) {
                float vx = ENEMY_SPEED * dx / length;
                float vy = ENEMY_SPEED * dy / length;
                enemies[i].x += (int)vx;
                enemies[i].y += (int)vy;
                enemyImages[i].moveTo(enemies[i].x, enemies[i].y);
            }
        }
    }
}

void Screen2View::handleTickEvent() {
    tickCount++;

    if (JoystickX != oldJoyX) {
        updateJoyX(JoystickX);
        oldJoyX = JoystickX;
    }

    const int DEADZONE = 300;
    const int SPEED = 3;
    const int SHIP_Y_MAX = HAL::DISPLAY_HEIGHT - spaceship.getHeight() - 10;

    int dx = 0, dy = 0;

    if (JoystickX < 2048 - DEADZONE) dx = -SPEED;
    else if (JoystickX > 2048 + DEADZONE) dx = SPEED;

    if (JoystickY < 2048 - DEADZONE) dy = -SPEED;
    else if (JoystickY > 2048 + DEADZONE) dy = SPEED;

    spaceshipX = clamp(spaceshipX + dx, 0, HAL::DISPLAY_WIDTH - spaceship.getWidth());
    spaceshipY = clamp(spaceshipY + dy, 0, SHIP_Y_MAX);
    spaceship.moveTo(spaceshipX, spaceshipY);

    moveEnemiesTowardSpaceship();

    bool fireRequest = shootFlag;
    shootFlag = false;

    if (fireRequest && fireCooldown == 0 && totalShotsFired < MAX_TOTAL_SHOTS) {
        for (int i = 0; i < MAX_ACTIVE_BULLETS; ++i) {
            if (!bullets[i].active) {
                bullets[i].active = true;
                totalShotsFired++;
                bullets[i].x = spaceship.getX() + spaceship.getWidth() / 2 - bulletImages[i].getWidth() / 2;
                bullets[i].y = spaceship.getY();
                bulletImages[i].moveTo(bullets[i].x, bullets[i].y);
                bulletImages[i].setVisible(true);
                buzz(10);
                break;
            }
        }
        fireCooldown = FIRE_COOLDOWN_TICKS;
    }
    if (fireCooldown > 0) --fireCooldown;

    for (int i = 0; i < MAX_ACTIVE_BULLETS; ++i) {
        if (bullets[i].active) {
            bullets[i].y -= BULLET_SPEED;
            if (bullets[i].y < -bulletImages[i].getHeight()) {
                bullets[i].active = false;
                bulletImages[i].setVisible(false);
                continue;
            }

            bulletImages[i].moveTo(bullets[i].x, bullets[i].y);

            for (int j = 0; j < MAX_ENEMIES; ++j) {
                if (enemies[j].alive && checkCollision(bulletImages[i], enemyImages[j])) {
                    bullets[i].active = false;
                    bulletImages[i].setVisible(false);
                    enemies[j].alive = false;
                    enemyImages[j].setVisible(false);
                    enemyImages[j].invalidate();
                    buzz(50);
                    scores += 10;
                    updatePoint(scores);

                    totalEnemiesDefeated++;
                    if (totalEnemiesDefeated >= 50 && !playerHasWon) {
                        playerHasWon = true;
                        // Thông báo thắng (tùy bạn bổ sung giao diện)
                    }
                    break;
                }
            }
        }
    }

    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (enemies[i].alive && !flickering && checkCollision(spaceship, enemyImages[i])) {
            lives--;
            enemyImages[i].setVisible(false);
            enemyImages[i].invalidate();
            enemies[i].alive = false;
            updateHearts();
            buzz(250);
            flickering = true;
            flickerCount = 0;
            break;
        }
    }

    if (flickering) {
        flickerCount++;
        if ((flickerCount & 1) == 0) {
            spaceship.setVisible(!spaceship.isVisible());
        }
        if (flickerCount >= 60) {
            flickering = false;
            spaceship.setVisible(true);
        }
    }

    bool allEnemiesDead = true;
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (enemies[i].alive) {
            allEnemiesDead = false;
            break;
        }
    }
    if (allEnemiesDead && totalEnemiesDefeated < 50) {
        uint32_t randNum;
        HAL_RNG_GenerateRandomNumber(&hrng, &randNum);
        int newEnemies = 1 + (randNum % 5);
        if (totalEnemiesDefeated + newEnemies > 50) {
            newEnemies = 50 - totalEnemiesDefeated;
        }
        spawnRandomEnemies(newEnemies);
    }
}

void Screen2View::updatePoint(int newValue)
{
    Unicode::snprintf(txtBuffer, POINT_SIZE, "%d", newValue);
    point.setWildcard(txtBuffer);
    point.invalidate();
}

void Screen2View::updateHearts()
{
    switch (lives) {
        case 0:
            resetGame();
            application().gotoScreen1ScreenNoTransition();
            break;
        case 1:
            heart1.setVisible(true);
            heart2.setVisible(false);
            heart3.setVisible(false);
            break;
        case 2:
            heart1.setVisible(true);
            heart2.setVisible(true);
            heart3.setVisible(false);
            break;
        case 3:
            heart1.setVisible(true);
            heart2.setVisible(true);
            heart3.setVisible(true);
            break;
        default:
            break;
    }

    heart1.invalidate();
    heart2.invalidate();
    heart3.invalidate();
}

void Screen2View::resetGame()
{
    tickCount = 0;
    flickerCount = 0;
    flickering = false;
    lives = 3;
    scores = 0;
    totalEnemiesDefeated = 0;
    playerHasWon = false;
    totalShotsFired = 0;
    fireCooldown = 0;
    for (int i = 0; i < MAX_ACTIVE_BULLETS; ++i) {
        bullets[i].active = false;
        bulletImages[i].setVisible(false);
    }
    spaceship.moveTo(103, 206);

    updatePoint(0);
    updateHearts();
}

bool Screen2View::checkCollision(const Image& img1, const Image& img2)
{
    return (img1.getX() < img2.getX() + img2.getWidth() &&
            img1.getX() + img1.getWidth() > img2.getX() &&
            img1.getY() < img2.getY() + img2.getHeight() &&
            img1.getY() + img1.getHeight() > img2.getY());
}

int Screen2View::getScore() const
{
    return scores;
}

void Screen2View::ExitFromScreen2()
{
    presenter->saveLastScore(scores);
}
