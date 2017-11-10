// IMUTest.ino
//

#include <DefaultSettings.h>
#include <Arduino.h>
#include <Wire.h>
#include <I2CDevice.h>
#include <VertVelocity.h>
#include <IMUModule.h>
#include <GlobalConfig.h>
#include <EEPROMDriver.h>

//
HardWire Wire1(1, I2C_FAST_MODE);
HardWire Wire2(2, I2C_FAST_MODE);

// set wire reference member to I2C1
HardWire & I2CDevice::Wire = Wire1;

// set unlock callback function
UnlockCallback I2CDevice::cbUnlock = SensorMS5611::UnlockI2C;

// EEPROMDriver
EEPROMDriver eeprom(Wire2);

// global configuration
GlobalConfig Config(eeprom, EEPROM_ADDRESS);

//
VertVelocity vertVel;
IMUModule imu;



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

void setup()
{
	//
	delay(1000);
	
	//
	Serial.begin(115200);
	while (! Serial);
	
	Serial.println("IMU Test!!!");
	
	// scan
	Wire1.begin();
	Wire2.begin();
	
	scan(Wire1);
	scan(Wire2);
	
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
	imu.init();
	Serial.println("imu.init()");
	#if 1
	for (int i = 0; i < 100; i++)
	{
		while (! imu.dataReady());
		imu.updateData();
		//Serial.print("Altitude = "); Serial.println(imu.getAltitude());
	}
	#else
	while (! imu.dataReady());
	imu.updateData();
	#endif
	Serial.println("imu.dataReady()");
	
	// initialize kalman filtered vertical velocity calculator
	vertVel.init(imu.getAltitude(), 
				imu.getVelocity(),
				Config.kalman_sigmaP, // POSITION_MEASURE_STANDARD_DEVIATION,
				Config.kalman_sigmaA, // ACCELERATION_MEASURE_STANDARD_DEVIATION,
				millis());	
	Serial.println("vertVel.init()");
}

void loop()
{
	//
	if (imu.dataReady())
	{
		imu.updateData();
		vertVel.update(imu.getAltitude(), imu.getVelocity(), millis());
		
		//Serial.print(imu.getAltitude()); Serial.print(", ");
		//Serial.print(imu.getVelocity()); Serial.print(", ");
		
		Serial.print(vertVel.getVelocity() * 100.0); 
		//Serial.print(", ");
		//Serial.print(vertVel.getPosition());
		
		Serial.println("");
	}
}
