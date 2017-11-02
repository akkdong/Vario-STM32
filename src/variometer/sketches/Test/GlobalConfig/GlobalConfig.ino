// GlobalConfig.ino
//

#include <DefaultSettings.h>
#include <EEPROMDriver.h>
#include <GlobalConfig.h>

//
//

HardWire Wire2(2);
EEPROMDriver eeprom(Wire2);
GlobalConfig Config(eeprom, EEPROM_ADDRESS);


//
//

void setup()
{
	//
	delay(1000);
	
	//
	Serial.begin(115200);
	while (! Serial);
	
	//
	unsigned char buf[EEPROM_PAGE_SIZE];
	
	memse(buf, 0xFF, sizeof(buf));
	eeprom.readBuffer(EEPROM_ADDRESS, 0, buf, EEPROM_PAGE_SIZE);
	
	Serial.print("Read from 0x0000\r\n  ==> ");
	for (int i = 0; i < EEPROM_PAGE_SIZE; i++ )
	{
		Serial.print(buf[i], HEX);
		Serial.print(", ");
	}
	Serial.println("");
	
	//
	if (buf[0] == 0xFF && buf[1] == 0xFF)
	{
		//
		buf[0] = 0x55; buf[1] = 0xAA;
		for (int i = 3; i < EEPROM_PAGE_SIZE; i++ )
			buf[i] = 0x80 + i;
		eeprom.writePage(EEPROM_ADDRESS, 0, buf, EEPROM_PAGE_SIZE);
		
		Serial.print("Write to 0x0000\r\n  ==> ");
		for (int i = 0; i < EEPROM_PAGE_SIZE; i++ )
		{
			Serial.print(buf[i], HEX);
			Serial.print(", ");
		}
		Serial.println("");		

		//
		memse(buf, 0xFF, sizeof(buf));
		eeprom.readBuffer(EEPROM_ADDRESS, 0, buf, EEPROM_PAGE_SIZE);
		
		Serial.print("Read back from 0x0000\r\n  ==> ");
		for (int i = 0; i < EEPROM_PAGE_SIZE; i++ )
		{
			Serial.print(buf[i], HEX);
			Serial.print(", ");
		}
		Serial.println("");
	}
}

void loop()
{
}
