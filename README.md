# Embedeed_C_Pong_Game
This programm is an implementation of one-person pong-game. I've made this game on NUCLEO-L476RG board so if you want to use different stm32 microcontroler you should change PIN's settings. Also, I've used ST7735S Controller to display the game and B103 267 JoyStick for platform control.

I've connected ST7735S and B103 267 to following PIN's:
- ST7735S:
    - VCC - CN7 PIN 5 (VDD)
    - GND - CN7 PIN 8 (GND)
    - DIN - CN7 PIN 37 (PC3)
    - CLK - CN10 PIN 25 (PB10)
    - CS - CN10 PIN 16 (PB12)
    - DC - CN10 PIN 18 (PB11)
    - RST - CN10 PIN 22 (PB2)
    - BL - None
- B103 267:
    - GND - CN6 PIN 7 (GND)
    - +5V - CN6 PIN 5 (+5V)
    - VRX - CN8 PIN 1 A0
    - VRY - CN8 PIN 2 A1
    - SW - CN5 PIN 1 D8
 
<img width="720" height="695" alt="PIN'y na płytce" src="https://github.com/user-attachments/assets/ca1805b4-aa08-473d-ab77-76a94867bcfa" />
