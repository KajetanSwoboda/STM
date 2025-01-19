/*
 * lcd.c
 *
 *  Created on: 10/06/2018
 *      Author: Olivier Van den Eede
 */

#include "lcd.h"
const uint8_t ROW_16[] = {0x00, 0x40, 0x10, 0x50};
const uint8_t ROW_20[] = {0x00, 0x40, 0x14, 0x54};
/************************************** Static declarations **************************************/

static void lcd_write_data(Lcd_HandleTypeDef * lcd, uint8_t data);
static void lcd_write_command(Lcd_HandleTypeDef * lcd, uint8_t command);
static void lcd_write(Lcd_HandleTypeDef * lcd, uint8_t data, uint8_t len);


/************************************** Function definitions **************************************/


Lcd_HandleTypeDef Lcd_create(
		Lcd_PortType port[], Lcd_PinType pin[],
		Lcd_PortType rs_port, Lcd_PinType rs_pin,
		Lcd_PortType en_port, Lcd_PinType en_pin, Lcd_ModeTypeDef mode)
{
	Lcd_HandleTypeDef lcd;

	lcd.mode = mode;

	lcd.en_pin = en_pin;
	lcd.en_port = en_port;

	lcd.rs_pin = rs_pin;
	lcd.rs_port = rs_port;

	lcd.data_pin = pin;
	lcd.data_port = port;

	Lcd_init(&lcd);

	return lcd;
}
///@brief Inicjalizacji wyświetlacza LCD.
///
///Ustawienie kursora, wyczyszczenie ekranu.
///
///@param[in] *lcd handler wyświetlacza
void Lcd_init(Lcd_HandleTypeDef * lcd)
{
	if(lcd->mode == LCD_4_BIT_MODE)
	{
			lcd_write_command(lcd, 0x33);
			lcd_write_command(lcd, 0x32);
			lcd_write_command(lcd, FUNCTION_SET | OPT_N);				// 4-bit mode
	}
	else
		lcd_write_command(lcd, FUNCTION_SET | OPT_DL | OPT_N);


	lcd_write_command(lcd, CLEAR_DISPLAY);						// Clear screen
	lcd_write_command(lcd, DISPLAY_ON_OFF_CONTROL | OPT_D);		// Lcd-on, cursor-off, no-blink
	lcd_write_command(lcd, ENTRY_MODE_SET | OPT_INC);			// Increment cursor
}
///@brief Wypisanie liczby zmiennoprzecinkowej w miejscu kursora.
///
///@@param[in] *lcd handler wyświetlacza.
///@param[in] number liczba float do wyświetlenia.
void Lcd_float(Lcd_HandleTypeDef * lcd, float number)
{
	char buffer[11];
	char buffer_out[11];
	sprintf(buffer, "%f", number);
	int period = 0;
	for(period = 0; period<sizeof(buffer); period++)
	{
		if(buffer[period] == '.')
			break;
	}
	strncpy(buffer_out, buffer, period + 4);
	Lcd_string(lcd, buffer_out);
}
///@brief Wypisanie liczby całkowitej w miejscu kursora.
///
///@param[in] *lcd handler wyświetlacza.
///@param[in] number liczba int do wyświetlenia.
void Lcd_int(Lcd_HandleTypeDef * lcd, int number)
{
	char buffer[11];
	sprintf(buffer, "%d", number);
	Lcd_string(lcd, buffer);
}
///@brief Wypisanie ciągu znaków w miejscu kursora.
///
///@param[in] *lcd handler wyświetlacza.
///@param[in] string Tablica znaków do wyświetlenia.
void Lcd_string(Lcd_HandleTypeDef * lcd, char * string)
{
	for(uint8_t i = 0; i < strlen(string); i++)
	{
		lcd_write_data(lcd, string[i]);
	}
}

///@brief Ustawienie kursora.
///
///@param[in] *lcd handler wyświetlacza.
///@param[in] row Indeks rzędu.
///@param[in] col Indeks kolumny.
void Lcd_cursor(Lcd_HandleTypeDef * lcd, uint8_t row, uint8_t col)
{
	#ifdef LCD20xN
	lcd_write_command(lcd, SET_DDRAM_ADDR + ROW_20[row] + col);
	#endif

	#ifdef LCD16xN
	lcd_write_command(lcd, SET_DDRAM_ADDR + ROW_16[row] + col);
	#endif
}

///@brief Procedura czyszczenia wyświetlacza LCD z danych.
///
///@param[in] *lcd: handler wyświetlacza
void Lcd_clear(Lcd_HandleTypeDef * lcd) {
	lcd_write_command(lcd, CLEAR_DISPLAY);
}

///@brief Procedura czyszczenia wyświetlacza LCD z danych.
///
///@param[in] *lcd: handler wyświetlacza
///@param[in] code wartość wypisywana na wyświetlaczu
///@param[in] bitmap macierz pozycji wartości na LCD
void Lcd_define_char(Lcd_HandleTypeDef * lcd, uint8_t code, uint8_t bitmap[]){
	lcd_write_command(lcd, SETCGRAM_ADDR + (code << 3));
	for(uint8_t i=0;i<8;++i){
		lcd_write_data(lcd, bitmap[i]);
	}

}


/************************************** Static function definition **************************************/

///@brief Procedura wpisywania komendy na LCD.
///
///@param[in] *lcd: handler wyświetlacza
///@param[in] command: wartość wypisywana na lcd
void lcd_write_command(Lcd_HandleTypeDef * lcd, uint8_t command)
{
	HAL_GPIO_WritePin(lcd->rs_port, lcd->rs_pin, LCD_COMMAND_REG);		// Write to command register

	if(lcd->mode == LCD_4_BIT_MODE)
	{
		lcd_write(lcd, (command >> 4), LCD_NIB);
		lcd_write(lcd, command & 0x0F, LCD_NIB);
	}
	else
	{
		lcd_write(lcd, command, LCD_BYTE);
	}

}

///@brief Procedura wpisywania wartości na LCD.
///
///@param[in] *lcd: handler wyświetlacza
///@param[in] data: wartość wypisywana na lcd
void lcd_write_data(Lcd_HandleTypeDef * lcd, uint8_t data)
{
	HAL_GPIO_WritePin(lcd->rs_port, lcd->rs_pin, LCD_DATA_REG);			// Write to data register

	if(lcd->mode == LCD_4_BIT_MODE)
	{
		lcd_write(lcd, data >> 4, LCD_NIB);
		lcd_write(lcd, data & 0x0F, LCD_NIB);
	}
	else
	{
		lcd_write(lcd, data, LCD_BYTE);
	}

}

///@brief Procedura wpisywania danych o odpowiedniej długości na LCD.
///
///@param[in] *lcd: handler wyświetlacza
///@param[in] data: wartość wypisywana na lcd
///@param[in] len: długość wiadomości
void lcd_write(Lcd_HandleTypeDef * lcd, uint8_t data, uint8_t len)
{
	for(uint8_t i = 0; i < len; i++)
	{
		HAL_GPIO_WritePin(lcd->data_port[i], lcd->data_pin[i], (data >> i) & 0x01);
	}

	HAL_GPIO_WritePin(lcd->en_port, lcd->en_pin, 1);
	DELAY(1);
	HAL_GPIO_WritePin(lcd->en_port, lcd->en_pin, 0); 		// Data receive on falling edge
}
