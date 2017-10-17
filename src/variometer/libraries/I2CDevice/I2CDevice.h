// I2CDevice.h
//

#ifndef __I2CDEVICE_H__
#define __I2CDEVICE_H__

#include <Arduino.h>
#include <Wire.h>

// -----------------------------------------------------------------------------
// Arduino-style "Serial.print" debug constant (uncomment to enable)
// -----------------------------------------------------------------------------
//#define I2CDEV_SERIAL_DEBUG


// 1000ms default read timeout (modify with "I2CDevice::readTimeout = [ms];")
#define I2CDEV_DEFAULT_READ_TIMEOUT     1000

// unlock callback function
typedef void (* UnlockCallback)(void);


///////////////////////////////////////////////////////////////////////////////////////////////
// class I2CDevice

class I2CDevice
{
public:
//	static int8_t readBit(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t *data, uint16_t timeout=I2CDevice::readTimeout);
//	static int8_t readBitW(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint16_t *data, uint16_t timeout=I2CDevice::readTimeout);
//	static int8_t readBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t *data, uint16_t timeout=I2CDevice::readTimeout);
//	static int8_t readBitsW(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint16_t *data, uint16_t timeout=I2CDevice::readTimeout);
	static int8_t readByte(uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint16_t timeout=I2CDevice::readTimeout);
	static int8_t readWord(uint8_t devAddr, uint8_t regAddr, uint16_t *data, uint16_t timeout=I2CDevice::readTimeout);
	static int8_t readBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data, uint16_t timeout=I2CDevice::readTimeout, boolean lock = true);
	static int8_t readWords(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint16_t *data, uint16_t timeout=I2CDevice::readTimeout, boolean lock = true);

//	static bool writeBit(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t data);
//	static bool writeBitW(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint16_t data);
//	static bool writeBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data);
//	static bool writeBitsW(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint16_t data);
	static bool writeByte(uint8_t devAddr, uint8_t regAddr, uint8_t data);
	static bool writeWord(uint8_t devAddr, uint8_t regAddr, uint16_t data);
	static bool writeBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data, boolean lock = true);
	static bool writeWords(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint16_t *data, boolean lock = true);
	
	static void lockDevice();
	static void unlockDevice();

public:	
	static uint16_t 			readTimeout;
	static volatile boolean		locked;
	
	static UnlockCallback		cbUnlock;
};


///////////////////////////////////////////////////////////////////////////////////////////////
//

extern I2CDevice I2C;

#endif // __I2CDEVICE_H__
