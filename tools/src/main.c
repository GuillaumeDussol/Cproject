/**
 *  @file    main.c
 *  @author  Tomas Fryza, Brno University of Technology, Czechia
 *  @version V1.2
 *  @date    Oct 27, 2018
 *  @brief   Scan the TWI bus for all connected slave devices and transmit
 *           info to UART.
 */

/* Includes ------------------------------------------------------------------*/
#include "settings.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>         /* itoa() function */
#include "twi.h"
#include "lcd.h"

/* Constants and macros ------------------------------------------------------*/
/**
 *  @brief Define RTC clock I2C adress.
 */
#define SLAVE_ADDRESS_DS3231 0x68 // 0x57 or 0x68

/* Function prototypes -------------------------------------------------------*/
/**
*  @brief Initialize UART, TWI, and Timer/Counter1.
*/
void setup(void);

/**
*  @brief TWI Finite State Machine transmits all slave addresses.
*/
void fsm_menu(void);

/**
* @brief Read actual time value from RTC clock.
*/
void read_time(void);

/**
* @brief Read actual date value from RTC clock.
*/
void read_date(void);

/**
* @brief Write new time value to RTC clock.
*/
void write_time(void);

/**
* @brief Write new date value to RTC clock.
*/
void write_date(void);

/* Global variables ----------------------------------------------------------*/
/* Time structure */
struct Time {
   char seconds;
   char minutes;
   char hours;
};
/* Date structure */
struct Date {
    char day;
    char month;
    int year;
};
/* Actual time */
struct Time actual_time;
/* Actual date */
struct Date actual_date;
/* Counter */
struct Time counter;
/* Chronometer */
struct Time chrono;
/* Alarm */
struct Time alarm;
/* Menu index */
int menu_index;
/* New time */
struct Time new_time;
/* New date */
struct Date new_date;
/* Buttons state */
typedef enum {
    NONE = 0,
    UP,
    RIGHT,
    DOWN,
    LEFT,
    SELECT
} button;
/* Button pressed */
button button_pressed = NONE;

/* Functions -----------------------------------------------------------------*/
int main(void)
{
    /* Initializations */
    setup();

    /* Enables interrupts by setting the global interrupt mask */
    sei();

    /* Forever loop */
    while (1) {
        /* Cycle here, do nothing, and wait for an interrupt */
    }            //new_time.seconds = actual_time.seconds;
            //new_time.minutes = actual_time.minutes;
            //new_time.hours = actual_time.hours;

    return 0;
}

/*******************************************************************************
 * Function: setup()
 * Purpose:  Initialize UART, TWI, and Timer/Counter1.
 * Input:    None
 * Returns:  None
 ******************************************************************************/
void setup(void)
{
    /* LCD display */
        /* Initialize display and select type of cursor */
        lcd_init(LCD_DISP_ON);

        /* Clear display and set cursor to home position */
        lcd_clrscr();

	/* Analog to Digital Converter */
        /* register ADMUX: Set ADC voltage reference to AVcc with external capacitor,
                           select input channel ADC0 (PC0) */
        ADMUX |= _BV(REFS0);

        /* register ADCSRA: ADC Enable,
                            ADC Auto Trigger Enable,
                            ADC Interrupt Enable,
                            ADC Prescaler 128 => fadc = fcpu / 128 = 125 kHz */
        ADCSRA |= _BV(ADEN) | _BV(ADATE) | _BV(ADIE) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);

        /* register ADCSRB: Set ADC Auto Trigger Source to Timer/Counter1 overflow */
        ADCSRB |= _BV(ADTS2) | _BV(ADTS1);

    /* Initialize TWI */
    twi_init();

    /* Timer/Counter1 */
        /* Clock prescaler 64 => overflows every 262ms */
        TCCR1B |= _BV(CS11) | _BV(CS10);
        /* Overflow interrupt enable */
        TIMSK1 |= _BV(TOIE1);

    /* Initialize date and Time */
    actual_time.seconds = 0;
    actual_time.minutes = 0;
    actual_time.hours = 0;
    actual_date.day = 1;
    actual_date.month = 1;
    actual_date.year = 2000;
    new_time.seconds = 0;
    new_time.minutes = 0;
    new_time.hours = 12;
    new_date.day = 1;
    new_date.month = 1;
    new_date.year = 2000;

    /* Initialize menu index */
    menu_index = 300;

    //write_time();
}

