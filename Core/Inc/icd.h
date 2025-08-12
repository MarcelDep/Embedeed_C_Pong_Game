#pragma once
#include <stdint.h>
#include <stdbool.h>
#define LCD_WIDTH	160
#define LCD_HEIGHT	128
#define LOST -1
#define DEFAULT 0
#define LEFT 1
#define RIGHT 2
#define MIDDLE 3
#define UP 4
#define DOWN 5
#define SPEED_NO_1 2
#define SPEED_NO_2 3
#define SPEED_NO_3 4
#define SPEED_NO_4 5
#define SPEED_NO_5 6
#define SPEED_NO_6 7
//Definicje kolorów z odwróconą kolejnością bajtów
#define BLACK 0x0000
#define WHITE 0xffff
#define ST7735S_SLPOUT			0x11
#define ST7735S_DISPOFF			0x28
#define ST7735S_DISPON			0x29
#define ST7735S_CASET			0x2a
#define ST7735S_RASET			0x2b
#define ST7735S_RAMWR			0x2c
#define ST7735S_MADCTL			0x36
#define ST7735S_COLMOD			0x3a
#define ST7735S_FRMCTR1			0xb1
#define ST7735S_FRMCTR2			0xb2
#define ST7735S_FRMCTR3			0xb3
#define ST7735S_INVCTR			0xb4
#define ST7735S_PWCTR1			0xc0
#define ST7735S_PWCTR2			0xc1
#define ST7735S_PWCTR3			0xc2
#define ST7735S_PWCTR4			0xc3
#define ST7735S_PWCTR5			0xc4
#define ST7735S_VMCTR1			0xc5
#define ST7735S_GAMCTRP1		0xe0
#define ST7735S_GAMCTRN1		0xe1

void lcd_init(void); // Inicjalizacja wyświetlacza
void lcd_put_pixel(int x, int y, uint16_t color);
void lcd_copy(void); // Przesłanie zawartości bufora
bool buttonPressed();
void move_bar(int x, bool direction);
void lcd_transfer_done(void);
bool lcd_is_busy(void);
bool joyStickPressed(void);
uint16_t movePlatform(uint16_t x, uint16_t direction, uint16_t platformSpeed);
bool joyStickPressed();
uint16_t joyStickVRX(uint16_t joyStickPosition);
struct direction ballMovement(uint16_t ballPositionOX, uint16_t ballPositionOY, int ballSpeed, int ballDirectionOX, int ballDirectionOY, uint16_t platformPositionOX, int loss, uint16_t platformDirection, int basicSpeed);
struct direction chooseRandomDirection(uint16_t, uint16_t);
int changeBallSpeed(int ballSpeed, int ballDirectionOX, uint16_t platformDirection, int basicSpeed);
