// IMUTest.ino
//

#if TEST_IMU

#include <DefaultSettings.h>
#include <I2CDevice.h>
#include <EEPROMDriver.h>
#include <GlobalConfig.h>
#include <KalmanVVelocity.h>
#include <IMU.h>

//
TwoWire Wire1(1, I2C_FAST_MODE);
TwoWire Wire2(2, I2C_FAST_MODE);

// set wire reference member to I2C1
TwoWire & I2CDevice::Wire = Wire1;

void dummy()
{
}

// set unlock callback function
unlockCallback I2CDevice::cbUnlock = dummy;

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
	
	Serial.print("Kalman filter sigmaP = "); Serial.println(Config.kalman.sigmaP);
	Serial.print("Kalman filter sigmaA = "); Serial.println(Config.kalman.sigmaA);
	Serial.print("Accelerometer calibration data = [");
	Serial.print(Config.calData.accel[0], 10); Serial.print(", ");
	Serial.print(Config.calData.accel[1], 10); Serial.print(", ");
	Serial.print(Config.calData.accel[2], 10); Serial.println("]");
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

#else // test IMUModule

#include <DefaultSettings.h>
#include <I2CDevice.h>
#include <EEPROMDriver.h>
#include <VertVelocity.h>
#include <IMUModule.h>
#include <GlobalConfig.h>

//
// declare I2C instance
//
// I2C1 is used by IMU : I2CDevice is proxy interface of I2C1
// I2C2 is used by EEPROM
//

TwoWire Wire1(1, I2C_FAST_MODE);
TwoWire Wire2(2, I2C_FAST_MODE);

// initialize some static member of I2CDevice(I2C1)
//

// set wire reference member to I2C1
TwoWire & I2CDevice::Wire = Wire1;

// set unlock callback function
unlockCallback I2CDevice::cbUnlock = SensorMS5611::unlockI2C;

// declare EEPROMDriver
EEPROMDriver eeprom(Wire2);

//
IMUModule imu;

//
VertVelocity vertVel;

//
GlobalConfig Config(eeprom, EEPROM_ADDRESS);


//
//
//

void setup()
{
	//
	Serial.begin();
	while (! Serial);
	delay(1000);
	
	//
	Wire1.begin();
	Wire2.begin();
	
	//
	Config.readAll();
	
	//
	imu.init();
	
	for (int i = 0; i < 100; i++)
	{
		while (! imu.dataReady());
		imu.updateData();
	}
	
	// initialize kalman filtered vertical velocity calculator
	vertVel.init(imu.getAltitude(), 
				imu.getVelocity(),
				Config.kalman.sigmaP, // POSITION_MEASURE_STANDARD_DEVIATION,
				Config.kalman.sigmaA, // ACCELERATION_MEASURE_STANDARD_DEVIATION,
				millis());

				
}

void loop()
{
	if (imu.dataReady())
	{
		//
		imu.updateData(/* &sensorReporter */);

		//
		vertVel.update(imu.getAltitude(), imu.getVelocity(), millis());

		//
		float velocity = vertVel.getVelocity();
		Serial.println(velocity * 100.0, 2);
	}		
}

#endif