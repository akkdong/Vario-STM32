// BluetoothMan.h
//

#ifndef __BLUETOOTHMAN_H__
#define __BLUETOOTHMAN_H__

#if 0
#define BTMAN_BLOCK_NONE		(0)
#define BTMAN_BLOCK_NMEA		(1<<0)
#define BTMAN_BLOCK_SENSOR		(1<<1)
#define BTMAN_BLOCK_MASK		(BTMAN_BLOCK_NMEA|BTMAN_BLOCK_SENSOR)
#endif

class NmeaParserEx;
class VarioSentence;
class SensorReporter;
class ResponseStack;
class SerialEx;


/////////////////////////////////////////////////////////////////////////////
// class BluetoothMan

class BluetoothMan
{
public:
	BluetoothMan(SerialEx & serial, NmeaParserEx & nmea, VarioSentence & vario, SensorReporter & sensor, ResponseStack & resp);
	
public:
	void				update();
	
	int					available();
	int					read();
	
	#if 0
	void				blockNmeaSentence(uint8_t block);
	void				blockSensorData(uint8_t block);
	#endif

private:
	void				writeVarioSentence();
	void				writeGPSSentence();
	void				writeSensorData();
	void				writeResponse();
	
private:
	//
	uint8_t				lockState;
	#if 0
	uint8_t				blockTransfer;	// block transfer of nmea or sensor
	#endif
	
	//
	SerialEx &			SerialBT;
	
	NmeaParserEx &		nmeaParser;
	VarioSentence &		varioSentence;
	SensorReporter &	sensorReporter;
	ResponseStack &		responseStack;
};

#endif // __BLUETOOTHMAN_H__
