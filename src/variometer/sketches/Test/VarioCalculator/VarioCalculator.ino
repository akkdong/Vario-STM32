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
UnlockCallback I2CDevice::cbUnlock = VarioCalculator::unlockI2C_;

//
EEPROMDriver eeprom(Wire2);

//
GlobalConfig Config(eeprom, EEPROM_ADDRESS);

//
VarioCalculator & vario = VarioCalculator::getInstance();

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
	Serial.print(Config.accel_calData[0], 10); Serial.print(", ");
	Serial.print(Config.accel_calData[1], 10); Serial.print(", ");
	Serial.print(Config.accel_calData[2], 10); Serial.println("]");
	Serial.println("");	
	
    // wait for ready
    Serial.println("\nSend any character to begin: ");
    while (Serial.available() && Serial.read()); // empty buffer
    while (!Serial.available());                 // wait for data
    while (Serial.available() && Serial.read()); // empty buffer again

	//
	vario.begin(Config.kalman_sigmaP, Config.kalman_sigmaA);
	
	//
	lastTick = micros();
}


//
//

void loop()
{
	//
	vario.update();
	
	if (vario.available())
	{
		Serial.print(vario.getVelocity()*100.0);
		//Serial.print(",");
		//Serial.print(vario.getPressure());
		//Serial.print(",");
		//Serial.print(vario.getAltitude());
		Serial.println("");
		
		vario.flush();
		lastTick = millis();
	}
}
