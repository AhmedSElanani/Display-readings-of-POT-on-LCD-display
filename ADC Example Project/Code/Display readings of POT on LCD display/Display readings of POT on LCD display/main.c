/*
 * Display readings of POT on LCD display.c
 *
 * Created: 25-Jun-20 5:53:49 PM
 * Author : Ahmed Salah
 */ 

#include "ADC Driver/ADC.h"
#include "LCD Driver/LCD_INIT.h"
#include <util/delay.h>


//	Application defined Constants
#define		Entry_Message			(u8*)"ADC Project .."
#define		First_Message			(u8*)"ADC0  ADC1  ADC2"
#define		Measuring_Unit			'v'

#define		Number_OF_ADCs			3

#define		ADC0_XPOS				1
#define		ADC1_XPOS				7
#define		ADC2_XPOS				13

#define		VREF					4.79
#define		MAX_WIDTH				1023



//	Global Variables
static volatile u8 Binary_Semaphore = 0;


//	Function Prototypes
void ADC_handler(void);
void LCD_handler(u16 Reading,u8 key);
u8* Convert_Integer_To_String(u16 integer_value);
u8* Convert_Float_To_String(float float_value);


//	Main Entry Point
int main(void)
{	
	//Initialize LCD
	LCD_Vid_4BitInit();
	
	//Initialize ADC
	ADC_Init();

	//Send message to the User
	LCD_Vid4BitSendString(Entry_Message);	
	
	_delay_ms(2000);
	LCD_VidClrLCD();
	_delay_ms(2000);

	//	Print first message to the User
	LCD_Vid4BitSendString(First_Message);
	
	//	Start First Conversion in Channel 0
	ADC_Start_Conversion();


    while (1) 
    {
		if (Binary_Semaphore)
		{
			ADC_handler();
			Binary_Semaphore = 0;
		}
		
    }
}




ISR(ADC_vect)
{
	Binary_Semaphore = 1;
}



void ADC_handler(void)
{
	//	Initialize the array of Available ADCs for the first time
	static volatile u8 Available_ADCs[] = {ADC0,ADC1,ADC2};
	static volatile u8 i = 0;
	
	//	Read the ADC and Call LCD Handler
	LCD_handler(ADC_Read() ,Available_ADCs[i++]);
	
	if (i == Number_OF_ADCs)			i = 0;		//Reset i

	ADC_Switch_SingleChannel(Available_ADCs[i]);
	ADC_Start_Conversion();
}



void LCD_handler(u16 Reading,u8 ADCx)
{
	//	Define LCD position for Each ADC reading
	static volatile u8 LCD_XPosition[] = {ADC0_XPOS, ADC1_XPOS, ADC2_XPOS};
	
	//	Define some variables needed for Function Logic
	static volatile u16 Count = 0xFC00;
	static volatile u8 Binary_Semaphore = 0;

	//	Update Count value 
	Count++;
	
	//	When the Counter Overflows
	if (!Count)
	{
		//	Update values
		Binary_Semaphore ^= 1;
		Count = 0xFC00;
		
		LCD_VidClrLCD();
		LCD_Vid4BitSendString(First_Message);
	}
	

	//	Move to LCD Position	
	LCD_Vid_GoToXY(LCD_XPosition[ADCx],2);

	if (Binary_Semaphore)
	{
		LCD_Vid4BitSendString((Convert_Integer_To_String(Reading)));
	} 
	else
	{
		LCD_Vid4BitSendString((Convert_Float_To_String(Reading*VREF/MAX_WIDTH)));
		LCD_Vid_4BitSendChar(Measuring_Unit);
	}
	
}




u8* Convert_Integer_To_String(u16 integer_value)
{
	static  u8 string_value[5] = {};
	
	for(u8 i=0, div =10 ;i<4 ; i++ )
	{
		string_value[3-i] = ((integer_value % 10) + 48) ;
		integer_value /= div ;
	}
	
	return string_value;
}



u8* Convert_Float_To_String(float float_value)
{
	static  u8 string_value[5] = {};

	string_value[0] = (((int)float_value % 10) + 48) ;
	string_value[1] = '.' ;
	string_value[2] = (((int)(float_value*10))% 10) + 48;
	string_value[3] = ((((int)(float_value * 100)) % 10) + 48) ;

	return string_value;
}