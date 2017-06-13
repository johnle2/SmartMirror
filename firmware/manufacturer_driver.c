/*******************************************************************************
 * Function Name : UILCD_Init
 * Description : Initializes LCD.
 * Input : None
 * Output : None
 * Return : None
 *******************************************************************************/
void TFT_7_Init(void)
{
    GPIO_ResetBits(GPIOC, CS1);
    GPIO_SetBits(GPIOC, nRD);
    GPIO_ResetBits(GPIOC, nWR);
    GPIO_WriteBit(GPIOC, RES, Bit_RESET);
    TFT_delay(5);
    GPIO_WriteBit(GPIOC, RES, Bit_SET);
    TFT_delay(100);
    TFT_7_Write_Command(0x01); //Software Reset
    TFT_7_Write_Command(0x01);
    TFT_7_Write_Command(0x01);
    TFT_delay(10);
    TFT_7_Command_Write(0xe0,0x01); //START PLL
    TFT_7_Command_Write(0xe0,0x03); //LOCK PLL
    TFT_7_Write_Command(0xb0); //SET LCD MODE SET TFT 18Bits MODE
    GPIO_SetBits(GPIOC, RS);
    TFT_7_Write_Data(0x08); //SET TFT MODE & hsync+Vsync+DEN MODE
    TFT_7_Write_Data(0x80); //SET TFT MODE & hsync+Vsync+DEN MODE
    TFT_7_Write_Data(0x03); //SET horizontal size=800-1 HightByte
    TFT_7_Write_Data(0x1f); //SET horizontal size=800-1 LowByte
    TFT_7_Write_Data(0x01); //SET vertical size=480-1 HightByte
    TFT_7_Write_Data(0xdf); //SET vertical size=480-1 LowByte
    TFT_7_Write_Data(0x00); //SET even/odd line RGB seq.=RGB
    TFT_7_Command_Write(0xf0,0x00); //SET pixel data I/F format=8bit
    TFT_7_Command_Write(0x36,0x09); //SET address mode=flip vertical, BGR
    TFT_7_Command_Write(0x3a,0x60); // SET R G B format = 6 6 6
    TFT_7_Write_Command(0xe2); //SET PLL freq=113.33MHz
    GPIO_SetBits(GPIOC, RS);
    TFT_7_Write_Data(0x22);
    TFT_7_Write_Data(0x03);
    TFT_7_Write_Data(0x04);
    TFT_7_Write_Command(0xe6); //SET PCLK freq=33.26MHz
    GPIO_SetBits(GPIOC, RS);
    TFT_7_Write_Data(0x02);
    TFT_7_Write_Data(0xff);
    TFT_7_Write_Data(0xff);
    TFT_7_Write_Command(0xb4); //SET HBP,
    GPIO_SetBits(GPIOC, RS);
    TFT_7_Write_Data(0x03); //SET HSYNC Total
    TFT_7_Write_Data(0xef);
    TFT_7_Write_Data(0x00); //SET HBP
    TFT_7_Write_Data(0xa3);
    TFT_7_Write_Data(0x07); //SET VBP
    TFT_7_Write_Data(0x00); //SET Hsync pulse start position
    TFT_7_Write_Data(0x00); 
    TFT_7_Write_Data(0x00); //SET Hsync pulse subpixel start position
    TFT_7_Write_Command(0xb6); //SET VBP,
    GPIO_SetBits(GPIOC, RS);
    TFT_7_Write_Data(0x01); //SET Vsync total = 496
    TFT_7_Write_Data(0xef);
    TFT_7_Write_Data(0x00); //SET VBP=4
    TFT_7_Write_Data(0x04);
    TFT_7_Write_Data(0x01); //SET Vsync pulse 2=1+1
    TFT_7_Write_Data(0x00); //SET Vsync pulse start position
    TFT_7_Write_Data(0x00);
    TFT_7_Write_Command(0x2a); //SET column address
    GPIO_SetBits(GPIOC, RS);
    TFT_7_Write_Data(0x00); //SET start column address=0
    TFT_7_Write_Data(0x00);
    TFT_7_Write_Data(0x03); //SET end column address=799
    TFT_7_Write_Data(0x1f);
    TFT_7_Write_Command(0x2b); //SET page address
    GPIO_SetBits(GPIOC, RS);
    TFT_7_Write_Data(0x00); //SET start page address=0
    TFT_7_Write_Data(0x00);
    TFT_7_Write_Data(0x01); //SET end page address=479
    TFT_7_Write_Data(0xdf);
    /*TFT_7_Write_Command(0x33); //SET scroll area
      GPIO_SetBits(GPIOC, RS);
      TFT_7_Write_Data(0x00); //SET top fixed area=0
      TFT_7_Write_Data(0x00);
      TFT_7_Write_Data(0x01); //SET vertical scrolling area=479
      TFT_7_Write_Data(0xdf);
      TFT_7_Write_Data(0x00); //SET bottom fixed area=0
      TFT_7_Write_Data(0x00);
      TFT_7_Write_Command(0x28); //SET display off */
}
void TFT_7_Write_Command(unsigned char command)
{
    GPIO_Write(GPIOB, command);
    GPIO_ResetBits(GPIOC, RS);
    GPIO_ResetBits(GPIOC, nWR);
    GPIO_SetBits(GPIOC, nWR);
}
void TFT_7_Write_Data(unsigned char data1)
{
    GPIO_SetBits(GPIOC, RS);
    GPIO_Write(GPIOB, data1);
    GPIO_ResetBits(GPIOC, nWR);
    GPIO_SetBits(GPIOC, nWR);
}
void TFT_7_Command_Write(unsigned char REG,unsigned char VALUE)
{
    TFT_7_Write_Command(REG);
    TFT_7_Write_Data(VALUE);
}
void TFT_7_SendData(unsigned long color) 
{
    GPIO_SetBits(GPIOC, RS);
    GPIO_Write(GPIOB, (color>>16));
    GPIO_ResetBits(GPIOC, nWR);
    GPIO_SetBits(GPIOC, nWR);
    GPIO_Write(GPIOB, (color>>8));
    GPIO_ResetBits(GPIOC, nWR);
    GPIO_SetBits(GPIOC, nWR);
    GPIO_Write(GPIOB, (color));
    GPIO_ResetBits(GPIOC, nWR);
    GPIO_SetBits(GPIOC, nWR);
}
void TFT_7_WindowSet(unsigned int s_x,unsigned int e_x,unsigned int s_y,unsigned int e_y)
{
    TFT_7_Write_Command(0x2a); //SET page address
    TFT_7_Write_Data((s_x)>>8); //SET start page address=0
    TFT_7_Write_Data(s_x);
    TFT_7_Write_Data((e_x)>>8); //SET end page address=639
    TFT_7_Write_Data(e_x);
    TFT_7_Write_Command(0x2b); //SET column address
    TFT_7_Write_Data((s_y)>>8); //SET start column address=0
    TFT_7_Write_Data(s_y);
    TFT_7_Write_Data((e_y)>>8); //SET end column address=479
    TFT_7_Write_Data(e_y);
} 
