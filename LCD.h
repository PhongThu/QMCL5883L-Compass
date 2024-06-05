void LCD_Write(uint8_t address, uint8_t *data, int size);
void LCD_SendCmd(char cmd);
void LCD_SendData(char data);
void LCD_Clear(void);
void LCD_PutCur(int row, int col);
void LCD_Init(void);
void LCD_SendString(char *str);
