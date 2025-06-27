#include <gui/screen2_screen/Screen2View.hpp>
#include <images/BitmapDatabase.hpp>
#include "cmsis_os.h"
#include "main.h"

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
{
    for (int i = 0; i < MAX_ACTIVE_BULLETS; ++i) {
        bullets[i].active       = false;
//        bulletsActive[i]        = false;
        bulletImages[i].setBitmap(Bitmap(BITMAP_MISSILE_ID));
        bulletImages[i].setVisible(false);
        add(bulletImages[i]);
    }

    for (int i = 0; i < MAX_ENEMIES; ++i) {
        enemyImages[i].setBitmap(Bitmap(BITMAP_ENEMY_ID));  // bitmap bạn đã import
        enemyImages[i].setVisible(false);
        add(enemyImages[i]);
    }
}

void Screen2View::updateJoyX(uint16_t value)
{
	Unicode::snprintf(joyXBuffer, JOY_BUF_SIZE, "%4u,%4u", JoystickX, JoystickY);
    txtJoystickX.invalidate();                 // vẽ lại
}

void Screen2View::setupScreen()
{
	Screen2ViewBase::setupScreen();

	txtJoystickX.setWildcard(joyXBuffer);      // trỏ wildcard vào buffer
	updateJoyX(JoystickX);

//    localImageX = presenter->getImageX();
//    localImageY = presenter->getImageY();
//    spaceship.moveTo(localImageX, localImageY);
    // --- reset bullets ---
    for (int i = 0; i < MAX_ACTIVE_BULLETS; i++) {
    	bullets[i].active       = false;
//    	bulletsActive[i]        = false;
    	bulletImages[i].setVisible(false);
    }

    // --- reset và bố trí 5x5 enemies ---
    const int marginX = 10;
    const int marginY = 20;
    const int spacingX = (HAL::DISPLAY_WIDTH - 2*marginX) / ENEMY_COLS;
    const int spacingY = 20;
    for (int r = 0; r < ENEMY_ROWS; ++r) {
        for (int c = 0; c < ENEMY_COLS; ++c) {
            int idx = r*ENEMY_COLS + c;
            enemies[idx].alive = true;
            int x = marginX + c * spacingX;
            int y = marginY + r * spacingY;
            enemies[idx].x = x;
            enemies[idx].y = y;
            enemyImages[idx].moveTo(x, y);
            enemyImages[idx].setVisible(true);
        }
    }

    // khởi vị trí và trạng thái ship
    spaceshipX = HAL::DISPLAY_WIDTH/2  - spaceship.getWidth()/2;
    spaceshipY = HAL::DISPLAY_HEIGHT - spaceship.getHeight() - 10;
    spaceship.moveTo(spaceshipX, spaceshipY);

    // reset điểm–mạng
    tickCount = 0;
    lives = 3;
    scores = 0;
    point.setWildcard(txtBuffer);   // thêm dòng này
    updatePoint(0);
    updateHearts();

}

void Screen2View::tearDownScreen()
{
    Screen2ViewBase::tearDownScreen();

}

void Screen2View::handleTickEvent() {
    tickCount++;


    if (JoystickX != oldJoyX)
       {
           updateJoyX(JoystickX);
           oldJoyX = JoystickX;
       }

    int rawX = JoystickX;
    int rawY = 4095 - JoystickY;
    const int SHIP_Y_MAX = HAL::DISPLAY_HEIGHT - spaceship.getHeight() - 10;

    // 1) di chuyển ship
    spaceshipX = mapClamped(rawX, HAL::DISPLAY_WIDTH  - spaceship.getWidth());
    spaceshipY = mapClamped(rawY, SHIP_Y_MAX);
    spaceship.moveTo(spaceshipX, spaceshipY);

    bool fireRequest = shootFlag;
    shootFlag = false;

    // 2) bắn đạn nếu có tín hiệu
    if (fireRequest && fireCooldown == 0 && totalShotsFired < MAX_TOTAL_SHOTS) {
        for (int i = 0; i < MAX_ACTIVE_BULLETS; ++i) {
            if (!bullets[i].active ) {
//                bulletsActive[i]     = true;
                bullets[i].active     = true;
                totalShotsFired++;
                bullets[i].x = spaceship.getX() + spaceship.getWidth()/2;
                bullets[i].y = spaceship.getY();
                bulletImages[i].moveTo(bullets[i].x, bullets[i].y);
                bulletImages[i].setVisible(true);
                break;
            }
        }
        fireCooldown = FIRE_COOLDOWN_TICKS;

    }
    if (fireCooldown > 0) --fireCooldown;

    // 3) cập nhật toàn bộ bullets
    for (int i = 0; i < MAX_ACTIVE_BULLETS; ++i) {
        if (bullets[i].active) {
        	bullets[i].y -= BULLET_SPEED;
        	if (bullets[i].y < -bulletImages[i].getHeight())
        	    {
        	        bullets[i].active = false;
        	        bulletImages[i].setVisible(false);
        	        continue;
        	    }

            bulletImages[i].moveTo(bullets[i].x, bullets[i].y);

            // va chạm với mọi enemy
            for (int j = 0; j < MAX_ENEMIES; ++j) {
                if (enemies[j].alive && checkCollision(bulletImages[i], enemyImages[j])) {
                    bullets[i].active = false;
                    bulletImages[i].setVisible(false);
                    enemies[j].alive = false;
                    enemyImages[j].setVisible(false);

                    scores += 10;          // cộng 10 điểm
                    updatePoint(scores);
                    break;
                }
            }
        }
    }



    // 4) (giữ nguyên phần va chạm ship–enemy, nhấp nháy, giảm mạng…)…
    for (int i = 0; i < MAX_ENEMIES; ++i) {
            if (enemies[i].alive && !flickering && checkCollision(spaceship, enemyImages[i])) {
            	lives--;
            	enemyImages[i].setVisible(false);
            	enemies[i].alive = false;
            	updateHearts();          // cập nhật hình trái tim và xử lý reset nếu lives == 0
                flickering = true;       // bật chế độ nhấp nháy
            	flickerCount = 0;
            	break;
            }
    }

        // ✨ Nhấp nháy nếu bị va chạm
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

//    // 5) vẽ lại tất cả
//    spaceship.invalidate();
//    for (int i = 0; i < MAX_ACTIVE_BULLETS; ++i) bulletImages[i].invalidate();
//    for (int j = 0; j < MAX_ENEMIES; ++j) enemyImages[j].invalidate();
}


void Screen2View::updatePoint(int newValue)
{
    Unicode::snprintf(txtBuffer, POINT_SIZE, "%d", newValue);
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
//    scores = tickCount / 20;
    tickCount = 0;
    flickerCount = 0;
    flickering = false;
    lives = 3;
    scores = 0;
    // reset bullets pool
    totalShotsFired = 0;
    fireCooldown = 0;
    for (int i = 0; i < MAX_ACTIVE_BULLETS; ++i) {
      bullets[i].active = false;
      bulletImages[i].setVisible(false);
    }

//    spaceshipX = 103;
//    spaceshipY = 206;
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

void Screen2View::ExitFromScreen2()
{
    presenter->saveLastScore(scores);   // lưu điểm
    // KHÔNG gọi nữa application().gotoScreen1… vì màn hình 1 đã đang ở đó rồi
}

