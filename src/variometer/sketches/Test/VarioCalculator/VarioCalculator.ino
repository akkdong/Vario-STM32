// VarioCalculator.ino
//

#include <DefaultSettings.h>
#include <I2CDevice.h>
#include <EEPROMDriver.h>
#include <GlobalConfig.h>

#include "VarioCalculator.h"


//
HardWire Wire1(1, I2C_FAST_MODE);
HardWire Wire2(2, I2C_FAST_MODE);

// set wire reference member to I2C1
HardWire & I2CDevice::Wire = Wire1;

// set unlock callback function
unlockCallback I2CDevice::cbUnlock = VarioCalculator::unlockI2C;

//
EEPROMDriver eeprom(Wire2);

//
GlobalConfig Config(eeprom, EEPROM_ADDRESS);

//
//VarioCalculator & vario = VarioCalculator::getInstance();
VarioCalculator Vario;

//
uint32_t lastTick;


//
//

void setup()
{
	//
	Serial.begin();
	while (! Serial);
	delay(1000);
	
	Serial.println("VarioCalculator Test....");
	
	//
	Wire1.begin();
	Wire2.begin();
	
	//
	Config.readAll();
	
	Serial.print("Accelerometer calibration data = [");
	Serial.print(Config.calData.accel[0], 10); Serial.print(", ");
	Serial.print(Config.calData.accel[1], 10); Serial.print(", ");
	Serial.print(Config.calData.accel[2], 10); Serial.println("]");
	Serial.println("");	
	
    // wait for ready
    Serial.println("\nSend any character to begin: ");
    while (Serial.available() && Serial.read()); // empty buffer
    while (!Serial.available());                 // wait for data
    while (Serial.available() && Serial.read()); // empty buffer again

	//
	Vario.begin(Config.kalman.sigmaP, Config.kalman.sigmaA/*, calibrateGyro = true*/);
	
	//
	lastTick = micros();
}


//
//

void loop()
{
	//
	Vario.update();
	
	if (Vario.available())
	{
		Serial.print(Vario.getVelocity()*100.0);
		//Serial.print(",");
		//Serial.print(Vario.getPressure());
		//Serial.print(",");
		//Serial.print(Vario.getAltitude());
		Serial.println("");
		
		Vario.flush();
		lastTick = millis();
	}
}
