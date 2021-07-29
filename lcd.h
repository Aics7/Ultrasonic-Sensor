#ifndef LCD_H
#define LCD_H
void initialize_lcd(void);
void delay_ms(int delay);
void lcd_write_instruc(unsigned char instruction);
void lcd_write_char(unsigned char c);
void lcd_init(void);
void lcd_clear(void);
void lcd_goto(unsigned char column, unsigned char row);
void lcd_write_string(char *s);
#endif
