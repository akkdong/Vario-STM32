// DefaultSettings.h
//

#ifndef __DEFAULTSETTINGS_H__
#define __DEFAULTSETTINGS_H__

// Serial baud rate
//

#define BAUDRATE_DEBUG		115200
#define BAUDRATE_BT			9600
#define BAUDRATE_GPS		9600

// Pin definitions
//

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


// Variometer default settings
//

#define VARIOMETER_TIME_ZONE					(9)		// GMT+9
#define VARIOMETER_BEEP_VOLUME					(100)	// percentage

#define VARIOMETER_SINKING_THRESHOLD 			(-3.0)
#define VARIOMETER_CLIMBING_THRESHOLD 			(0.2)
#define VARIOMETER_NEAR_CLIMBING_SENSITIVITY 	(0.5)

#define VARIOMETER_LXNAV_SENTENCE				(1)
#define VARIOMETER_LK8_SENTENCE					(2)
#define VARIOMETER_DEFAULT_NMEA_SENTENCE		VARIOMETER_LK8_SENTENCE

#define VARIOMETER_SENTENCE_DELAY				(1000)


// EEPROM(AT25C256) settings
//

#define EEPROM_TOTAL_SIZE						(32*1024)
#define EEPROM_PAGE_SIZE						(64)

#define EEPROM_ADDRESS							(0x50)	// 1 0 1 0 A2 A1 A0 R/W


// Kalman filter settings
//

#define POSITION_MEASURE_STANDARD_DEVIATION 	(0.1)
#define ACCELERATION_MEASURE_STANDARD_DEVIATION (0.3)


// Voltage measurement settings
//

#define ADC_SCALE_FACTOR						(1000.0 / (270.0 + 1000.0))	// voltage divisor : R1(270K), R2(1M)
#define	ADC_LPF_FACTOR							(0.2)

#define ADC_CH									ADC1
#define ADC_PIN									PIN_ADC_BATTERY		// PA0
#define ADC_VREF								(3.3)
#define ADC_RESOLUTION							((1 << 12) - 1)
#define ADC_TO_VOLTAGE(x)						((x) *  ADC_VREF / ADC_RESOLUTION / ADC_SCALE_FACTOR)

#define ADC_MEASURE_COUNT						(10)
#define ADC_MEASURE_INTERVAL					(2000)


// Function key settings
//

#define FKEY_MIN_DEBOUNCE_TIME					(10)	// ms
#define FKEY_MIN_SHORTKEY_TIME					(300)	// ms
#define FKEY_MIN_FIRE_TIME						(500)	// ms


#define FKEY_INPUT_ACTIVE						(0)			// LOW ACTIVE


// LEDFlasher settings
//

#define FLASHER_LED_ACTIVE						(0)			// LOW ACTIVE(LOW: Turn On)


// SD-Card settings
//

#define SDCARD_CHANNEL							(1)			// SPI1
#define SDCARD_CS								PIN_SD_CS	// SPI1_CS : PA4, SPI2_CS : PB12
#define SDCARD_CLOCK							(18)		// 18MHz


// Flight detection settings
//

#define FLIGHT_START_MIN_SPEED 					(5.0)		// 5Km/h
#define FLIGHT_LANDING_THRESHOLD 				(10000)		// 10s


// InvenSense settings
//

#define TARGET_STM32DUINO_VARIOMETER
#define MPU6050	// one of MPU6050, MPU6500, MPU9150, MPU9250


// Nmea parsing settings
//

#define MAX_NMEA_PARSER_BUFFER					(128)

#define MAX_IGC_SENTENCE						(38)	// B-sentence max size : include null termination
														// ex: B1602405407121N00249342WA0028000421 CR LF
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

