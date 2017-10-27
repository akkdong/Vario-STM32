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
#include <NmeaParserEx.h>
#include <DigitalInput.h>
#include <GlobalConfig.h>
#include <ToneGenerator.h>
#include <TonePlayer.h>
#include <VarioBeeper.h>
#include <SerialEx.h>
#include <VarioSentence.h>
#include <BluetoothMan.h>
#include <IGCLogger.h>
#include <BatteryVoltage.h>

#define BAUDRATE_DEBUG		115200
#define BAUDRATE_BT			9600
#define BAUDRATE_GPS		9600

#define PIN_ADC_BATTERY		PA0		// ADC
#define PIN_EMPTY_1			PA1		// empty
#define PIN_USART2_TX		PA2		// USART2
#define PIN_USART2_RX		PA3		// USART2
#define PIN_SD_CS			PA4		// GPIO : output, active low
#define PIN_SD_SCLK			PA5		// SD
#define PIN_SD_MISO			PA6		// SD
#define PIN_SD_MOSI			PA7		// SD
#define PIN_PWM_H			PA8		// PWM
#define PIN_USART1_TX		PA9		// USART1
#define PIN_USART1_RX		PA10	// USART1
#define PIN_USB_DM			PA11	// USB
#define PIN_USB_DP			PA12	// USB
#define PIN_JTAG_JTMS		PA13	// JTAG
#define PIN_JTAG_JTCK		PA14	// JTAG
#define PIN_JTAG_JTDI		PA15	// JTAG
#define PIN_BT_EN			PB0		// GPIO : output, active low
#define PIN_GPS_EN			PB1		// GPIO : output, active low
#define PIN_BOOT1			PB2		// boot
#define PIN_JTAG_JTDO		PB3		// JTAG
#define PIN_JTAG_JNTRST		PB4		// JTAG
#define PIN_FUNC_INPUT		PB5		// GPIO : input, active low
#define PIN_I2C1_SCL		PB6		// I2C1
#define PIN_I2C1_SDA		PB7		// I2C1
#define PIN_USB_DETECT		PB8		// GPIO : input, active high
#define PIN_USB_EN			PB9		// GPIO : output,	active high
#define PIN_I2C2_SCL		PB10	// I2C2
#define PIN_I2C2_SDA		PB11	// I2C2
#define PIN_EMPTY_2			PB12	// empty
#define PIN_EMPTY_3			PB13	// empty
#define PIN_KILL_PWR		PB14	// GPIO : input, active low
#define PIN_SHDN_INT		PB15	// GPIO : input, active low
#define PIN_MCU_STATE		PC13	// GPIO : output, active low(led on)
#define PIN_MODE_SELECT		PC14	// GPIO : input, HIGH : UMS, LOW : DBG


struct GPIO_PINMODE {
	uint8 			pin;
	WiringPinMode 	mode;
	uint8			state; // output default state : HIGH or LOW
};

GPIO_PINMODE gpio_mode[] = 
{
	{ PIN_SD_CS			, OUTPUT, HIGH }, // PA4	output, active low
	{ PIN_BT_EN			, OUTPUT, HIGH }, // PB0	output, active low
	{ PIN_GPS_EN		, OUTPUT, HIGH }, // PB1	output, active low
	{ PIN_FUNC_INPUT	, INPUT,  LOW  }, // PB5	input, active low
	{ PIN_USB_DETECT	, INPUT,  HIGH }, // PB8	input, active high
	{ PIN_USB_EN		, OUTPUT, LOW  }, // PB9	output,	active high
	{ PIN_KILL_PWR		, INPUT,  LOW  }, // PB14	input, active low
	{ PIN_SHDN_INT		, INPUT,  LOW  }, // PB15	input, active low
	{ PIN_MCU_STATE		, OUTPUT, HIGH }, // PC13	output, active low(led on)
};



//
// Kalman Filter based Vertical Velocity Calculator
//
//
//

#define POSITION_MEASURE_STANDARD_DEVIATION 		(0.1)
#define ACCELERATION_MEASURE_STANDARD_DEVIATION 	(0.3)

VertVelocity  	vertVel;

//
// InertialMeasurementUnit Module : measure accelerometer & gyro & do calibration for accelerometer
//    It internally uses I2CDevice 
//

IMUModule imu;



//
// declare I2C instance
//
// I2C1 is used by IMU : I2CDevice is proxy interface of I2C1
// I2C2 is used by EEPROM
//

HardWire Wire1(1);
HardWire Wire2(2);

// initialize some static member of I2CDevice(I2C1)
//

// set wire reference member to I2C1
HardWire & I2CDevice::Wire = Wire1;

// set unlock callback function
UnlockCallback I2CDevice::cbUnlock = SensorMS5611::UnlockI2C;

// declare EEPROMDriver
EEPROMDriver	eeprom(Wire2);


//
// declare Serial relative instance
//
// BT uses Serial1
// GPS uses Serial2

NmeaParserEx nmeaParser(SerialEx2);


//
//
//

VarioSentence varioNmea(USE_LK8_SENTENCE);


//
//
//