/*******************************************************************************
 * Function: Read time value from RTC clock.
 * Purpose:  Update actual time value.
 ******************************************************************************/
 void read_time(void)
 {
     char twi_status;
     char value = 0;

     twi_status = twi_start((SLAVE_ADDRESS_DS3231<<1) + TWI_WRITE);
     if (twi_status == 0)
     {
         twi_write(0x00);    // Asking to read seconds
         twi_stop();

         twi_start((SLAVE_ADDRESS_DS3231<<1) + TWI_READ);
         value = twi_read_ack();    // Read seconds
         actual_time.seconds = ((value >> 4) * 10) + (value & 0x0F);

         value = twi_read_ack();    // Read minutes
         actual_time.minutes = ((value >> 4) * 10) + (value & 0x0F);

         value = twi_read_nack();   // Read hours
         actual_time.hours = ((value >> 4) * 10) + (value & 0x0F);

         twi_stop();
     }
 }

 /*******************************************************************************
  * Function: Read time value from RTC clock.
  * Purpose:  Update actual time value.
  ******************************************************************************/
  void write_time(void)
  {
      char twi_status;
      char value = 0;

      twi_status = twi_start((SLAVE_ADDRESS_DS3231<<1) + TWI_WRITE);
      if (twi_status == 0)
      {
          twi_write(0x00);      // Asking to write seconds
          twi_write(0);         // Write seconds

          value = ((new_time.minutes / 10) << 4) + (new_time.minutes % 10);
          twi_write(value);     // Write minutes

          value = ((new_time.hours / 10) << 4) + (new_time.hours % 10);
          twi_write(value);     // Write hours

          twi_stop();
      }
  }

 /*******************************************************************************
  * Function: Read date value from RTC clock.
  * Purpose:  Update actual date value.
  ******************************************************************************/
  void read_date(void)
  {
      char twi_status;
      char x = 0;

      twi_status = twi_start((SLAVE_ADDRESS_DS3231<<1) + TWI_WRITE);
      if (twi_status == 0)
      {
          /* TODO */
          twi_stop();
      }
  }

  /*******************************************************************************
   * Function: Analog to Digital conversion interrupt
   * Purpose:  Analog to Digital conversion is completed.
   ******************************************************************************/
ISR(ADC_vect)
{
    uint16_t value = 0;
    char lcd_string[5];

	/* Read 10-bit value from ADC */
    value = ADC;

	/* Detect which button is pressed */
			if (value <= 20)
				button_pressed = RIGHT;
	else 	if (value >= 82  && value <= 122 )
                button_pressed = UP;
	else 	if (value >= 226 && value <= 266 )
                button_pressed = DOWN;
	else 	if (value >= 383 && value <= 423 )
                button_pressed = LEFT;
	else 	if (value >= 631 && value <= 671 )
                button_pressed = SELECT;
	else 		button_pressed = NONE;
}

/*******************************************************************************
 * Function: Timer/Counter1 overflow interrupt
 * Purpose:  Update state of menu Finite State Machine.
 ******************************************************************************/
ISR(TIMER1_OVF_vect)
{
    fsm_menu();
}

/*******************************************************************************
 * Function: fsm_menu()
 * Purpose:  Manage display on LCD display.
 ******************************************************************************/
