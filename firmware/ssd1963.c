//Complete driver for the SSD1963 written in C for cortex M0
//See manufacturing documentation for usage
#include "ssd1963.h"
#include "LPC11xx.h"

__inline void configGPIOread(){
	LPC_GPIO0->DIR = 0x00;
}

__inline void configGPIOwrite(){
	LPC_GPIO0->DIR = 0x1E;
}

unsigned char SSD1963_ReadData(void){
	unsigned char result;
	LPC_GPIO1->DATA  |= SSD1963_PIN_WR | SSD1963_PIN_DC;
	LPC_GPIO1->MASKED_ACCESS[SSD1963_PIN_RD] = ~SSD1963_PIN_RD;
	configGPIOread();
	result = (LPC_GPIO0->DATA >> 1) & 0xFF;
	LPC_GPIO1->MASKED_ACCESS[SSD1963_PIN_RD]  = SSD1963_PIN_RD;
	configGPIOwrite();
	return result;
}

void SSD1963_WriteCommand(unsigned int commandToWrite){
	//LPC_GPIO0->DATA = (commandToWrite << 1) | (LPC_GPIO0->DATA & 0x1);
	LPC_GPIO0->MASKED_ACCESS[0x1FE] = commandToWrite<<1;
	//LPC_GPIO1->MASKED_ACCESS[SSD1963_PIN_RD] = SSD1963_PIN_RD ;
	LPC_GPIO1->DATA &= ~SSD1963_PIN_DC & ~SSD1963_PIN_WR;
	LPC_GPIO1->MASKED_ACCESS[SSD1963_PIN_WR]  = SSD1963_PIN_WR;
	
}

void SSD1963_WriteData(unsigned int dataToWrite){
	LPC_GPIO0->MASKED_ACCESS[0x1FE] = dataToWrite<<1;
	LPC_GPIO1->MASKED_ACCESS[SSD1963_PIN_DC]  = SSD1963_PIN_DC;
	LPC_GPIO1->MASKED_ACCESS[SSD1963_PIN_WR]  = ~SSD1963_PIN_WR;
	LPC_GPIO1->MASKED_ACCESS[SSD1963_PIN_WR]  = SSD1963_PIN_WR;
}




void SSD1963_SetPWM(void){
	SSD1963_WriteCommand(SSD1963_SET_PWM_CONF);
	//Frequency
	SSD1963_WriteData(0x08);
	SSD1963_WriteData(0x80);
	//Enable pwm for backlight control
	SSD1963_WriteData(0x09);
	//Manual bright level	00 is min, FF is max
	SSD1963_WriteData(0xFF);	
	//Min bright level
	SSD1963_WriteData(0x00);
	//Scale for transition effect, 0x8 for 16
	SSD1963_WriteData(0x08);
}

void backlight_threshold(void){
	SSD1963_WriteCommand(SSD1963_SET_DBC_CONF);
	//Conservative
	SSD1963_WriteData(0x00);
	SSD1963_WriteData(0x1c);
	SSD1963_WriteData(0x20);
	//Normal
	SSD1963_WriteData(0x00);
	SSD1963_WriteData(0x46);
	SSD1963_WriteData(0x50);
	//Aggressive
	SSD1963_WriteData(0x00);
	SSD1963_WriteData(0xA8);
	SSD1963_WriteData(0xC0);
	
}

void set_backlight_ctrl(void){
	SSD1963_SetPWM();
	backlight_threshold();
	SSD1963_WriteCommand(SSD1963_SET_DBC_CONF);
	SSD1963_WriteData(0x2D);
}

void set_scroll_area(uint16_t top, uint16_t mid, uint16_t bottom){
	SSD1963_WriteCommand(SSD1963_SET_SCROLL_AREA);
	SSD1963_WriteData(mHIGH(top));
	SSD1963_WriteData(mLOW(top));
	SSD1963_WriteData(mHIGH(mid));
	SSD1963_WriteData(mLOW(mid));
	SSD1963_WriteData(mHIGH(bottom));
	SSD1963_WriteData(mLOW(bottom));
}

