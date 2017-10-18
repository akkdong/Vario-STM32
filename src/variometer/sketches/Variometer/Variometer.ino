// Variometer.ino
//

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <I2CDevice.h>
#include <EEPROMDriver.h>
#include <VarioSettings.h>
#include <VertVelocity.h>
#include <IMUModule.h>

#include <SdFat.h>
#include <FreeStack.h>

#define BAUDRATE_DEBUG		115200
#define BAUDRATE_BT			9600
#define BAUDRATE_GPS		9600

/*
#define PIN_ADC_BATTERY		PA0		// adc
#define PIN_PWM_L			PA7		// output
#define PIN_SD_CS			PA4		// output, active low
#define PIN_USB_EN			PB0		// output
#define PIN_POWER_CTRL		PB1		// output
#define PIN_EE_CS			PB12	// output, active low
#define PIN_PWM_H			PA8		// output
#define PIN_FUNC_INPUT		PB5		// input
#define PIN_BT_EN			PB8		// output
#define PIN_GPS_EN			PB9		// output, active low
#define PIN_MCU_STATE		PC13	// output, active low
#define PIN_USB_DETECT		PC14	// input

struct BTN_STATE {
	uint8 pin;
	WiringPinMode mode;
};

BTN_STATE btn_state[] = 
{
//	{ PIN_ADC_BATTERY	, OUTPUT },
//	{ PIN_PWM_L			, OUTPUT },
	{ PIN_SD_CS			, OUTPUT },
	{ PIN_USB_EN		, OUTPUT },
	{ PIN_POWER_CTRL	, OUTPUT },
	{ PIN_EE_CS			, OUTPUT },
//	{ PIN_PWM_H			, OUTPUT },
	{ PIN_FUNC_INPUT	, INPUT },
	{ PIN_BT_EN			, OUTPUT },
	{ PIN_GPS_EN		, OUTPUT },
	{ PIN_MCU_STATE		, OUTPUT },
	{ PIN_USB_DETECT	, INPUT },
};


SPIClass		spi(2);
VertVelocity  	vertVel;


void board_init()
{
	// Initialize Serials
	Serial.begin(BAUDRATE_DEBUG);  	// Serial(USB2Serial) : for debugging
	//while (! Serial);
	
	Serial1.begin(BAUDRATE_BT); 	// Serial1(USART1) : for BT
	while (! Serial1);
	
	Serial2.begin(BAUDRATE_GPS);	// Serial2(USART2) : for GPS
	while (! Serial2);
	
	// Initialize I2C
	Wire.begin();
	Wire.setClock(400000); // 400KHz
	
	// Initialize SPI
	spi.setBitOrder(MSBFIRST); 			// Set the SPI_2 bit order
	spi.setDataMode(SPI_MODE0); 			// Set the  SPI_2 data mode 0
	spi.setClockDivider(SPI_CLOCK_DIV16);	// 72 / 16 = 4.5 MHz
  
	// Initialize GPIO
	for (int i = 0; i < sizeof(btn_state)/sizeof(btn_state[0]); i++)
	{
		pinMode(btn_state[i].pin, btn_state[i].mode);
	}
}
*/

//
//
//

InertialMeasurementUnit imu;

//
//
//

#define POSITION_MEASURE_STANDARD_DEVIATION 		(0.1)
#define ACCELERATION_MEASURE_STANDARD_DEVIATION 	(0.3)

VertVelocity  	vertVel;

//
//
//

HardWire Wire1(1);
HardWire Wire2(2);

// initialize some static member of I2CDevice(I2C1)
//

// set wire reference member to I2C1
HardWire & I2CDevice::Wire = Wire1;

// set unlock callback function
UnlockCallback I2CDevice::cbUnlock = SensorMS5611::UnlockI2C;

// declare EEPROMDriver(it's use I2C2)
EEPROMDriver	eeprom(Wire2);


//
// just for test
//

SdFat sd1(1);

const uint8_t SD1_CS = PA4;


//
//
//

void board_init()
{
	// Initialize Serials
	Serial.begin(BAUDRATE_DEBUG);  	// Serial(USB2Serial) : for debugging
	while (! Serial);
	
	// Initialize I2C
	Wire1.begin();
	Wire1.setClock(400000); // 400KHz
	
	Wire2.begin();
	Wire2.setClock(400000); // 400KHz
}

