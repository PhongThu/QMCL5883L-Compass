void LCD_Write(uint8_t address, uint8_t *data, int size);
void lcd_send_cmd(char cmd);
void lcd_send_data (char data);
void lcd_clear (void);
void lcd_put_cur(int row, int col);
void lcd_init (void);
void lcd_send_string (char *str);