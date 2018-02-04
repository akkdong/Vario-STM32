// GlobalConfig.ino
//

#include <DefaultSettings.h>
#include <EEPROMDriver.h>
#include <GlobalConfig.h>


//
//

TwoWire Wire2(2, I2C_FAST_MODE);
EEPROMDriver eeprom(Wire2);
GlobalConfig Config(eeprom, EEPROM_ADDRESS);


//
//

void scan(TwoWire & HWire)
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


void setup_eeprom()
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
	buf[0] = 0x55;
	buf[1] = 0xAA;
	buf[2] = 1;
	buf[3] = 2;
	buf[4] = 3;
	
	Serial.println("write some data at 0x0000...");
	eeprom.writeBuffer(EEPROM_ADDRESS, 0, buf, 5);
	delay(1000);
	
	//
	buf[0] = 0xDE;
	buf[1] = 0xAD;
	buf[2] = 6;
	buf[3] = 7;
	buf[4] = 8;
	buf[5] = 9;
	
	Serial.println("write some data at 0x0008...");
	eeprom.writeBuffer(EEPROM_ADDRESS, 8, buf, 6);
	delay(1000);
	
	//
	memset(buf, 0xFF, sizeof(buf));

	//
	Serial.print("Read from 0x0000\r\n  ==> ");
	eeprom.readBuffer(EEPROM_ADDRESS, 0, buf, 5);
	for (int i = 0; i < 5; i++ )
	{
		Serial.print(buf[i], HEX);
		Serial.print(", ");
	}
	Serial.println("");	

	//
	Serial.print("Read from 0x0008\r\n  ==> ");
	eeprom.readBuffer(EEPROM_ADDRESS, 8, buf, 6);
	for (int i = 0; i < 6; i++ )
	{
		Serial.print(buf[i], HEX);
		Serial.print(", ");
	}
	Serial.println("");		
	
	return; 
	
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
		eeprom.writeBuffer(EEPROM_ADDRESS, 0, buf, EEPROM_PAGE_SIZE);
		
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
		eeprom.writeBuffer(EEPROM_ADDRESS, 0, buf, 16);
		
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


void setup_GlobalConfig()
{
	//
	delay(400);
	
	//
	Serial.begin(115200);
	while (! Serial);
	
	Serial.println("GlobaConfig Test!!!");
	
	//
	Wire2.begin();
	scan(Wire2);
	
	//
	Config.readAll();
	
	Serial.print("vario_volume = "), Serial.println((int)Config.volume/vario);
	Serial.print("vario_sinkThreshold = "), Serial.println(Config.vario/sinkThreshold);
	Serial.print("vario_climbThreshold = "), Serial.println(Config.vario/climbThreshold);
	Serial.print("vario_sensitivity = "), Serial.println(Config.vario/sensitivity);
	
	Serial.print("vario_sentence = "), Serial.println(Config.vario/sentence);
	
	Serial.print("kalman_sigmaP = "), Serial.println(Config.kalman/sigmaP);
	Serial.print("kalman_sigmaA = "), Serial.println(Config.kalman/sigmaA);
	
	Serial.print("accel[0] = "), Serial.println(Config.calData.accel[0]);
	Serial.print("accel[1] = "), Serial.println(Config.calData.accel[1]);
	Serial.print("accel[2] = "), Serial.println(Config.calData.accel[2]);
	
	if (Config.calData.accel[0] == 0.0 && Config.calData.accel[1] == 0.0 && Config.calData.accel[2] == 0.0)
	{
		Config.calData.accel[0] = 0.123;
		Config.calData.accel[1] = -0.3;
		Config.calData.accel[2] = 0.032;
		
		Serial.println("write calibration data to EEPROM...");
		Config.writeCalibrationData();

		// read-again
		Config.readAll();
	
		Serial.print("accel[0] = "), Serial.println(Config.calData.accel[0]);
		Serial.print("accel[1] = "), Serial.println(Config.calData.accel[1]);
		Serial.print("accel[2] = "), Serial.println(Config.calData.accel[2]);
	}
}

void setup()
{
	setup_GlobalConfig();
}

void loop()
{
}
