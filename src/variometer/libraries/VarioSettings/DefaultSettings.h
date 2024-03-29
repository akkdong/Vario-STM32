// DefaultSettings.h
//

#ifndef __DEFAULTSETTINGS_H__
#define __DEFAULTSETTINGS_H__

//
//
//

// Hardware version

#define HW_VERSION_V1							(0x0100)
#define HW_VERSION_V1_REV2						(0x0102)

#define HW_VERSION								HW_VERSION_V1_REV2
#define KOBO_SUPPORT							(0)

// you can use one of KalmanVario and Variometer class
#define CLASS_KALMANVARIO						(1)
#define CLASS_VARIOMETER						(2)

#define VARIOMETER_CLASSS						CLASS_KALMANVARIO


// ADC has two choice. PRIMARY or ALTERNATE
#define ADC_PRIMARY								(1)
#define ADC_ALTERNATE							(2)

#define BATTERY_ADC								ADC_ALTERNATE


// Serial baud rate
//

#define BAUDRATE_DEBUG							(115200)
#if KOBO_SUPPORT
#define BAUDRATE_BT								(57600)
#else
#define BAUDRATE_BT								(57600)
#endif // KOBO_SUPPORT
#define BAUDRATE_GPS							(9600)


// Pin definitions
//

#if BATTERY_ADC == ADC_PRIMARY
#define PIN_ADC_BATTERY							PA0		// ADC
#else  // 
#define PIN_ADC_BATTERY							PA1		// ADC
#endif // BATTERY_ADC
#define PIN_EMPTY_1								PA1		// empty
#define PIN_USART2_TX							PA2		// USART2
#define PIN_USART2_RX							PA3		// USART2
#define PIN_SD_CS								PA4		// GPIO : output, active low
#define PIN_SD_SCLK								PA5		// SD
#define PIN_SD_MISO								PA6		// SD
#define PIN_SD_MOSI								PA7		// SD
#define PIN_PWM_H								PA8		// PWM
#define PIN_USART1_TX							PA9		// USART1
#define PIN_USART1_RX							PA10	// USART1
#define PIN_USB_DM								PA11	// USB
#define PIN_USB_DP								PA12	// USB
#define PIN_IMU_FSYNC							PA13	// GPIO : output, active high, Rev2 : JTAG -> MPU6050 FSYNC
#define PIN_IMU_INTA							PA14	// GPIO : input, active high, Rev2 : JTAG -> MPU6050 INT
#define PIN_IMU_DRDY							PA15	// GPIO : input, active low, Rev2 : JTAG -> HMC5883L DRDY
#define PIN_BT_EN								PB0		// GPIO : output, active low, Rev2 :  active high
#define PIN_GPS_EN								PB1		// GPIO : output, active low, Rev2 :  active high
#define PIN_BOOT1								PB2		// boot
#define PIN_FUNC_INPUT							PB5		// GPIO : input, active low
#define PIN_I2C1_SCL							PB6		// I2C1
#define PIN_I2C1_SDA							PB7		// I2C1
#define PIN_USB_DETECT							PB8		// GPIO : input, active high
#define PIN_USB_EN								PB9		// GPIO : output,	active high
#define PIN_I2C2_SCL							PB10	// I2C2
#define PIN_I2C2_SDA							PB11	// I2C2
#define PIN_IMU_EN								PB12	// GPIO : output,	active high, Rev2 only
#define PIN_SD_EN								PB13	// GPIO : output,	active high, Rev2 only
#define PIN_KILL_PWR							PB14	// GPIO : input, active low
#define PIN_SHDN_INT							PB15	// GPIO : input, active low
#define PIN_MCU_STATE							PC13	// GPIO : output, active low(led on), Not used

#define ACTIVE_LOW								(0)
#define ACTIVE_HIGH								(1)

#define INPUT_LOW								(0)		// input value according to gpio state
#define INPUT_HIGH								(1)		// ex) INPUT_HIGH <- HIGH, INPUT_LOW <- LOW

#define INPUT_INACTIVE							(0)		// input value according to active state
#define INPUT_ACTIVE							(1)		// ex) ACTIVE_LOW : INPUT_ACTIVE <- INPUT_LOW

#define OUTPUT_LOW								(0)		// output value
#define OUTPUT_HIGH								(1)		// ex) OUTPUT_HIGH -> HIGH, OUTPUT_LOW -> LOW

#define OUTPUT_INACTIVE							(0)		// output active-state value
#define OUTPUT_ACTIVE							(1)		// ex) ACTIVE_LOW : OUTPUT_ACTIVE -> OUTPUT_LOW


// Variometer default settings
//

#define VARIOMETER_TIME_ZONE					(9)		// GMT+9
#define VARIOMETER_BEEP_VOLUME					(80)	// percentage (0 ~ 100)
#define VARIOMETER_EFFECT_VOLUME				(5) 	//

#define VARIOMETER_SINKING_THRESHOLD 			(-3.0)
#define VARIOMETER_CLIMBING_THRESHOLD 			(0.2)
#define VARIOMETER_SENSITIVITY 					(0.18)
#define VARIOMETER_MIN_VELOCITY					(-10)
#define VARIOMETER_MAX_VELOCITY					(10)

#define VARIOMETER_LXNAV_SENTENCE				(1)
#define VARIOMETER_LK8_SENTENCE					(2)
#define VARIOMETER_DEFAULT_NMEA_SENTENCE		VARIOMETER_LK8_SENTENCE

