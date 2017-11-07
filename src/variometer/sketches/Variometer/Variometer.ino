// Variometer.ino
//

#include <DefaultSettings.h>
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <I2CDevice.h>
#include <EEPROMDriver.h>
#include <VertVelocity.h>
#include <IMUModule.h>
#include <NmeaParserEx.h>
#include <FunctionKey.h>
#include <InputKey.h>
#include <OutputKey.h>
#include <GlobalConfig.h>
#include <ToneGenerator.h>
#include <TonePlayer.h>
#include <VarioBeeper.h>
#include <SerialEx.h>
#include <VarioSentence.h>
#include <BluetoothMan.h>
#include <IGCLogger.h>
#include <BatteryVoltage.h>
#include <CommandParser.h>
#include <FuncKeyParser.h>
#include <LEDFlasher.h>
#include <SensorReporter.h>

/* not use anymore
// PIN map
//

struct GPIO_PINMODE {
	uint8 			pin;
	WiringPinMode 	mode;
	uint8			state; // output default state : HIGH or LOW
};

GPIO_PINMODE gpio_mode[] = 
{
//	{ PIN_SD_CS			, OUTPUT, HIGH }, // PA4	output, active low
	{ PIN_BT_EN			, OUTPUT, HIGH }, // PB0	output, active low
	{ PIN_GPS_EN		, OUTPUT, HIGH }, // PB1	output, active low
//	{ PIN_USB_EN		, OUTPUT,  LOW }, // PB9	output,	active high
	{ PIN_KILL_PWR		, OUTPUT, HIGH }, // PB14	output, active low
	{ PIN_MCU_STATE		, OUTPUT, HIGH }, // PC13	output, active low(led on)
	{ PIN_MODE_SELECT   , INPUT,  HIGH }, // PC14    input, active HIGH
	{ PIN_FUNC_INPUT	, INPUT,   LOW }, // PB5	 input, active low
	{ PIN_USB_DETECT	, INPUT,  HIGH }, // PB8	 input, active high
	{ PIN_SHDN_INT		, INPUT,   LOW }, // PB15	 input, active low
};
*/

// Objects used by each mode
//
//	Common used objects
//	  Config
//	  ToneGenerator
//	  TonePlayer
//	  FunctionKey
//	  BatteryVoltage
//	  CommandParser(*)
//    LEDNotify(*)
//
//	Vario-mode
//	  IMUModule
//	  IGCLogger
//	  VarioBeeper
//	  TurnOff-Checker(*)
//
//  UMS-mode
//      
//	Calibration-mode
//	  IMUModule
//	
//	Configuration-mode
//	  IMUModule
//	  IGCLogger
//	
//	  
//	Commands
//    by Function Key
//      Mode change : UMS, Calibration(no-interactive calibration)
//      Volume change : Mute, Loud, Quiet
//      Reset
//      
//    by Serial (BT & USB)
//      Mode change : Vario -> Configuration
//      Read/Update configuration parameters
//    	...
//    	accelerometer calibration
//    	vario-tone table
//      Interactive calibration
//      Device monitoring : Sensor(IMU, GPS, ADC, ...) state & data
//      Vario Tone Simulation
//
//	Communication(Serial) Protocol description
//	  1. Device -> Console
//        - nmea sentences
//            SOF($),[SENTENCE],,[DATA1],[DATA2],....[DATAn],[CHKSUM]\r\n
//        - response
//	          SOF(*)[TAG]<,[DATA1],[DATA2],....[DATAn]>\r\n : response data
//            SOF(*)[OK|FAIL]\r\n  : response result
//            SOF(<),[{FILED:VALUE},...] : response parameters
//        - log or message
//            SOF(@)[STRING]\r\n
//    2. Console -> Device
//        - command
//            SOF(#)[CMD],[DATA]\r\n
//        - set parameter
//            SOF(>)[TYPE],[VALUE]\r\n
//
//
//
//
//
//
//

