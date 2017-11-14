// VarioCalculator.ino
//

#include <DefaultSettings.h>
#include <I2CDevice.h>
#include <EEPROMDriver.h>
#include <GlobalConfig.h>

#include <inv_mpu.h>
#include <inv_mpu_dmp_motion_driver.h>


/* accelerometer parameters */
#define MPU6050_G_TO_MS 		(9.80665)

#define MPU6050_GYRO_FSR 		(2000)
#define MPU6050_ACCEL_FSR 	(4)
#define MPU6050_FIFO_RATE 	(100)

/*  2G ~= 16384 2^14 */
/*  4G ~=  8192 2^13 */
/*  8G ~=  4096 2^12 */
/* 16G ~=  2048 2^11 */
#define MPU6050_ACCEL_SCALE 	(8192.0)
/*  250 degree/s ~= 131 */
/*  500 degree/s ~= 65.5 */
/* 1000 degree/s ~= 32.8 */
/* 2000 degree/s ~= 16.4 */
#define MPU6050_GYRO_SCALE	(16.4)


/* 2^30 */
#define MPU6050_QUAT_SCALE 	(1073741824.0)


HardWire Wire1(1, I2C_FAST_MODE);
HardWire Wire2(2, I2C_FAST_MODE);


// set wire reference member to I2C1
HardWire & I2CDevice::Wire = Wire1;

void dummy()
{
}

// set unlock callback function
UnlockCallback I2CDevice::cbUnlock = dummy;

//
EEPROMDriver eeprom(Wire2);

//
GlobalConfig Config(eeprom, EEPROM_ADDRESS);

//
uint8_t calibrateGyro = true;
uint32_t lastTick;

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

void setup()
{
	//
	Serial.begin();
	while (! Serial);
	delay(1000);
	
	Serial.println("InvenSense Test....");
	
	//
	Wire1.begin();
	Wire2.begin();
	
	scan(Wire1);
	scan(Wire2);
	
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

	
	// setting mpu
	mpu_init(NULL);
	mpu_set_sensors(INV_XYZ_GYRO|INV_XYZ_ACCEL); 
	mpu_set_gyro_fsr(MPU6050_GYRO_FSR);
	mpu_set_accel_fsr(MPU6050_ACCEL_FSR);
	mpu_configure_fifo(INV_XYZ_GYRO|INV_XYZ_ACCEL);

	// setting dmp
	dmp_load_motion_driver_firmware();
	dmp_set_fifo_rate(MPU6050_FIFO_RATE);
	mpu_set_dmp_state(1);
	if( calibrateGyro ) {
		dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT|/*DMP_FEATURE_SEND_RAW_GYRO|*/DMP_FEATURE_SEND_RAW_ACCEL|DMP_FEATURE_GYRO_CAL); 
	} else {
		dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT|DMP_FEATURE_SEND_RAW_GYRO|DMP_FEATURE_SEND_RAW_ACCEL);
	}
	
	//
	lastTick = micros();
}


//
//

void loop()
{
	short igyro[3], iaccel[3];
	long iquat[4];
	unsigned long timestamp;
	short sensors;
	unsigned char fifoCount;
	
	igyro[0] = igyro[1] = igyro[2] = 0;
	if (dmp_read_fifo(igyro,iaccel,iquat,&timestamp,&sensors,&fifoCount))
		return;
	
	//Serial.print("sensors = "); Serial.print(sensors);
	//Serial.print(", fifoCount = "); Serial.println(fifoCount);
	
	if (sensors)
	{
		//
		//Serial.println(micros() - lastTick);
		//lastTick = micros();
	
		#if 0
		Serial.print(igyro[0]/*/MPU6050_GYRO_SCALE*/); Serial.print(",");
		Serial.print(igyro[1]/*/MPU6050_GYRO_SCALE*/); Serial.print(",");
		Serial.print(igyro[2]/*/MPU6050_GYRO_SCALE*/); Serial.print(",");
		
		Serial.print(iaccel[0]/MPU6050_ACCEL_SCALE); Serial.print(",");
		Serial.print(iaccel[1]/MPU6050_ACCEL_SCALE); Serial.print(",");
		Serial.print(iaccel[2]/MPU6050_ACCEL_SCALE); Serial.print(",");
		
		Serial.print(iquat[0]/MPU6050_QUAT_SCALE); Serial.print(",");
		Serial.print(iquat[1]/MPU6050_QUAT_SCALE); Serial.print(",");
		Serial.print(iquat[2]/MPU6050_QUAT_SCALE); Serial.print(",");
		Serial.print(iquat[3]/MPU6050_QUAT_SCALE); 
		
		Serial.println("");
		#endif
		
		float accel[3];
		float quat[4]; 

		accel[0] = ((float)iaccel[0])/MPU6050_ACCEL_SCALE + Config.accel_calData[0];
		accel[1] = ((float)iaccel[1])/MPU6050_ACCEL_SCALE + Config.accel_calData[1];
		accel[2] = ((float)iaccel[2])/MPU6050_ACCEL_SCALE + Config.accel_calData[2];

		quat[0] = ((float)iquat[0])/MPU6050_QUAT_SCALE;
		quat[1] = ((float)iquat[1])/MPU6050_QUAT_SCALE;
		quat[2] = ((float)iquat[2])/MPU6050_QUAT_SCALE;
		quat[3] = ((float)iquat[3])/MPU6050_QUAT_SCALE;		
		
		float ux, uy, uz;
		ux = 2*(quat[1]*quat[3]-quat[0]*quat[2]);
		uy = 2*(quat[2]*quat[3]+quat[0]*quat[1]);
		uz = 2*(quat[0]*quat[0]+quat[3]*quat[3])-1;

		/* compute real acceleration (without gravity) */
		float rax, ray, raz;
		rax = accel[0] - ux;
		ray = accel[1] - uy;
		raz = accel[2] - uz;

		/* compute vertical acceleration */
		float uv[3];
		uv[0] = ux;
		uv[1] = uy;
		uv[2] = uz;
		
		float va = (ux*rax + uy*ray + uz*raz);	
		Serial.println(va);
	}
	else
	{
		Serial.println("");
		Serial.print("sensors = "); Serial.println(sensors);
		Serial.println("");
	}
}