BluetoothMan	btMan(SerialEx1, nmeaParser, varioNmea);

//
// IGC Logger
//

IGCLogger logger;


//
//
//

DigitalInput	funcInput;

//
//
//

BatteryVoltage batVolt;


//
//
//

#define PLAYER_TIMER_ID		(1)
#define PLAYER_TIMER_CH		(1)

static Tone startTone[] = {
	{ 262, 1000 / 4 }, 
	{ 196, 1000 / 8 }, 
	{ 196, 1000 / 8 }, 
	{ 220, 1000 / 4 }, 
	{ 196, 1000 / 4 }, 
	{   0, 1000 / 4 }, 
	{ 247, 1000 / 4 }, 
	{ 262, 1000 / 4 },
	{   0, 1000 / 8 }, 
};

ToneGenerator toneGen;
TonePlayer	tonePlayer(toneGen);
VarioBeeper	varioBeeper(tonePlayer);


//
//
//

#define EEPROM_TOTAL_SIZE		(64*1024)
#define EEPROM_PAGE_SIZE		(16)

#define EEPROM_ADDRESS			(0x50)


GlobalConfig	Config(eeprom, EEPROM_ADDRESS);


//
//
//

void board_init()
{
	// Initialize Serials
	Serial.begin(BAUDRATE_DEBUG);  	// Serial(USB2Serial) : for debugging
	while (! Serial);
	
	Serial1.begin(BAUDRATE_BT); 	// Serial1(USART1) : for BT
	while (! Serial1);
	
	Serial2.begin(BAUDRATE_GPS);	// Serial2(USART2) : for GPS
	while (! Serial2);
	
	// Initialize I2C
	Wire1.begin();
	Wire1.setClock(400000); // 400KHz
	
	Wire2.begin();
	Wire2.setClock(400000); // 400KHz
	
	// Initialize GPIO
	for (int i = 0; i < sizeof(gpio_mode)/sizeof(gpio_mode[0]); i++)
	{
		pinMode(gpio_mode[i].pin, gpio_mode[i].mode);
		
		if (gpio_mode[i].mode == OUTPUT)
			digitalWrite(gpio_mode[i].pin, gpio_mode[i].state);
	}
}



//
//
//

void setup()
{
	//
	board_init();
	
	//
	Config.readAll();

  
	// initialize imu module & measure first data
	imu.init();
	while (! imu.dataReady());
	
	// initialize kalman filtered vertical velocity calculator
	vertVel.init(imu.getAltitude(), 
				imu.getVelocity(),
				Config.kalman_sigmaP, // POSITION_MEASURE_STANDARD_DEVIATION,
				Config.kalman_sigmaA, // ACCELERATION_MEASURE_STANDARD_DEVIATION,
				millis());


	
	// Initialize IGC Logger
	logger.init();
	
	//
	batVolt.begin(PIN_ADC_BATTERY);
	
	//
	funcInput.begin(PIN_FUNC_INPUT);
	
	// ToneGenerator uses PIN_PWM_H(PA8 : Timer1, Channel1)
	toneGen.begin(PIN_PWM_H);
	
	//
	tonePlayer.setVolume(Config.vario_volume);
	tonePlayer.setMelody(&startTone[0], sizeof(startTone) / sizeof(startTone[0]), 1, 0);	
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
		
		//
		varioBeeper.setVelocity(vertVel.getVelocity());
		//
		logger.update(vertVel.getPosition());
	}
	
	// read & prase gps sentence
	nmeaParser.update();
	// update vario sentence periodically
	if (varioNmea.checkInterval())
		varioNmea.begin(vertVel.getPosition(), vertVel.getVelocity(), imu.getTemperature(), batVolt.getVoltage());
	// send any prepared sentence to BT
	btMan.update();
	
	// we can change global configuration by BT communication
	//       and execute some command also
	if (btMan.available())
	{
		//
		int c = btMan.read();
		
		//
		// execute(c);
	}

	// check logging state
	if (! logger.isLogging())
	{
		// if is not, check start condition
		//
		// ....
		// logger.begin(nmeaParser.getDate());
	}
	else
	{
		// nmeaParser parses GPS sentence and converts it to IGC sentence
		
		//static unsigned long tick = millis();
		//static int index = 0;
		
		//if ((millis()-tick) > time_interval)
		{		
			while (nmeaParser.availableIGC())
				logger.write(nmeaParser.readIGC());
		}
	}
	
	// beep beep beep!
	tonePlayer.update();
	
	// start voltage measurement periodically
	batVolt.update();
	
	// process key-input
	funcInput.update();

	if (funcInput.fired())
	{
		// value format :
		// b7 b6 b5 b4 b3 b2 b1 b0
		// C2 C1 C0 I4 I3 I2 I1 I0
		//
		// Cx : count of bits (valid: 1~5, forbidden(ignore): 0, 6, 7)
		// Ix : input value
		//      each bit represents LONG(1) or SHORT(0) input, default is SHORT
		//		MSB first, RIGHT aligned
		//
		
		uint8_t value = funcInput.getValue();
	}
}