//  Packet from Configuration console
//    1. mode switch
//         "#SW,[1|2|3|4]"
//            1 : start interactive calibration
//            2 : start no-interactive calibration
//            3 : start UMS
//            4 : configuraiton(?)
//         response
//            "*OK\r\n"
//            "*FAIL\r\n"
//    2. device status
//         "#DS<,[0|1|2|3>\r\n"
//            0 : all (imu, sd-card, gps, voltage, ...), default
//            1 : imu
//            2 : sd-card
//            3 : gps
//            4 : voltage
//         response
//            "*STS,[DATA1],[DATA2],...,[DATAn]\r\n"
//            DATAx -> {TYPE:VALUE(V|F)}
//            ex) "*STS,IMU:V,SD:V,GPS:F\r\n"
//    2. sensor dump
//         "#DU,[bitmasks]"
//            0    : none (stop)
//            bit0 : accelerometer
//            bit1 : pressure
//            bit2 : temperature
//            bit3 : voltage
//            bit4 : gps
//            bit5 : v-velocity
//         response : send repeatedly
//            "*ACC,x,y,z\r\n"  
//            "*PRS,x\r\n"  
//            "*TEM,x\r\n"  
//            "*VOL,x\r\n"  
//    2.1. nmea sentence
//         "#NM,[0|1]"
//            0 : stop sending
//            1 : start sending
//         response
//            "*OK\r\n"
//            "*FAIL\r\n"
//	  3. vario tone test : configuration mode only(?)
//         "#TT,[0|1]"
//            0 : stop if is running
//            1 : start tone test
//    4. vario sound level
//         "#SL,[0|1|2]"
//            0 : mute
//            1 : quiet`
//            2 : loud
//         response
//            "*OK\r\n"
//            "*FAIL\r\n"
//    5. reset(restart)
//         "#RS<,[0|1]>\r\n"
//            0 : just reset
//            1 : write parameters to eeprom(in configuration mode)
//         response
//            (no response)
//    6. query parameter
//         "#QU,FIELD\r\n"
//            0   : all
//            1~n : each field
//         response
//            "*PARAM,[DATA1],[DATA2],...,[DATAn]\r\n"
//            DATAx -> {FIELD:VALUE}
//    7. update parameter
//         "#UD,FIELD,VALUE\r\n"
//            FIELD : ...
//            VALUE : BYTE, WORD, DWORD, FLOAT, STRING
//         response
//            "*OK\r\n"
//            "*FAIL\r\n"
//
//  Packet from Firmware downloader
//    DFU....


//	LED status
//	  IMU failed
//	  SDCard failed
//	  No-interactive Calibration
//	    measure
//		measure result (valid, invalid)
//		can calibrate(?)
//	  Firmware update
//	
//	Sound Notify
//	  Device ready
//	  Take-off
//	  Landing
//	  Calibration : start, calibration done, calibration failed, measure valid, measure invalid
//	  Command(key) acquire : replay input
//	  Command(key) done
//
//
// 
//

//
//
//

#define DEVICE_MODE_VARIO			(0)
#define DEVICE_MODE_UMS				(1)
#define DEVICE_MODE_CALIBRATION		(2)
#define DEVICE_MODE_CONFIGURATION	(3)

#define VARIO_MODE_INIT				(0)
#define VARIO_MODE_LANDING			(1)
#define VARIO_MODE_FLYING			(2)
#define VARIO_MODE_HALT				(3)

uint8_t deviceMode = DEVICE_MODE_VARIO;

uint8_t	varioMode = VARIO_MODE_INIT;
uint32_t flightTick;

void (* main_loop)(void) = 0;

void vario_loop();
void ums_loop();
void calibration_loop();
void configuration_loop();


//
// Kalman Filter based Vertical Velocity Calculator
//
//
//


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

VarioSentence varioNmea(VARIOMETER_DEFAULT_NMEA_SENTENCE);


//
//
//

SensorReporter sensorReporter;

//
//
//

BluetoothMan	btMan(SerialEx1, nmeaParser, varioNmea, sensorReporter);

//
// IGC Logger
//

IGCLogger logger;


//
// Digital & Analog Input/Output
//

// generic digital input
InputKey	keyMode;
InputKey	keyShutdown;
InputKey	keyUSB;
// functional input
FunctionKey	keyFunc;
// analog input
BatteryVoltage batVolt;

// generic digital output
OutputKey	keyPowerGPS;
OutputKey	keyPowerBT;
OutputKey	keyPowerDev;
// functional output
LEDFlasher  ledFlasher;


//
//
//

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

GlobalConfig	Config(eeprom, EEPROM_ADDRESS);


//
//
//

CommandStack	cmdStack;

CommandParser	cmdParser1(Serial, cmdStack); // USB serial parser
CommandParser	cmdParser2(Serial1, cmdStack); // BT serial parser
FuncKeyParser	keyParser(keyFunc, cmdStack, tonePlayer);


//
//
//

