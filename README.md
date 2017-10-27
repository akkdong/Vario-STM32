# Variomenter with STM32

## Variometer Features
* Calcualte Vertical Velocity : uses sensor fusion(pressure & accelleration)
* Vario sound
* Receive GPS data
* IGC Logging : save to SD-Card
* BT connectivity : support GPS & Vario NMEA setences
* Firmware update : support Windows application that updates using USB2Serial
* Function Key
	* Change sound level (mute, low, loud)
	* UMS On/Off
	* Calibration On/Off
* Accllerator calibration
* Configuration console
	* View & Update parameters
	* Interactive calibration
	* Support Window or Android applicatioin that uses USB2Serial or BT
* One key Power on/off
* Self Power off
* Low battery detection
* USB suppport : UMS(USB mass stroage), USB2Serial
	* UMS can be enabled by Function key or when power on while functin key is pressed.
	* USB2Serial supports Firmware update & Debugging

## Hardware specifications
* STM32F103CB Processor
* 10DOF IMU : MPU6050, HMC5883, MS5611
* 66ch GPS module : AKS6C
* BT : Bluetooth Specifcation 2.1 support, Calss2, SPP
* Li-po battery & Charger
* Push button On/Off controller
* SD card
* USB : UMS, USB to Serial, DFU
* EEPROM : 32KB I2C

## Device modes
* Vario : default mode
* UMS
* Calibration
* Configuration

## Vario modes
* Init
* Landing
* Flying
* Halt
