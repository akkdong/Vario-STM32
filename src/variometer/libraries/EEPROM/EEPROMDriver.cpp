// EEPROMDriver.cpp
//

#include "EEPROMDriver.h"


///////////////////////////////////////////////////////////////////////////////////////////////
// class EEPROMDriver

EEPROMDriver::EEPROMDriver(HardWire & wire) : Wire(wire)
{
}

void EEPROMDriver::writeByte(unsigned char devAddr, unsigned short memAddr, unsigned char data)
{
	int rdata = data;
	
	Wire.beginTransmission(devAddr);
	Wire.write((int)(memAddr >> 8)); // MSB
	Wire.write((int)(memAddr & 0xFF)); // LSB
	Wire.write(rdata);
	Wire.endTransmission();
}

// WARNING: address is a page address, 6-bit end will wrap around
// also, data can be maximum of about 30 bytes, because the Wire library has a buffer of 32 bytes
void EEPROMDriver::writePage(unsigned char devAddr, unsigned short memAddr, unsigned char * data, short dataLen)
{
	Wire.beginTransmission(devAddr);
	Wire.write((int)(memAddr >> 8)); // MSB
	Wire.write((int)(memAddr & 0xFF)); // LSB
	
	for (short c = 0; c < dataLen; c++)
		Wire.write(data[c]);
	
	Wire.endTransmission();
}

unsigned char EEPROMDriver::readByte(unsigned char devAddr, unsigned short memAddr)
{
    unsigned char rdata = 0xFF;
	
    Wire.beginTransmission(devAddr);
    Wire.write((int)(memAddr >> 8)); // MSB
    Wire.write((int)(memAddr & 0xFF)); // LSB
    Wire.endTransmission();	
	
    Wire.requestFrom(devAddr, 1);
    if (Wire.available()) 
		rdata = Wire.read();
	
    return rdata;	
}

void EEPROMDriver::readBuffer(unsigned char devAddr, unsigned short memAddr, unsigned char * buf, short bufLen)
{
    Wire.beginTransmission(devAddr);
    Wire.write((int)(memAddr >> 8)); // MSB
    Wire.write((int)(memAddr & 0xFF)); // LSB
    Wire.endTransmission();
	
    Wire.requestFrom(devAddr,bufLen);
    for (short c = 0; c < bufLen; c++ )
        if (Wire.available()) buf[c] = Wire.read();
}
