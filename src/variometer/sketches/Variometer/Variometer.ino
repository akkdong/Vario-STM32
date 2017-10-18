// Variometer.ino
//

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <I2CDevice.h>
#include <VarioSettings.h>
#include <VertVelocity.h>
#include <IMUModule.h>

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

void board_init()
{
	// Initialize Serials
	Serial.begin(BAUDRATE_DEBUG);  	// Serial(USB2Serial) : for debugging
	while (! Serial);
	
	// Initialize I2C
	Wire.begin();
	Wire.setClock(400000); // 400KHz
	
	I2CDevice::cbUnlock = SensorMS5611::UnlockI2C;
}

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

	
	// imu.update()
	// imu.get();
	// ...
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