/*
GlobalConfig config;

struct eeprom_block
{
	unsigned short address;
	unsigned short mask;

	unsigned char length;
	unsigned char data[1];
};

typedef struct tagGlobalConfig
{
	char	name[];
	char 	pilot[];
	char	glider[];
	
	unsigned char timeZone; // GMT+9
	
	char	vario_volume;
	float	vario_sinkThreshold;
	float	vario_climbThreshold;
	float	vario_velocitySensitivity;
	
	vario_tone_table 
	
	float	kalman_sigmaP;
	float	kalman_sigmaA;
	
	float	accel[3]; // accel calibration data
	
} GlobalConfig;

//
// EEPROM block map
//

0x0000 0x1234 32
0x0020 0x3251 64
0x0040 0x6324 32
....

unsigned char eeprom_buf[MAX_EEPROM_BUF];

eeprom_block * block = (eeprom_block *)&eeprom_buf[0];

eeprom_read_block(block_map, block);
switch(block_type)
{
case vario_setting :
    eeprom_block_vario_setting * p = block;
	config.vario_volume = p->volmume;
	....
	break;
case xxxx :
	break;
}


eeprom_write(block(block_map, block);


*/

//
//
//

void setup()
{
	//
	board_init();

  
	// initialize imu module & measure first data
	imu.init();
	while (! imu.dataReady());
	
	// initialize kalman filtered vertical velocity calculator
	vertVel.init(imu.getAltitude(), 
				imu.getVelocity(),
				POSITION_MEASURE_STANDARD_DEVIATION,
				ACCELERATION_MEASURE_STANDARD_DEVIATION,
				millis());
	

	// SdFat test...
	Serial.print(F("FreeStack: "));
	Serial.println(FreeStack());

	// initialize the first card
	if (!sd1.begin(SD1_CS, SD_SCK_MHZ(18))) {
		Serial.println("sd1.initError(\"sd1:\");");
		return;
	}
	// create Dir1 on sd1 if it does not exist
	if (!sd1.exists("/Dir1")) {
		if (!sd1.mkdir("/Dir1")) {
			Serial.println("sd1.errorExit(\"sd1.mkdir\");");
			return;
		}
	}
	// list root directory on both cards
	Serial.println(F("------sd1 root-------"));
	sd1.ls();	
}

//
//
//

void loop()
{
	//
	if (imu.dataReady())
	{
		imu.updateData();
		vertVel.update(imu.getAltitude(), imu.getVelocity(), millis());
		
		//Serial.print(imu.getAltitude()); Serial.print(", ");
		//Serial.print(imu.getVelocity()); Serial.print(", ");
		Serial.print(vertVel.getVelocity());
		Serial.println("");
	}
	
	// mpu6050 normal test
	/*
	if (mpu6050.dataReady())
	{
		mpu6050.updateData();
		Serial.println(mpu6050.getVelocity());
	}
	*/
	
	// mpu6050 raw-data test : it's used by calibration
	/*
	double accel[3], uv[3], va[3];
	
	if (mpu6050.rawReady(accel, uv, va))
	{
		mpu6050.updateData();
		
		Serial.print(accel[0]); Serial.print(", ");
		Serial.print(accel[1]); Serial.print(", ");
		Serial.print(accel[2]); Serial.print(", ");
		
		Serial.print(uv[0]); Serial.print(", ");
		Serial.print(uv[1]); Serial.print(", ");
		Serial.print(uv[2]); Serial.print(", ");
		
		Serial.print(va[0]); Serial.print(", ");
		Serial.print(va[1]); Serial.print(", ");
		Serial.print(va[2]); 
		
		Serial.println("");
	}
	*/
	
	// ms5611 test
	/*
	double p, t, h;
	
	if (ms5611.dataReady())
	{
		ms5611.updateData();
		
		Serial.print(ms5611.getPressure()); Serial.print(", ");
		Serial.print(ms5611.getTemperature()); Serial.print(", ");
		Serial.print(ms5611.getAltitude()); 
		
		Serial.println("");		
	}
	*/
}
