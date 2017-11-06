// GlobalConfig.ino
//

#include <DefaultSettings.h>
#include <EEPROMDriver.h>
#include <GlobalConfig.h>


//
//

HardWire Wire2(2, I2C_FAST_MODE);
EEPROMDriver eeprom(Wire2);
GlobalConfig Config(eeprom, EEPROM_ADDRESS);


//
//

void scan(HardWire & HWire)
{
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for(address = 1; address < 127; address++) {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.

    HWire.beginTransmission(address);
    error = HWire.endTransmission();
    
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) 
        Serial.print("0");
      Serial.println(address, HEX);

      nDevices++;
    }
    else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) 
        Serial.print("0");
      Serial.println(address, HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found");
  else
    Serial.println("done");
}


//
//

unsigned char buf[EEPROM_PAGE_SIZE];


void setup()
{
	//pinMode(PB9, OUTPUT);
	//digitalWrite(PB9, HIGH);
	
	//
	delay(1000);
	
	//
	Serial.begin(115200);
	while (! Serial);
	
	Serial.println("EEPROM Test!!!");
	
	//
	Wire2.begin();
	scan(Wire2);
	
	//
	memset(buf, 0xFF, sizeof(buf));
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
		for (int i = 2; i < EEPROM_PAGE_SIZE; i++ )
			buf[i] = 0x10 + i;
		eeprom.writePage(EEPROM_ADDRESS, 0, buf, EEPROM_PAGE_SIZE);
		
		Serial.print("Write to 0x0000\r\n  ==> ");
		for (int i = 0; i < EEPROM_PAGE_SIZE; i++ )
		{
			Serial.print(buf[i], HEX);
			Serial.print(", ");
		}
		Serial.println("");	

		// wait write completion!!
		delay(1000);

		//
		memset(buf, 0xFF, sizeof(buf));
		eeprom.readBuffer(EEPROM_ADDRESS, 0, buf, EEPROM_PAGE_SIZE);
		
		Serial.print("Read back from 0x0000\r\n  ==> ");
		for (int i = 0; i < EEPROM_PAGE_SIZE; i++ )
		{
			Serial.print(buf[i], HEX);
			Serial.print(", ");
		}
		Serial.println("");
	}
	
	//
	if (buf[0] == 0x55 && buf[1] == 0xAA)
	{
		//
		buf[0] = 0xDE; buf[1] = 0xAD;
		for (int i = 2; i < EEPROM_PAGE_SIZE; i++ )
			buf[i] = 0x50 + i;
		eeprom.writePage(EEPROM_ADDRESS, 0, buf, 16);
		
		Serial.print("Write to 0x0000\r\n  ==> ");
		for (int i = 0; i < EEPROM_PAGE_SIZE; i++ )
		{
			Serial.print(buf[i], HEX);
			Serial.print(", ");
		}
		Serial.println("");		

		// wait write completion!!
		delay(4000);
		
		//
		memset(buf, 0xFF, sizeof(buf));
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