void board_init()
{
	// Initialize Serials
	Serial.begin();
	//Serial.begin(BAUDRATE_DEBUG);  	// Serial(USB2Serial) : for debugging
	//while (! Serial);
	
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
#if 0
	for (int i = 0; i < sizeof(gpio_mode)/sizeof(gpio_mode[0]); i++)
	{
		pinMode(gpio_mode[i].pin, gpio_mode[i].mode);
		
		if (gpio_mode[i].mode == OUTPUT)
			digitalWrite(gpio_mode[i].pin, gpio_mode[i].state);
	}
#else
	// input pins
	keyMode.begin(PIN_MODE_SELECT, ACTIVE_LOW);
	keyShutdown.begin(PIN_SHDN_INT, ACTIVE_LOW);
	keyUSB.begin(PIN_USB_DETECT, ACTIVE_HIGH);
	// function-key
	keyFunc.begin(PIN_FUNC_INPUT, ACTIVE_HIGH);
	// adc input
	batVolt.begin(PIN_ADC_BATTERY);	
	
	// output pins
	keyPowerGPS.begin(PIN_GPS_EN, ACTIVE_LOW, OUTPUT_HIGH);
	keyPowerBT.begin(PIN_BT_EN, ACTIVE_LOW, OUTPUT_HIGH);
	//keyPowerDev.begin(PIN_KILL_PWR, ACTIVE_LOW, OUPUT_HIGH);
	// 
	ledFlasher.begin(PIN_MCU_STATE);
#endif
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
	//batVolt.begin(PIN_ADC_BATTERY);
	
	//
	//keyFunc.begin(PIN_FUNC_INPUT);

	/*
	keyMode.begin(PIN_MODE_SELECT, ACTIVE_LOW); or keyMode.begin(GPIO_PINMODE gpio);
	keyShutdown.begin(PIN_SHDN_INT, ACTIVE_LOW);
	keyUSB.begin(PIN_USB_DETECT, ACTIVE_HIGH);
	
	keyPowerGPS.begin(PIN_GPS_EN, ACTIVE_LOW);
	keyPowerBT.begin(PIN_BT_EN, ACTIVE_LOW);
	keyPowerDev.begin(PIN_KILL_PWR, ACTIVE_LOW);
	*/

	//ledFlasher.begin(PIN_MCU_STATE);
	
	//
	/*
	keyPowerBT.enable();
	btMan.begin(&nmeaParser, &varioNmea, &sensorReporter);
	*/
	
	// ToneGenerator uses PIN_PWM_H(PA8 : Timer1, Channel1)
	toneGen.begin(PIN_PWM_H);
	
	//
	tonePlayer.setVolume(Config.vario_volume);
	tonePlayer.setMelody(&startTone[0], sizeof(startTone) / sizeof(startTone[0]), 1, 0);
	
	//
	varioMode = VARIO_MODE_LANDING;
	main_loop = vario_loop;
}


//
//
//

void loop()
{
	main_loop();
}


//
//
//

void vario_loop()
{
	//
	if (imu.dataReady())
	{
		imu.updateData();
		vertVel.update(imu.getAltitude(), imu.getVelocity(), millis());
		
		//Serial.print(imu.getAltitude()); Serial.print(", ");
		//Serial.print(imu.getVelocity()); Serial.print(", ");
		//Serial.print(vertVel.getVelocity());
		//Serial.println("");
		
		//
		//sensorReporter.setData(...);
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
	//if (btMan.available())
	//{
	//	//
	//	int c = btMan.read();
	//	
	//	//
	//	// execute(c);
	//}

	
	//
	cmdParser1.update();
	cmdParser2.update();
	keyParser.update();
	
	while(cmdStack.getSize())
	{
		Command cmd = cmdStack.dequeue();
		
		switch(cmd.code)
		{
		case CMD_MODE_SWITCH 	:
			// change current mode
			if (deviceMode != cmd.param)
			{
				switch (cmd.param)
				{
				case PARAM_SW_ICALIBRATION :
				case PARAM_SW_CALIBRATION  :
					break;
				case PARAM_SW_UMS          :
					break;
				}
			}
			break;
		case CMD_DEVICE_STATUS 	:
			break;
		case CMD_SENSOR_DUMP 	:
			btMan.blockSensorData(cmd.param);
			break;
		case CMD_NMEA_SENTENCE  :
			btMan.blockNmeaSentence(cmd.param);
			break;
		case CMD_TONE_TEST 		:
			break;
		case CMD_SOUND_LEVEL 	:
			switch (cmd.param)
			{
			case PARAM_LV_LOUD 	: 
				Serial.println("set volume to 90%");
				tonePlayer.setVolume(90);
				break;
			case PARAM_LV_QUIET :
				Serial.println("set volume to 10%");
				tonePlayer.setVolume(10);
				break;
			case PARAM_LV_MUTE	:
			default				: 
				Serial.println("set volume to mute");
				tonePlayer.setVolume(0);
				break;
			}
			break;
		case CMD_DEVICE_RESET 	:
			break;
		case CMD_QUERY_PARAM 	:
			break;
		case CMD_UPDATE_PARAM 	:
			break;
		}
	}

	
	//
	if (varioMode == VARIO_MODE_LANDING)
	{
		if (nmeaParser.getSpeed() > FLIGHT_START_MIN_SPEED)
		{
			// start logging & change mode
			logger.begin(nmeaParser.getDate());
			varioMode = VARIO_MODE_FLYING;
			
			// play take-off melody
			// ...
			
			//
			flightTick = millis();
		}
	}
	else if (varioMode == VARIO_MODE_FLYING)
	{
		if (nmeaParser.getSpeed() < FLIGHT_START_MIN_SPEED)
		{
			if ((millis() - flightTick) < FLIGHT_LANDING_THRESHOLD)
			{
				// stop logging & change mode
				logger.end();
				varioMode = VARIO_MODE_LANDING;
				
				// play landing melody
				// ...
			}
		}
		else
		{
			// reset flightTick
			flightTick = millis();
		}
	}

	// check logging state
	if (logger.isLogging())
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
	
	// MCU State : LED Blinking
	ledFlasher.update();
	
	// process key-input
	#if 0 // FuncKeyParser treats key-input internally
	keyFunc.update();

	if (keyFunc.fired())
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
		
		uint8_t value = keyFunc.getValue();
	}
	#endif
}


//
//
//

void ums_loop()
{
}


//
//
//

void calibration_loop()
{
}


//
//
//

void configuration_loop()
{
}
