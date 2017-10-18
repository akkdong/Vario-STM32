// I2CDevice.cpp
//

#include <Arduino.h>
#include "I2CDevice.h"

///////////////////////////////////////////////////////////////////////////////////////////////
// class I2CDevice

// timeout variable
uint16_t I2CDevice::readTimeout = I2CDEV_DEFAULT_READ_TIMEOUT;

// lock status
volatile boolean I2CDevice::locked = false;



///////////////////////////////////////////////////////////////////////////////////////////////
// 

int8_t I2CDevice::readByte(uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint16_t timeout)
{
	return readBytes(devAddr, regAddr, 1, data, timeout);
}

int8_t I2CDevice::readWord(uint8_t devAddr, uint8_t regAddr, uint16_t *data, uint16_t timeout)
{
	return readWords(devAddr, regAddr, 1, data, timeout);
}

int8_t I2CDevice::readBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data, uint16_t timeout, boolean lock)
{
	//
	if (lock)
		lockDevice();
	
	//
#ifdef I2CDEV_SERIAL_DEBUG
	Serial.print("I2C (0x");
	Serial.print(devAddr, HEX);
	Serial.print(") reading ");
	Serial.print(length, DEC);
	Serial.print(" bytes from 0x");
	Serial.print(regAddr, HEX);
	Serial.print("...");
#endif

	//
    int8_t count = 0;
    uint32_t t1 = millis();
	
	// I2C/TWI subsystem uses internal buffer that breaks with large data requests
	// so if user requests more than BUFFER_LENGTH bytes, we have to do it in
	// smaller chunks instead of all at once
	for (uint8_t k = 0; k < length; k += min(length, BUFFER_LENGTH)) {
		Wire.beginTransmission(devAddr);
		Wire.write(regAddr);
		Wire.endTransmission();
		Wire.beginTransmission(devAddr);
		Wire.requestFrom(devAddr, (uint8_t)min(length - k, BUFFER_LENGTH));

		for (; Wire.available() && (timeout == 0 || millis() - t1 < timeout); count++) {
			data[count] = Wire.read();
			#ifdef I2CDEV_SERIAL_DEBUG
				Serial.print(data[count], HEX);
				if (count + 1 < length) Serial.print(" ");
			#endif
		}
	}
	
    // check for timeout
    if (timeout > 0 && millis() - t1 >= timeout && count < length) count = -1; // timeout
	
	
	//
	if (lock)
		unlockDevice();

	return count;
}

int8_t I2CDevice::readWords(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint16_t *data, uint16_t timeout, boolean lock)
{
	//
	if (lock)
		lockDevice();

    #ifdef I2CDEV_SERIAL_DEBUG
        Serial.print("I2C (0x");
        Serial.print(devAddr, HEX);
        Serial.print(") reading ");
        Serial.print(length, DEC);
        Serial.print(" words from 0x");
        Serial.print(regAddr, HEX);
        Serial.print("...");
    #endif
	
	//
    int8_t count = 0;
    uint32_t t1 = millis();

	// I2C/TWI subsystem uses internal buffer that breaks with large data requests
	// so if user requests more than BUFFER_LENGTH bytes, we have to do it in
	// smaller chunks instead of all at once
	for (uint8_t k = 0; k < length * 2; k += min(length * 2, BUFFER_LENGTH)) {
		Wire.beginTransmission(devAddr);
		Wire.write(regAddr);
		Wire.endTransmission();
		Wire.beginTransmission(devAddr);
		Wire.requestFrom(devAddr, (uint8_t)(length * 2)); // length=words, this wants bytes

		bool msb = true; // starts with MSB, then LSB
		for (; Wire.available() && count < length && (timeout == 0 || millis() - t1 < timeout);) {
			if (msb) {
				// first byte is bits 15-8 (MSb=15)
				data[count] = Wire.read() << 8;
			} else {
				// second byte is bits 7-0 (LSb=0)
				data[count] |= Wire.read();
				#ifdef I2CDEV_SERIAL_DEBUG
					Serial.print(data[count], HEX);
					if (count + 1 < length) Serial.print(" ");
				#endif
				count++;
			}
			msb = !msb;
		}

		Wire.endTransmission();
	}

    if (timeout > 0 && millis() - t1 >= timeout && count < length) count = -1; // timeout
	
	//
	if (lock)
		unlockDevice();

	return count;
}

bool I2CDevice::writeByte(uint8_t devAddr, uint8_t regAddr, uint8_t data)
{
	return writeBytes(devAddr, regAddr, 1, &data);
}

bool I2CDevice::writeWord(uint8_t devAddr, uint8_t regAddr, uint16_t data)
{
	return writeWords(devAddr, regAddr, 1, &data);
}

bool I2CDevice::writeBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data, boolean lock)
{
	//
	if (lock)
		lockDevice();
	
    #ifdef I2CDEV_SERIAL_DEBUG
        Serial.print("I2C (0x");
        Serial.print(devAddr, HEX);
        Serial.print(") writing ");
        Serial.print(length, DEC);
        Serial.print(" bytes to 0x");
        Serial.print(regAddr, HEX);
        Serial.print("...");
    #endif	
	
	//
	uint8_t status = 0;
	
	Wire.beginTransmission(devAddr);
	Wire.write((uint8_t) regAddr); // send address
	
    for (uint8_t i = 0; i < length; i++) {
		 Wire.write((uint8_t) data[i]);
    }
	
	status = Wire.endTransmission();
	
    #ifdef I2CDEV_SERIAL_DEBUG
        Serial.println(". Done.");
    #endif	
	
	//
	if (lock)
		unlockDevice();

	return status == 0;
}

bool I2CDevice::writeWords(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint16_t *data, boolean lock)
{
	//
	if (lock)
		lockDevice();

    #ifdef I2CDEV_SERIAL_DEBUG
        Serial.print("I2C (0x");
        Serial.print(devAddr, HEX);
        Serial.print(") writing ");
        Serial.print(length, DEC);
        Serial.print(" words to 0x");
        Serial.print(regAddr, HEX);
        Serial.print("...");
    #endif
	
	//
    uint8_t status = 0;	
	
	Wire.beginTransmission(devAddr);
	Wire.write(regAddr); // send address

    for (uint8_t i = 0; i < length * 2; i++) {
		Wire.write((uint8_t)(data[i] >> 8));    // send MSB
		Wire.write((uint8_t)data[i++]);         // send LSB
    }
	
	status = Wire.endTransmission();
	
    #ifdef I2CDEV_SERIAL_DEBUG
        Serial.println(". Done.");
    #endif
	
	//
	if (lock)
		unlockDevice();

	return status == 0;
}

void I2CDevice::lockDevice()
{
	locked = true;
}

void I2CDevice::unlockDevice()
{
	locked = false;
	
	if (cbUnlock)
		cbUnlock();	
}