void set_scroll_start(uint16_t line){
	SSD1963_WriteCommand(SSD1963_SET_SCROLL_START);
	SSD1963_WriteData(mHIGH(line));
	SSD1963_WriteData(mLOW(line));
}

void SSD1963_Hardware_RST(void){
	volatile unsigned int dly;
	LPC_GPIO1->DATA &= ~SSD1963_PIN_RST;	
	for(dly = 0; dly < 0x1FFFF; dly++);
	LPC_GPIO1->DATA |= SSD1963_PIN_RST;
	for(dly = 0; dly < 0x1FFFF; dly++);
	SSD1963_WriteCommand(SSD1963_SOFT_RESET);
	for(dly = 0; dly < 0x1FFFF; dly++);
}

void SSD1963_SetandLock_PLL(uint8_t mult, uint8_t div){
	volatile unsigned int dly;
	SSD1963_WriteCommand(SSD1963_SET_PLL_MN);
	//SET DLL ; Refclock should be 10MHz Oscilator
	/*SSD1963_WriteData(49);	// PLLclk = REFclk * 50 (500MHz)
	SSD1963_WriteData(4);	// SYSclk = PLLclk / 5  (100MHz)
	SSD1963_WriteData(4);  // dummy
	*/
	SSD1963_WriteData(0x23);		//10*24 / 3 = 80Mhz
	SSD1963_WriteData(0x2);
	SSD1963_WriteData(0x4);			//dummy byte
	//LOCK PLL
	SSD1963_WriteCommand(SSD1963_SET_PLL);
	SSD1963_WriteData(0x01);
	for(dly = 0; dly < 0x3FFFF; dly++);
	SSD1963_WriteCommand(SSD1963_SET_PLL);
	SSD1963_WriteData(0x03);
	SSD1963_WriteCommand(SSD1963_SOFT_RESET);
	for(dly = 0; dly < 0x0FFFF; dly++);	
}

void SSD1963_SetPixelClk(uint32_t mult){
	// Pclock = PLL clock* value_written /2^20
	SSD1963_WriteCommand(SSD1963_SET_LSHIFT_FREQ); 
	SSD1963_WriteData(0x03);
	SSD1963_WriteData(0xff);
	SSD1963_WriteData(0xff);
	/*	SSD1963_WriteData((LCD_FPR >> 16) & 0xFF);
			SSD1963_WriteData((LCD_FPR >> 8) & 0xFF);
			SSD1963_WriteData(LCD_FPR & 0xFF);	*/
}

