// IMUTest.ino
//

#include <DefaultSettings.h>
#include <I2CDevice.h>
#include <EEPROMDriver.h>
#include <GlobalConfig.h>
#include <KalmanVVelocity.h>
#include <IMU.h>

//
HardWire Wire1(1, I2C_FAST_MODE);
HardWire Wire2(2, I2C_FAST_MODE);

// set wire reference member to I2C1
HardWire & I2CDevice::Wire = Wire1;

void dummy()
{
}

// set unlock callback function
UnlockCallback I2CDevice::cbUnlock = dummy;

// EEPROMDriver
EEPROMDriver eeprom(Wire2);

// global configuration
GlobalConfig Config(eeprom, EEPROM_ADDRESS);

//
KalmanVVelocity vertVel;
IMU & imu = IMU::getInstance();

uint32_t lastTick;

void setup()
{
	//
	Serial.begin(115200);
	while (! Serial);
	delay(1000);
	
	Serial.println("IMU Test!!!");
	
	// scan
	Wire1.begin();
	Wire2.begin();
	
	//
	Config.readAll();
	
	Serial.print("Kalman filter sigmaP = "); Serial.println(Config.kalman_sigmaP);
	Serial.print("Kalman filter sigmaA = "); Serial.println(Config.kalman_sigmaA);
	Serial.print("Accelerometer calibration data = [");
	Serial.print(Config.accel_calData[0], 10); Serial.print(", ");
	Serial.print(Config.accel_calData[1], 10); Serial.print(", ");
	Serial.print(Config.accel_calData[2], 10); Serial.println("]");
	Serial.println("");
	
	// initialize imu module & measure first data
	imu.begin(&vertVel, true);
	
	Serial.println("a");
	//while (! vertVel.ready());
	lastTick = vertVel.getTimestamp();
	Serial.println("b");
}

void loop()
{
	if (lastTick != vertVel.getTimestamp())
	{
		lastTick = vertVel.getTimestamp();

		
		Serial.print(vertVel.getVelocity() * 100.0); 
		//Serial.print(", ");
		//Serial.print(vertVel.getPosition());
		
		Serial.println("");
	}
}
