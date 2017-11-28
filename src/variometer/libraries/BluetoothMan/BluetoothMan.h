// BluetoothMan.h
//

#ifndef __BLUETOOTHMAN_H__
#define __BLUETOOTHMAN_H__

#define BTMAN_BLOCK_NONE		(0)
#define BTMAN_BLOCK_NMEA		(1<<0)
#define BTMAN_BLOCK_SENSOR		(1<<1)
#define BTMAN_BLOCK_MASK		(BTMAN_BLOCK_NMEA|BTMAN_BLOCK_SENSOR)


class NmeaParserEx;
class VarioSentence;
class SensorReporter;
class ResponseSender;
class SerialEx;


/////////////////////////////////////////////////////////////////////////////
// class BluetoothMan

class BluetoothMan
{
public:
	BluetoothMan(SerialEx & serial, NmeaParserEx & nmea, VarioSentence & vario, SensorReporter & sensor, ResponseSender & resp);
	
public:
	void				update();
	
	int					available();
	int					read();
	
	void				blockNmeaSentence(uint8_t block);
	void				blockSensorData(uint8_t block);

private:
	void				writeVarioSentence();
	void				writeGPSSentence();
	void				writeSensorData();
	void				writeResponse();
	
private:
	//
	uint8_t				lockState;
	uint8_t				blockTransfer;	// block transfer of nmea or sensor
	
	//
	SerialEx &			serialBT;
	
	NmeaParserEx &		nmeaParser;
	VarioSentence &		varioSentence;
	SensorReporter &	sensorReporter;
	ResponseSender &	responseSender;
};

#endif // __BLUETOOTHMAN_H__