void SSD1963_Init (void){
	
	SSD1963_Hardware_RST();
	
	SSD1963_SetandLock_PLL(23,2);
	
	SSD1963_SetPixelClk(0x03ffff);
	//set pixel clock by 30Mhz
		
	//SET LCD MODE
	SSD1963_WriteCommand(SSD1963_SET_LCD_MODE); 
	//vert+horiz sync active low
	//data latch in rising edge + enable TFT FRC disable TFT dithering
	SSD1963_WriteData(0x0C);			
	//TFT mode in bit 5+6
	SSD1963_WriteData(0x00);			
	//SET dimensions 800 * 480
	SSD1963_WriteData(mHIGH((TFT_WIDTH-1)));
	SSD1963_WriteData(mLOW((TFT_WIDTH-1)));
	SSD1963_WriteData(mHIGH((TFT_HEIGHT-1)));
	SSD1963_WriteData(mLOW((TFT_HEIGHT-1)));
	//Send in sequence RGB
	SSD1963_WriteData(0x00);
	
	//set pixel data by 8 bit * 3 times
	SSD1963_WriteCommand(SSD1963_SET_PIXEL_DATA_INTERFACE);
	SSD1963_WriteData(SSD1963_PDI_8BIT);

	SSD1963_WriteCommand(SSD1963_SET_HORI_PERIOD);
	SSD1963_WriteData(((928>>8) &0x0FF));
	SSD1963_WriteData(((928) &0x0FF));
	SSD1963_WriteData(((46>>8) &0x0FF));
	SSD1963_WriteData(((46) &0x0FF));
	SSD1963_WriteData(((48) &0x0FF));
	SSD1963_WriteData(((15>>8) &0x0FF));
	SSD1963_WriteData(((15) &0x0FF));
	SSD1963_WriteData(0x00);			
	
	SSD1963_WriteCommand(SSD1963_SET_VERT_PERIOD);
	SSD1963_WriteData(((525>>8) &0x0FF));
	SSD1963_WriteData(((525) &0x0FF));
	SSD1963_WriteData(((16>>8) &0x0FF));
	SSD1963_WriteData(((16) &0x0FF));
	SSD1963_WriteData(((16) &0x0FF));
	SSD1963_WriteData(((8>>8) &0x0FF));
	SSD1963_WriteData(((8) &0x0FF));
	
	SSD1963_WriteCommand(SSD1963_SET_DISPLAY_ON);		//SET display on
	set_address_mode(0x02);
	//set_backlight_ctrl();
	//set_scroll_area(40, 400 ,39);
	/*
	//Total period
	SSD1963_WriteData(mHIGH(TFT_HSYNC_PERIOD));
	SSD1963_WriteData(mLOW(TFT_HSYNC_PERIOD));
	//non display period
	SSD1963_WriteData(mHIGH((TFT_HSYNC_PULSE + TFT_HSYNC_BACK_PORCH)));
	SSD1963_WriteData(mLOW((TFT_HSYNC_PULSE + TFT_HSYNC_BACK_PORCH)));
	//horiz pulse width
	SSD1963_WriteData(TFT_HSYNC_PULSE);
	//start location in pix clock
	SSD1963_WriteData(0x00);			
	
	
	SSD1963_WriteCommand(SSD1963_SET_VERT_PERIOD); 		
	SSD1963_WriteData(mHIGH(TFT_VSYNC_PERIOD));
	SSD1963_WriteData(mLOW(TFT_VSYNC_PERIOD));
	SSD1963_WriteData(mHIGH((TFT_VSYNC_PULSE + TFT_VSYNC_BACK_PORCH)));
	SSD1963_WriteData(mLOW((TFT_VSYNC_PULSE + TFT_VSYNC_BACK_PORCH)));
	SSD1963_WriteData(TFT_VSYNC_PULSE);
	SSD1963_WriteData(0x00);			
	SSD1963_WriteData(0x00);
	*/
}

void SSD1963_SetArea(unsigned int sx, unsigned int ex, unsigned int sy, unsigned int ey){
	SSD1963_WriteCommand(SSD1963_SET_COLUMN_ADDRESS);	
	SSD1963_WriteData((sx >> 8) & 0xFF);
	SSD1963_WriteData((sx >> 0) & 0xFF);
	SSD1963_WriteData((ex >> 8) & 0xFF);
	SSD1963_WriteData((ex >> 0) & 0xFF);

	SSD1963_WriteCommand(SSD1963_SET_PAGE_ADDRESS);	
	SSD1963_WriteData((sy >> 8) & 0xFF);
	SSD1963_WriteData((sy >> 0) & 0xFF);
	SSD1963_WriteData((ey >> 8) & 0xFF);
	SSD1963_WriteData((ey >> 0) & 0xFF);
}

void SSD1963_FillArea(unsigned int sx, unsigned int ex, unsigned int sy, unsigned int ey, int color){
	int i;
	SSD1963_SetArea(sx, ex, sy, ey);
	SSD1963_WriteCommand(SSD1963_WRITE_MEMORY_START);
	for(i = 0; i < ((ex-sx+1)*(ey-sy+1)); i++)
	{
		SSD1963_WriteData(color >> 16);
		SSD1963_WriteData((color >> 8)&0xFF);
		SSD1963_WriteData(color & 0xFF);
	}
}
void SSD1963_SetPixel(int x, int y, int color){
	SSD1963_SetArea(x, x, y, y);
	SSD1963_WriteCommand(SSD1963_WRITE_MEMORY_START);
	SSD1963_WriteData(color >> 16);
	SSD1963_WriteData((color >> 8)&0xFF);
	SSD1963_WriteData(color & 0xFF);
}

//MODE: 
void set_address_mode(uint8_t in_mode){
	
	SSD1963_WriteCommand(SSD1963_SET_ADDRESS_MODE);
	SSD1963_WriteData(in_mode);
}