#define VARIOMETER_SENTENCE_DELAY				(500)

#define VARIOMETER_DEFAULT_DAMPING_FACTOR 		(0.5)



// Auto shutdown threshold
//

#define STABLE_SINKING_THRESHOLD				(-0.2)
#define STABLE_CLIMBING_THRESHOLD				(0.2)
#define AUTO_SHUTDOWN_THRESHOLD					(10*60*1000)	// 10min (600s)


// Low battery threshold
//

#define LOW_BATTERY_THRESHOLD					(2.8)	// VBAT drop down about 0.4v. so rear threshold voltage is -0.4
#define SHUTDOWN_HOLD_TIME						(2000)


// EEPROM(AT25C256) settings
//

#define EEPROM_TOTAL_SIZE						(32*1024)
#define EEPROM_PAGE_SIZE						(64)

#define EEPROM_ADDRESS							(0x50)	// 1 0 1 0 A2 A1 A0 R/W


// Kalman filter settings
//

// Variometer class configuration
#define POSITION_MEASURE_STANDARD_DEVIATION 	(0.1)
#define ACCELERATION_MEASURE_STANDARD_DEVIATION (0.3)

// KalmanFilter class configuration
#define KF_ZMEAS_VARIANCE       				(400.0f)
#define KF_ZACCEL_VARIANCE      				(1000.0f)
#define KF_ACCELBIAS_VARIANCE   				(1.0f)


// Voltage measurement settings
//

#define ADC_SCALE_FACTOR						(1000.0 / (560.0 + 1000.0))	// voltage divisor : R1(560K), R2(1M)
#define	ADC_LPF_FACTOR							(0.2)

#if BATTERY_ADC == ADC_PRIMARY
#define ADC_CH									ADC0
#else
#define ADC_CH									ADC1
#endif // BATTERY_ADC
#define ADC_PIN									PIN_ADC_BATTERY		// PA0
#define ADC_VREF								(3.3)
#define ADC_RESOLUTION							((1 << 12) - 1)
#define ADC_TO_VOLTAGE(x)						((x) *  ADC_VREF / ADC_RESOLUTION / ADC_SCALE_FACTOR)

#define ADC_MEASURE_COUNT						(10)
#define ADC_MEASURE_INTERVAL					(2000)

#define ADC_BIAS_VOLTAGEDROP					(0.2)


// Function key settings
//

#define FKEY_MIN_DEBOUNCE_TIME					(10)	// ms
#define FKEY_MIN_SHORTKEY_TIME					(300)	// ms
#define FKEY_MIN_FIRE_TIME						(500)	// ms
#define FKEY_MIN_LONGLONGKEY_TIME				(2000)	// ms


#define FKEY_INPUT_ACTIVE						ACTIVE_LOW	// LOW ACTIVE


// LEDFlasher settings
//

#define FLASHER_LED_ACTIVE						(0)			// LOW ACTIVE(LOW: Turn On)


// SD-Card settings
//

#define SDCARD_SPICLASS							(SPI)
#define SDCARD_CHANNEL							(1)			// SPI1
#define SDCARD_CS								PIN_SD_CS	// SPI1_CS : PA4, SPI2_CS : PB12
#define SDCARD_CLOCK							(36)		// 18MHz


// Flight detection settings
//

#define FLIGHT_START_MIN_SPEED 					(8.0)		// 8Km/h
#define FLIGHT_LANDING_THRESHOLD 				(30000)		// 30s
#define FLIGHT_LOGGING_INTERVAL					(1000)		// 1s


// InvenSense settings
//

#define TARGET_STM32DUINO_VARIOMETER
#define MPU6050	// one of MPU6050, MPU6500, MPU9150, MPU9250


// Nmea parsing settings
//

#define MAX_NMEA_PARSER_BUFFER					(128)

#define MAX_IGC_SENTENCE						(37)	// B-sentence max size
														// ex: B1602405407121N00249342WA0028000421\r\n
														//     0123456789012345678901234567890123456789
														//     BHHMMSSDDMMmmmNDDDMMmmmWAPPPPPGGGGGCL
														//                   S        EV          RF

#define IGC_OFFSET_START						(0)
#define IGC_OFFSET_TIME							(1)
#define IGC_OFFSET_LATITUDE						(7)
#define IGC_OFFSET_LATITUDE_					(14)
#define IGC_OFFSET_LONGITUDE					(15)
#define IGC_OFFSET_LONGITUDE_					(23)
#define IGC_OFFSET_VALIDITY						(24)
#define IGC_OFFSET_PRESS_ALT					(25)
#define IGC_OFFSET_GPS_ALT						(30)
#define IGC_OFFSET_RETURN						(35)
#define IGC_OFFSET_NEWLINE						(36)
#define IGC_OFFSET_TERMINATE					(37)

#define IGC_SIZE_TIME							(6)
#define IGC_SIZE_LATITUDE						(7)
#define IGC_SIZE_LATITUDE_						(1)
#define IGC_SIZE_LONGITUDE						(8)
#define IGC_SIZE_LONGITUDE_						(1)
#define IGC_SIZE_VALIDITY						(1)
#define IGC_SIZE_PRESS_ALT						(5)
#define IGC_SIZE_GPS_ALT						(5)



#endif // __DEFAULTSETTINGS_H__

