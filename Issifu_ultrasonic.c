//Embedded systems code to read distance from ultrasonic sensor and display on an LCD
#include "mkl25z4.h"
#include "lcd.h"
#include "ultrasonic.h"
#include <stdio.h>

int main()
{
	initialize_pit();
  	initialize_timer();
	init_pin_IC();
	init_Timer_IC();
	
	initialize_lcd();
	lcd_init();
	char distance[7]; 
	
	lcd_clear();
	lcd_goto(0,0);
	lcd_write_string("Distance: ");
	
	lcd_goto(0,1);	
	lcd_write_string("By Issifu");

	while (1)
  {
		sprintf(distance, "%d", get_distance());
		lcd_goto(10,0);
		lcd_write_string(distance);
		lcd_write_string("     ");
		delay_ms(100);
  }
}