void fsm_menu(void)
{
    char value;
    char i;
    static char cursor;
    char disp_string[5];

    switch(menu_index)
    {
        case 100:                   // Read time
            read_time();
            menu_index = 300;
            break;

        case 110 :                  // Write time
            write_time();
            menu_index = 300;
            break;

        case 200 :                  // Read date
            //read_date();
            break;

        case 210 :                  // Write Date
            //write_date();
            actual_date.day = new_date.day;
            actual_date.month = new_date.month;
            actual_date.year = new_date.year;
            menu_index = 300;
            break;

        case 300 :                  // Date and time display
            lcd_clrscr();
            read_time();

            /* Display time */
	        lcd_gotoxy(0, 0);
            if (actual_time.hours < 10) lcd_puts("0");
            itoa(actual_time.hours, disp_string, 10);
            lcd_puts(disp_string);
            lcd_puts(":");
            if (actual_time.minutes < 10) lcd_puts("0");
            itoa(actual_time.minutes, disp_string, 10);
            lcd_puts(disp_string);
            lcd_puts(":");
            if (actual_time.seconds < 10) lcd_puts("0");
            itoa(actual_time.seconds, disp_string, 10);
            lcd_puts(disp_string);

            /* Display date */
            lcd_gotoxy(0, 1);
            if (actual_date.day < 10) lcd_puts("0");
            itoa(actual_date.day, disp_string, 10);
            lcd_puts(disp_string);
            lcd_puts("/");
            if (actual_date.month < 10) lcd_puts("0");
            itoa(actual_date.month, disp_string, 10);
            lcd_puts(disp_string);
            lcd_puts("/");
            itoa(actual_date.year, disp_string, 10);
            lcd_puts(disp_string);

            /* Check if a button is pressed */
            if((button_pressed == UP) || (button_pressed == DOWN))
                menu_index = 310;

            break;

        case 310 :                   // Menu - alarms
            lcd_clrscr();

            /* Display actual menu */
            lcd_gotoxy(0, 0);
            lcd_puts("Alarms");

            /* Check if a button is pressed */
            if(button_pressed == UP)
                menu_index = 350;
            else if (button_pressed == DOWN)
                menu_index = 320;
            else if (button_pressed == LEFT)
                menu_index = 300;
            break;

        case 320 :                   // Menu - Chronometer
            lcd_clrscr();

            /* Display actual menu */
            lcd_gotoxy(0, 0);
            lcd_puts("Chronometer");

            /* Check if a button is pressed */
            if(button_pressed == UP)
                menu_index = 310;
            else if (button_pressed == DOWN)
                menu_index = 330;
            else if (button_pressed == LEFT)
                menu_index = 300;
            break;

        case 330 :                   // Menu - Counter
            lcd_clrscr();

            /* Display actual menu */
            lcd_gotoxy(0, 0);
            lcd_puts("Counter");

            /* Check if a button is pressed */
            if(button_pressed == UP)
                menu_index = 320;
            else if (button_pressed == DOWN)
                menu_index = 340;
            else if (button_pressed == LEFT)
                menu_index = 300;
            break;

        case 340 :                   // Menu - Set time
            lcd_clrscr();

            /* Display actual menu */
            lcd_gotoxy(0, 0);
            lcd_puts("Set time");

            /* Check if a button is pressed */
            if(button_pressed == UP)
                menu_index = 330;
            else if (button_pressed == DOWN)
                menu_index = 350;
            else if (button_pressed == LEFT)
                menu_index = 300;
            else if (button_pressed == SELECT)
            {
                menu_index = 341;
                /* Init cursor and new time */
                cursor = 0;
                new_time.seconds = actual_time.seconds;
                new_time.minutes = actual_time.minutes;
                new_time.hours = actual_time.hours;
            }
            break;

        case 341 :                   // Set time - display values
            lcd_clrscr();

            /* Display new time */
            lcd_gotoxy(0, 0);
            itoa(new_time.hours, disp_string, 10);
            lcd_puts(disp_string);
            lcd_gotoxy(2, 0);
            lcd_puts(":");
            itoa(new_time.minutes, disp_string, 10);
            lcd_puts(disp_string);

            menu_index = 342;
            break;

        case 342 :                   // Set time - modify values
            /* Show and replace cursor at previous location */
            lcd_command(LCD_DISP_ON_CURSOR);
            lcd_gotoxy(cursor, 0);

            /* Check if a button is pressed */
            if (button_pressed == LEFT)
                cursor = (cursor + 5 - 1) % 5;
            else if (button_pressed == RIGHT)
                cursor = (cursor + 1) % 5;
            else if (button_pressed == UP)
            {
                if ((cursor==0) || (cursor==1)) // Hours
                {
                    new_time.hours = (new_time.hours + 1) % 24;
                    menu_index = 341;
                }
                else if ((cursor==3) || (cursor==4))
                {
                    new_time.minutes = (new_time.minutes + 1) % 60;
                    menu_index = 341;
                }
            }
            else if (button_pressed == DOWN)
            {
                if ((cursor==0) || (cursor==1)) // Minutes
                {
                    new_time.hours = (new_time.hours + 24 - 1) % 24;
                    menu_index = 341;
                }
                else if ((cursor==3) || (cursor==4))
                {
                    new_time.minutes = (new_time.minutes + 60 - 1) % 60;
                    menu_index = 341;
                }
            }
            else if (button_pressed == SELECT)
                menu_index = 110;
            break;

        case 350 :                   // Menu - Set date
            lcd_clrscr();

            /* Display actual menu */
            lcd_gotoxy(0, 0);
            lcd_puts("Set date");

            /* Check if a button is pressed */
            if(button_pressed == UP)
                menu_index = 340;
            else if (button_pressed == DOWN)
                menu_index = 310;
            else if (button_pressed == LEFT)
                menu_index = 300;
            else if (button_pressed == SELECT)
            {
                menu_index = 351;
                cursor = 0;
                new_date.day = actual_date.day;
                new_date.month = actual_date.month;
                new_date.year = actual_date.year;
            }
            break;

        case 351 :                   // Set date - display values
            lcd_clrscr();

            /* Display new date */
            lcd_gotoxy(0, 0);
            itoa(new_date.day, disp_string, 10);
            lcd_puts(disp_string);
            lcd_gotoxy(2, 0);
            lcd_puts("/");
            itoa(new_date.month, disp_string, 10);
            lcd_puts(disp_string);
            lcd_gotoxy(5, 0);
            lcd_puts("/");
            itoa(new_date.year, disp_string, 10);
            lcd_puts(disp_string);

            menu_index = 352;
            break;

        case 352 :                   // Set date - modify values
            /* Show and replace cursor at previous location */
            lcd_command(LCD_DISP_ON_CURSOR);
            lcd_gotoxy(cursor, 0);

            /* Check if a button is pressed */
            if (button_pressed == LEFT)
                cursor = (cursor + 10 - 1) % 10;
            else if (button_pressed == RIGHT)
                cursor = (cursor + 1) % 10;
            else if (button_pressed == UP)
            {
                if ((cursor==0) || (cursor==1)) // Day
                {
                    new_date.day++;

                    /* Check number max of days in current month */
                    if ((new_date.month == 4) || (new_date.month == 6) || (new_date.month == 9) || (new_date.month == 11))
                    {
                        if (new_date.day > 30)
                            new_date.day = 1;
                    }
                    else if ((new_date.month == 2))
                    {
                        if ((new_date.year % 4 == 0) && (new_date.day > 29))
                            new_date.day = 1;
                        else if (new_date.day > 28)
                            new_date.day = 1;
                    }
                    else
                    {
                        if (new_date.day > 31)
                            new_date.day = 1;
                    }
                    menu_index = 351;
                }
                else if ((cursor==3) || (cursor==4))    // Month
                {
                    new_date.month++;
                    if (new_date.month > 12)
                        new_date.month = 1;
                    menu_index = 351;
                }
                else if ((cursor > 5) && (cursor < 10)) // Year
                {
                    new_date.year++;
                    if (new_date.year > 2099)
                        new_date.year = 0;
                    menu_index = 351;
                }
            }
            else if (button_pressed == DOWN)
            {
                if ((cursor==0) || (cursor==1)) // Day
                {
                    new_date.day--;

                    /* Check number max of days in current month */
                    if (new_date.day < 1)
                    {
                        if ((new_date.month == 4) || (new_date.month == 6) || (new_date.month == 9) || (new_date.month == 11))
                            new_date.day = 30;
                        else if ((new_date.month == 2))
                        {
                            if (new_date.year % 4 == 0)
                                new_date.day = 29;
                            else if (new_date.day < 1)
                                new_date.day = 28;
                        }
                        else
                            new_date.day = 31;
                    }

                    menu_index = 351;
                }
                else if ((cursor==3) || (cursor==4)) // Month
                {
                    new_date.month--;
                    if (new_date.month < 1)
                        new_date.month = 12;
                    menu_index = 351;
                }
                else if ((cursor > 5) && (cursor < 10)) // Year
                {
                    new_date.year--;
                    if (new_date.year < 2000)
                        new_date.year = 2099;
                    menu_index = 351;
                }
            }
            else if (button_pressed == SELECT)
                menu_index = 210;

            break;

        default:
            menu_index = 300;
    } /* End of switch (twi_state) */
}

/* END OF FILE ****************************************************************/
