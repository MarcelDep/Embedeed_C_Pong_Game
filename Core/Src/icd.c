#include "icd.h"
#include "spi.h"
#include <stdio.h>
#include <stdlib.h>

#define CMD(x)			((x) | 0x100)
#define LCD_OFFSET_X  1
#define LCD_OFFSET_Y  2

struct direction {
	uint16_t valueOX;
	uint16_t valueOY;
	int DirectionOX;
	int DirectionOY;
	int loss;
	int ballSpeed;
};

struct direction changedDirection;
struct direction lostGame = {0, 0, 0, 0, 0};

volatile bool lcd_dma_busy = false;

static uint16_t frame_buffer[LCD_WIDTH * LCD_HEIGHT];

static void lcd_cmd(uint8_t cmd)
{
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi2, &cmd, 1, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

static void lcd_data(uint8_t data)
{
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi2, &data, 1, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

static void lcd_send(uint16_t value)
{
	if (value & 0x100) {
		lcd_cmd(value);
	} else {
		lcd_data(value);
	}
}

static const uint16_t init_table[] = {
  CMD(ST7735S_FRMCTR1), 0x01, 0x2c, 0x2d,
  CMD(ST7735S_FRMCTR2), 0x01, 0x2c, 0x2d,
  CMD(ST7735S_FRMCTR3), 0x01, 0x2c, 0x2d, 0x01, 0x2c, 0x2d,
  CMD(ST7735S_INVCTR), 0x07,
  CMD(ST7735S_PWCTR1), 0xa2, 0x02, 0x84,
  CMD(ST7735S_PWCTR2), 0xc5,
  CMD(ST7735S_PWCTR3), 0x0a, 0x00,
  CMD(ST7735S_PWCTR4), 0x8a, 0x2a,
  CMD(ST7735S_PWCTR5), 0x8a, 0xee,
  CMD(ST7735S_VMCTR1), 0x0e,
  CMD(ST7735S_GAMCTRP1), 0x0f, 0x1a, 0x0f, 0x18, 0x2f, 0x28, 0x20, 0x22,
                         0x1f, 0x1b, 0x23, 0x37, 0x00, 0x07, 0x02, 0x10,
  CMD(ST7735S_GAMCTRN1), 0x0f, 0x1b, 0x0f, 0x17, 0x33, 0x2c, 0x29, 0x2e,
                         0x30, 0x30, 0x39, 0x3f, 0x00, 0x07, 0x03, 0x10,
  CMD(0xf0), 0x01,
  CMD(0xf6), 0x00,
  CMD(ST7735S_COLMOD), 0x05,
  CMD(ST7735S_MADCTL), 0xa0,
};

static void lcd_data16(uint16_t value)
{
	lcd_data(value >> 8);
	lcd_data(value);
}

static void lcd_set_window(int x, int y, int width, int height)
{
  lcd_cmd(ST7735S_CASET);
  lcd_data16(LCD_OFFSET_X + x);
  lcd_data16(LCD_OFFSET_X + x + width - 1);
  lcd_cmd(ST7735S_RASET);
  lcd_data16(LCD_OFFSET_Y + y);
  lcd_data16(LCD_OFFSET_Y + y + height- 1);
}

void lcd_init(void)
{
  int i;

  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
  HAL_Delay(100);
  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(100);

  for (i = 0; i < sizeof(init_table) / sizeof(uint16_t); i++) {
    lcd_send(init_table[i]);
  }

  HAL_Delay(200);

  lcd_cmd(ST7735S_SLPOUT);
  HAL_Delay(120);

  lcd_cmd(ST7735S_DISPON);
}

void lcd_put_pixel(int x, int y, uint16_t color)
{
	frame_buffer[x + y * LCD_WIDTH] = color;
}

void lcd_copy(void)
{
	lcd_dma_busy = true;
	lcd_set_window(0, 0, LCD_WIDTH, LCD_HEIGHT);
	lcd_cmd(ST7735S_RAMWR);
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit_DMA(&hspi2, (uint8_t*)frame_buffer, sizeof(frame_buffer));
}

bool lcd_is_busy(void)
{
	return lcd_dma_busy;
}

void lcd_transfer_done(void)
{
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
	lcd_dma_busy = false;
}

void move_bar(int x, bool direction) {
	if (direction == false) {
		hagl_fill_rounded_rectangle(x+5, 69, 21, 61, 5, BLACK);
		hagl_fill_rounded_rectangle(x, 70, 20, 60, 5, WHITE);
	}
	else {
		hagl_fill_rounded_rectangle(x, 70, 21, 60, 5, WHITE);
	}
}

uint16_t joyStickVRX(uint16_t joyStickPosition) {
	/* Jeżeli gałka została przesunięta w prawo to zwróć informacje ze structa direction czy w prawo czy w lewo
	   analogicznie dla gałki przesunietej w lewo */
	if (joyStickPosition > 3050) {
		return RIGHT;
	}
	else if ((joyStickPosition < 3050) && (joyStickPosition > 1000)) {
		return MIDDLE;
	}
	else {
		return LEFT;
	}
}

bool joyStickPressed() {
	/* jeżeli joystick został naciśnięty to zwróć prawde (domyślnie ma wartość false)*/
	if (HAL_GPIO_ReadPin(SW_GPIO_Port, SW_Pin) == GPIO_PIN_RESET) {
		return true;
	}
	else {
		return false;
	}
}

uint16_t movePlatform(uint16_t x, uint16_t joyStickValue, uint16_t platformSpeed) {
	/* Jeżeli direction to right to wtedy wykonaj animacje przesunięcia o 2 pixele w prawo (chyba, że dojdziesz do granicy)
	   Analogicznie w lewo */
	hagl_fill_rectangle(x - 25, 121, x + 25, 124, BLACK);
	if ((joyStickValue == LEFT) && (x > 29)) {
		x-=platformSpeed;
	}
	else if ((joyStickValue == RIGHT) && (x < 129)) {
		x+=platformSpeed;
	}
	hagl_fill_rectangle(x - 25, 121, x + 25, 124, WHITE);
	return x;
}

int changeBallSpeed(int ballSpeed, int ballDirectionOX, uint16_t platformDirection, int basicSpeed) {
	if (ballSpeed < basicSpeed - 1) {
		ballSpeed+=1;
	}
	if (ballDirectionOX == platformDirection) {
		if ((ballSpeed + 1) <= (basicSpeed + 1)) {
					return ballSpeed+=1;
				}
				else {
					return ballSpeed;
				}
	}
	else if (ballDirectionOX != platformDirection && platformDirection != MIDDLE) {
		if ((ballSpeed - 1) >= (basicSpeed - 1)) {
			return ballSpeed-=1;
		}
		else {
			return ballSpeed;
		}
	}
	else {
		return ballSpeed;
	}
}

struct direction ballMovement(uint16_t ballPositionOX, uint16_t ballPositionOY, int ballSpeed, int ballDirectionOX, int ballDirectionOY, uint16_t platformPositionOX, int loss, uint16_t platformDirection, int basicSpeed) {
	/* w tej funkcji opiszę to jak będzie się poruszać piłka co każdą klatkę, kąt padania = kąt odbicia w momencie w którym platforma jest nieruchoma albo odbija się od ścian
	 * jeżeli platforma się porusza i piłka odbija się od platformy to musimy lekko zmienić kąt odbicia - jeżeli platforma rusza się w lewo, to w prawo jeżeli w prawo to w lewo */
	struct direction previousDirection = {.valueOX = ballPositionOX,
										  .valueOY = ballPositionOY,
  										  .DirectionOX = ballDirectionOX,
										  .DirectionOY = ballDirectionOY,
										  .loss = loss};
	// Zamazujemy pozycję starej piłki
	hagl_fill_circle(ballPositionOX, ballPositionOY, 2, BLACK);
	// Piłka trafia w lewą ścianę - zmiana kierunku na prawo
	if (ballPositionOX - ballSpeed < 6) {
		previousDirection.DirectionOX = RIGHT;
		ballPositionOX = 6;
	}
	// Piłka trafia w prawą ścianę - zmiana kierunku na lewo
	else if (ballPositionOX + ballSpeed > 152) {
		previousDirection.DirectionOX = LEFT;
		ballPositionOX = 152;
	}
	// Piłka trafia w górną ścianę - zmiana kierunku na dół
	if (ballPositionOY - ballSpeed < 6) {
		previousDirection.DirectionOY = DOWN;
		ballPositionOY = 6;
	}
	// Piłka odbita od gracza - zmiana kierunku na górę
	else if ((ballPositionOY + ballSpeed > 118 && ((platformPositionOX - 25 < ballPositionOX) && (ballPositionOX < platformPositionOX + 25)))) {
		previousDirection.DirectionOY = UP;
		ballSpeed = changeBallSpeed(ballSpeed, ballDirectionOX, platformDirection, basicSpeed);
		ballPositionOY = 118;
	}
	// Piłka nie odbita od gracza - porażka
	else if (ballPositionOY + ballSpeed > 118) {
		previousDirection.loss = LOST;
		return lostGame;
	}
	// Jeżeli kierunek to lewo - to przesówamy piłkę w lewo o wartość równą jej prędkości
	if (previousDirection.DirectionOX == LEFT) {
		ballPositionOX -= ballSpeed;
	}
	// Jeżeli kierunek to lewo - to przesówamy piłkę w prawo o wartość równą jej prędkości
	if (previousDirection.DirectionOX == RIGHT) {
		ballPositionOX += ballSpeed;
	}
	// Jeżeli kierunek to lewo - to przesówamy piłkę w dół o wartość równą jej prędkości
	if (previousDirection.DirectionOY == DOWN) {
		ballPositionOY += ballSpeed;
	}
	// Jeżeli kierunek to lewo - to przesówamy piłkę w gór o wartość równą jej prędkości
	if (previousDirection.DirectionOY == UP) {
		ballPositionOY -= ballSpeed;
	}
	// Rysujemy nową pozycję piłki
	hagl_fill_circle(ballPositionOX, ballPositionOY, 2, WHITE);
	lcd_copy();
	// Zwracamy zmienione wartości o położeniu piłki
	previousDirection.ballSpeed = ballSpeed;
	previousDirection.valueOX = ballPositionOX;
	previousDirection.valueOY = ballPositionOY;
	// Jeżeli nic z powyższych to kontynuujemy animacje nie zmieniając żadnych wartości w strukturze poza pozycją piłki
	// W każdym ptzypadku zwracamy strukturę - albo zmienioną, albo nie
	return previousDirection;
}

struct direction chooseRandomDirection(uint16_t valueOX, uint16_t valueOY) {
	// W tej funkcji losujemy wartości z jakimi piłka będzie rozpoczynała grę
	struct direction tempStruct;
	tempStruct.valueOX = valueOX;
	tempStruct.valueOY = valueOY;
	tempStruct.DirectionOX = (LEFT + (rand() % 2));
	tempStruct.DirectionOY = (UP + (rand() % 2));
	tempStruct.loss = DEFAULT;
	tempStruct.ballSpeed = SPEED_NO_1;
	return tempStruct;
}
