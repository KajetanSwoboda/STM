/*
 * lcd.h
 *
 *  Created on: 10/06/2018
 *      Author: Olivier Van den Eede
 */

#ifndef LCD_H_
#define LCD_H_

#include "stm32f7xx_hal.h"
#include "string.h"
#include "stdio.h"
#include "main.h"

// #define LCD20xN 		// For 20xN LCDs
#define LCD16xN			// For 16xN LCDs

// For row start addresses
extern const uint8_t ROW_16[];
extern const uint8_t ROW_20[];

/************************************** Command register **************************************/
#define CLEAR_DISPLAY 0x01

#define RETURN_HOME 0x02

#define ENTRY_MODE_SET 0x04
#define OPT_S	0x01					// Shift entire display to right
#define OPT_INC 0x02					// Cursor increment

#define DISPLAY_ON_OFF_CONTROL 0x08
#define OPT_D	0x04					// Turn on display
#define OPT_C	0x02					// Turn on cursor
#define OPT_B 	0x01					// Turn on cursor blink

#define CURSOR_DISPLAY_SHIFT 0x10		// Move and shift cursor
#define OPT_SC 0x08
#define OPT_RL 0x04

#define FUNCTION_SET 0x20
#define OPT_DL 0x10						// Set interface data length
#define OPT_N 0x08						// Set number of display lines
#define OPT_F 0x04						// Set alternate font
#define SETCGRAM_ADDR 0x040
#define SET_DDRAM_ADDR 0x80				// Set DDRAM address


/************************************** Helper macros **************************************/
#define DELAY(X) HAL_Delay(X)


/************************************** LCD defines **************************************/
#define LCD_NIB 4
#define LCD_BYTE 8
#define LCD_DATA_REG 1
#define LCD_COMMAND_REG 0


/************************************** LCD typedefs **************************************/
#define Lcd_PortType GPIO_TypeDef*
#define Lcd_PinType uint16_t

/// Tryby pracy wyświetlacza.
///
/// Pozwala na wybranie pracy w trybie 4 lub 8 bitowym.
typedef enum {
	LCD_4_BIT_MODE,
	LCD_8_BIT_MODE
} Lcd_ModeTypeDef;

/// Struktura opisująca wyświetlacz LCD
///
/// Zawiera informację o urzywanych pinach oraz trybie pracy.
/// @param data_port Port zawierający Piny Danych.
/// @param data_pin Ustawienie pinów danych wykorzystywanych do komunikacji z wyświetlaczem.
/// @param rs_port Port zawierający pin Resetu.
/// @param rs_pin Pin resetu.
/// @param en_port Port zawierający pin Enable.
/// @param en_pin Pin enable.
typedef struct {
	Lcd_PortType * data_port;
	Lcd_PinType * data_pin;

	Lcd_PortType rs_port;
	Lcd_PinType rs_pin;

	Lcd_PortType en_port;
	Lcd_PinType en_pin;

	Lcd_ModeTypeDef mode;

} Lcd_HandleTypeDef;


/************************************** Public functions **************************************/
///

///Inicjalizacja wyświetlacza.
///
///Ustawienie kursora, wyczyszczenie ekranu.
///@param *lcd handler wyświetlacza.
void Lcd_init(Lcd_HandleTypeDef * lcd);
///Wypisanie liczby zmiennoprzecinkowej w miejscu kursora.
///
///@param *lcd handler wyświetlacza.
///@param number liczba float do wyświetlenia.
void Lcd_float(Lcd_HandleTypeDef * lcd, float number);
///Wypisanie liczby całkowitej w miejscu kursora.
///
///@param *lcd handler wyświetlacza.
///@param number liczba int do wyświetlenia.
void Lcd_int(Lcd_HandleTypeDef * lcd, int number);
///Wypisanie ciągu znaków w miejscu kursora.
///
///@param *lcd handler wyświetlacza.
///@param string Tablica znaków do wyświetlenia.
void Lcd_string(Lcd_HandleTypeDef * lcd, char * string);
///Ustawienie kursora.
///
///@param *lcd handler wyświetlacza.
///@param row Indeks rzędu.
///@param col Indeks kolumny.
void Lcd_cursor(Lcd_HandleTypeDef * lcd, uint8_t row, uint8_t col);
///Inicjalizacja struktury wyświetlacza.
///
// UStawienie wykorzystywanych Pinów. Ustawienie trybu pracy.
///@see Lcd_HandleTypeDef.
Lcd_HandleTypeDef Lcd_create(
		Lcd_PortType port[], Lcd_PinType pin[],
		Lcd_PortType rs_port, Lcd_PinType rs_pin,
		Lcd_PortType en_port, Lcd_PinType en_pin, Lcd_ModeTypeDef mode);
void Lcd_define_char(Lcd_HandleTypeDef * lcd, uint8_t code, uint8_t bitmap[]);
///Wyczyszczenie wyświetlacza.
///
///@param *lcd handler wyświetlacza.
void Lcd_clear(Lcd_HandleTypeDef * lcd);

#endif /* LCD_H_ */
