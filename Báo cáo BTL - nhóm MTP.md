# BTL_Nhung_20242
**Đề bài : Game Galaga**
**Sản phẩm:**
Tính năng:
1. Di chuyển bằng Joystick
2. Phát âm thành khi hạ địch và mất máu và bắn đạn(Còi Buzzer
3. Sinh kẻ địch tại các vị trí ngẫu nhiên và số lượng ngẫu nhiên(Dùng RNG)
![image](https://github.com/user-attachments/assets/72b9afba-1601-445b-b3ca-baf4af60afe4)
**Tác giả:**
---
Tên nhóm:MTP
| STT | Họ tên              | MSSV      | Vai trò     |
|-----|---------------------|-----------|-------------|
| 1   | Đỗ Nhật Minh        | 20225209  | Thiết kế còi buzzer, thiết kế RNG sinh ngẫu nhiên, sửa lỗi RNG và còi buzzer, chuẩn bị báo cáo             |
| 2   | Vũ Hữu Tiến         | 20225231  | Thiết kế phần cứng, thiết kế đối tượng và logic ban đầu Screenview 1,2 và sửa lỗi logic                    |
| 3   | Nguyễn Tuấn Phong   | 20225219  | Hoàn thiện giao diện, Debug hệ thống, tối ưu di chuyển Joystick và tính điểm, hoàn thiện sửa lỗi phần cứng |
**Môi trường hoạt động:**
---
- CPU / Dev Kit: STM32F429I-DISCOVERY
- Các module sử dụng:
 - Joystick di chuyển 2 trục x,y
 - Màn hình LCD trên bộ kit
- Phần mềm sử dụng:
 - TouchGFX 4.25
 - STM32CubeIDE 1.17.0
 - FreeRtos **SƠ ĐỒ SCHEMATIC**

| STM32F429            | Joystick          | Buzzer       | RNG                   |
|----------------------|-------------------|--------------|-----------------------|
| PA0 (ADC1_IN0)       | Trục X            |              |                       |
| PA5 (ADC1_IN5)       | Trục Y            |              |                       |
| PC3 (GPIO Input)     | Nút nhấn (SW)     |              |                       |
| 3V                   | VCC               | VCC          | VCC                   |
| GND                  | GND               | GND          | GND                   |
| PD12 (GPIO Output)   |                   | Buzzer In    |                       |
| Nội bộ               |                   |              | RNG tích hợp nội bộ    |


**TÍCH HỢP HỆ THỐNG**
- Phần cứng:
 - STM32F429I-DISCO: Board xử lý trung tâm, đọc giá trị joystick (ADC, GPIO), sinh số ngẫu nhiên, xử lý toàn bộ logic game bắn máy bay, điều khiển hiển thị và ngoại vi.
 - Màn hình: cảm ứng:Hiển thị giao diện game, điểm số, số mạng, các hiệu ứng. Nhận cảm ứng khi cần.
 - Joystick: Điều khiển di chuyển tàu bằng trục X/Y và bắn đạn bằng nút bấm.
 - Loa (Buzzer): Phát âm thanh khi bắn, va chạm hoặc hạ địch.
 - IC RNG: Sinh số ngẫu nhiên để tạo vị trí và số lượng enemy xuất hiện ngẫu nhiên.

- Phần mềm :
 - TouchGFX: Thiết kế và quản lý giao diện các màn hình (Start, Game, Restart), xử lý sự kiện chạm như nút Start Game, Restart.
 - FreeRTOS: Quản lý đa nhiệm, giúp chạy đồng thời giao diện TouchGFX và luồng đọc joystick/nút bấm.
 - STM32Cube HAL: Thư viện điều khiển phần cứng: đọc ADC cho joystick, điều khiển màn hình LTDC, giao tiếp I2C với touch controller.
 - Custom Game Logic: Xử lý toàn bộ thuật toán game (di chuyển tàu, bắn đạn, điều kiện thắng/thua, tính điểm), cập nhật trạng thái và va chạm giữa các đối tượng game.

**ĐẶC TẢ HÀM**
#  Các Hàm Quan Trọng trong `Screen2View.cpp`

---

##  `void Screen2View::handleTickEvent()`

```cpp
void Screen2View::handleTickEvent() {
    tickCount++;  // Tăng biến đếm số tick, thường dùng để đo thời gian trôi qua
```
Tick là đơn vị thời gian lặp lại mỗi frame  cần cho việc tính toán tốc độ và delay.
```cpp
    if (JoystickX != oldJoyX) {
        updateJoyX(JoystickX);  // Cập nhật hiển thị giá trị joystick nếu có thay đổi
        oldJoyX = JoystickX;    // Lưu lại giá trị cũ để so sánh ở tick sau
    }
```

Giúp tránh việc cập nhật GUI liên tục nếu joystick không thay đổi.

```cpp
    const int DEADZONE = 300;
    const int SPEED = 3;
    const int SHIP_Y_MAX = HAL::DISPLAY_HEIGHT - spaceship.getHeight() - 10;
```

Đặt ngưỡng chết (deadzone) để bỏ qua nhiễu nhỏ và giới hạn Y tàu.

```cpp
    int dx = 0, dy = 0;

    if (JoystickX < 2048 - DEADZONE) dx = -SPEED;
    else if (JoystickX > 2048 + DEADZONE) dx = SPEED;

    if (JoystickY < 2048 - DEADZONE) dy = -SPEED;
    else if (JoystickY > 2048 + DEADZONE) dy = SPEED;
```

Tính vận tốc X/Y dựa trên giá trị joystick.

```cpp
    spaceshipX = clamp(spaceshipX + dx, 0, HAL::DISPLAY_WIDTH - spaceship.getWidth());
    spaceshipY = clamp(spaceshipY + dy, 0, SHIP_Y_MAX);
    spaceship.moveTo(spaceshipX, spaceshipY);
```

Di chuyển tàu trong vùng giới hạn màn hình.

```cpp
    moveEnemiesTowardSpaceship();  // Di chuyển các enemy biết đuổi theo tàu
    bool fireRequest = shootFlag;
    shootFlag = false;
```

Nếu có nhấn nút bắn thì xử lý tại đây, rồi reset cờ bắn.

```cpp
    if (fireRequest && fireCooldown == 0 && totalShotsFired < MAX_TOTAL_SHOTS) {
        for (int i = 0; i < MAX_ACTIVE_BULLETS; ++i) {
            if (!bullets[i].active) {
                bullets[i].active = true;
                totalShotsFired++;
                bullets[i].x = spaceship.getX() + spaceship.getWidth() / 2 - bulletImages[i].getWidth() / 2;
                bullets[i].y = spaceship.getY();
                bulletImages[i].moveTo(bullets[i].x, bullets[i].y);
                bulletImages[i].setVisible(true);
                buzz(10);  // Rung nhẹ khi bắn
                break;
            }
        }
        fireCooldown = FIRE_COOLDOWN_TICKS;  // Thời gian chờ giữa 2 lần bắn
    }
    if (fireCooldown > 0) --fireCooldown;
```

```cpp
    for (int i = 0; i < MAX_ACTIVE_BULLETS; ++i) {
        if (bullets[i].active) {
            bullets[i].y -= BULLET_SPEED;  // Di chuyển đạn lên trên
            if (bullets[i].y < -bulletImages[i].getHeight()) {
                bullets[i].active = false;
                bulletImages[i].setVisible(false);
                continue;
            }
```

Xử lý di chuyển và loại bỏ đạn vượt ra ngoài màn hình.

```cpp
            bulletImages[i].moveTo(bullets[i].x, bullets[i].y);
            for (int j = 0; j < MAX_ENEMIES; ++j) {
                if (enemies[j].alive && checkCollision(bulletImages[i], enemyImages[j])) {
                    bullets[i].active = false;
                    bulletImages[i].setVisible(false);
                    enemies[j].alive = false;
                    enemyImages[j].setVisible(false);
                    enemyImages[j].invalidate();
                    buzz(50);  // Rung mạnh khi trúng enemy
                    scores += 10;
                    updatePoint(scores);
```
Xử lý va chạm giữa đạn và enemy, tăng điểm nếu trúng.
```cpp
                    totalEnemiesDefeated++;
                    if (totalEnemiesDefeated >= 50 && !playerHasWon) {
                        playerHasWon = true;
                        
                    }
                    break;
                }
            }
        }
    }
```

---

```cpp
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (enemies[i].alive && !flickering && checkCollision(spaceship, enemyImages[i])) {
            lives--;
            enemyImages[i].setVisible(false);
            enemyImages[i].invalidate();
            enemies[i].alive = false;
            updateHearts();
            buzz(250);  // Rung dài khi trúng enemy
            flickering = true;
            flickerCount = 0;
            break;
        }
    }
```
Xử lý va chạm giữa tàu và enemy.
```cpp
    if (flickering) {
        flickerCount++;
        if ((flickerCount & 1) == 0) {
            spaceship.setVisible(!spaceship.isVisible());  // Nhấp nháy khi bị thương
        }
        if (flickerCount >= 60) {
            flickering = false;
            spaceship.setVisible(true);
        }
    }
```

---

```cpp
    bool allEnemiesDead = true;
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (enemies[i].alive) {
            allEnemiesDead = false;
            break;
        }
    }
```
Kiểm tra nếu tất cả enemy đã bị tiêu diệt thì sinh ra enemy mới.
```cpp
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
```

---

 Kết thúc hàm `handleTickEvent()`. Hàm này là trái tim xử lý logic game trong từng frame (tick).

---






---

## `void Screen2View::buzz(uint16_t duration_ms)`

> Điều khiển buzzer bằng cách bật/tắt chân PB4 trong khoảng thời gian `duration_ms`.
```cpp
void Screen2View::buzz(uint16_t duration_ms)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);   // Bật buzzer (PB4 = HIGH)
    HAL_Delay(duration_ms);                               // Giữ trạng thái trong duration_ms mili giây
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET); // Tắt buzzer (PB4 = LOW)
}
```
---
## `void Screen2View::spawnRandomEnemies(int n)`
> Tạo ra `n` enemy ngẫu nhiên ở vị trí ngẫu nhiên (chỉ tạo nếu còn enemy chưa hoạt động).
```cpp
void Screen2View::spawnRandomEnemies(int n)
{
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < MAX_ENEMIES; ++j)
        {
            if (!enemies[j].alive) {
                uint32_t randNumX, randNumY;
                HAL_RNG_GenerateRandomNumber(&hrng, &randNumX);  // Lấy số ngẫu nhiên cho tọa độ X
                HAL_RNG_GenerateRandomNumber(&hrng, &randNumY);  // Lấy số ngẫu nhiên cho tọa độ Y
                int x = randNumX % (HAL::DISPLAY_WIDTH - enemyImages[j].getWidth());
                int y = randNumY % ((HAL::DISPLAY_HEIGHT / 2) - enemyImages[j].getHeight());
                enemies[j].alive = true;
                enemies[j].chasePlayer = true;  // Cho phép enemy rượt đuổi tàu
                enemies[j].x = x;
                enemies[j].y = y;
                enemyImages[j].moveTo(x, y);             // Cập nhật vị trí hiển thị
                enemyImages[j].setVisible(true);
                enemyImages[j].invalidate();             // Vẽ lại enemy trên màn hình
                break;
            }
        }
    }
}
```

---

## `void Screen2View::moveEnemiesTowardSpaceship()`

> Di chuyển các enemy đang hoạt động về phía tàu theo vector chuẩn hóa.
```cpp
void Screen2View::moveEnemiesTowardSpaceship()
{
    const int ENEMY_SPEED = 2;
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (enemies[i].alive && enemies[i].chasePlayer) {
            int dx = spaceshipX + spaceship.getWidth()/2 - (enemies[i].x + enemyImages[i].getWidth()/2);
            int dy = spaceshipY + spaceship.getHeight()/2 - (enemies[i].y + enemyImages[i].getHeight()/2);

            float length = sqrtf(dx*dx + dy*dy);  // Tính độ dài vector hướng

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
```

---

## `void Screen2View::updateHearts()`

> Cập nhật icon mạng sống dựa trên biến `lives`. Nếu hết mạng sẽ reset game và quay về màn hình chính.
```cpp
void Screen2View::updateHearts()
{
    switch (lives) {
        case 0:
            resetGame();  // Reset game khi hết mạng
            application().gotoScreen1ScreenNoTransition();  // Chuyển về màn hình Screen1
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
```

---

## `void Screen2View::resetGame()`

> Reset toàn bộ trạng thái game về ban đầu: điểm, mạng, đạn, enemy...
```cpp
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

    spaceship.moveTo(103, 206);  // Đặt lại vị trí tàu

    updatePoint(0);
    updateHearts();
}
```

---

## `bool Screen2View::checkCollision(...)`

> Kiểm tra va chạm giữa 2 hình ảnh theo bounding box (hình chữ nhật).
```cpp
bool Screen2View::checkCollision(const Image& img1, const Image& img2)
{
    return (img1.getX() < img2.getX() + img2.getWidth() &&
            img1.getX() + img1.getWidth() > img2.getX() &&
            img1.getY() < img2.getY() + img2.getHeight() &&
            img1.getY() + img1.getHeight() > img2.getY());
}
```
---
#  Screen2View.hpp – Phân tích các thành phần và cấu trúc lớp
##  Cấu trúc tổng quát
```cpp
class Screen2View : public Screen2ViewBase
```
Lớp `Screen2View` kế thừa từ `Screen2ViewBase`, là lớp giao diện trong TouchGFX điều khiển logic và hiển thị của màn hình chơi game.

---

##  Các `define` cấu hình
```cpp
#define ENEMY_ROWS      5
#define ENEMY_COLS      5
#define MAX_ENEMIES     (ENEMY_ROWS * ENEMY_COLS)
```
- Xác định số lượng dòng/cột của enemy (kẻ địch) là 5x5 → tổng là 25 enemy.

```cpp
#define BULLET_SPEED    3
#define FIRE_COOLDOWN_TICKS 15
```
- Tốc độ đạn là 3 pixel/tick. Đợi 15 tick sau mới được bắn tiếp.
```cpp
static const int MAX_ACTIVE_BULLETS = 50;
static const int MAX_TOTAL_SHOTS   = 1000000000;
```
- Tối đa 50 viên đạn đang bay. Số lần bắn tối đa rất lớn để tránh giới hạn.

---

##  Các struct

### `struct Bullet`
```cpp
struct Bullet {
    int x, y;
    bool active;
};
```
Đại diện cho 1 viên đạn đang di chuyển.
### `struct Enemy`
```cpp
struct Enemy {
    int x, y;
    bool alive;
    bool chasePlayer;
};
```
Đại diện cho 1 enemy: có vị trí, trạng thái sống/chết, và có đuổi theo người chơi hay không.

---

##  Hàm công khai (public)

| Hàm | Mô tả |
|-----|------|
| `Screen2View()` | Constructor khởi tạo |
| `~Screen2View()` | Destructor |
| `setupScreen()` | Cài đặt ban đầu cho screen |
| `tearDownScreen()` | Hủy tài nguyên khi rời screen |
| `handleTickEvent()` | Gọi mỗi tick để cập nhật game |
| `ExitFromScreen2()` | Chuyển màn hình và lưu điểm |
| `updatePoint(int)` | Cập nhật điểm số trên giao diện |
| `updateHearts()` | Cập nhật trái tim (mạng sống) |
| `resetGame()` | Đặt lại toàn bộ trạng thái game |
| `checkCollision(...)` | Kiểm tra va chạm giữa 2 ảnh |
| `updateJoyX(...)` | Cập nhật giá trị joystick |
| `buzz(ms)` | Rung motor trong ms mili-giây |
| `clamp(val, min, max)` | Hàm giới hạn giá trị |
| `getScore()` | Lấy điểm hiện tại |
| `spawnRandomEnemies(n)` | Sinh thêm n enemy ngẫu nhiên |
| `moveEnemiesTowardSpaceship()` | Cho enemy đuổi theo tàu |

---


**KÊT QUẢ**
LINK VIDEO DEMO:https://drive.google.com/file/d/127BjGbm-vWZzeKWVDejcFgOb9IJpmtmR/view?usp=sharing
