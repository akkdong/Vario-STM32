// BluetoothMan.h
//

#ifndef __BLUETOOTHMAN_H__
#define __BLUETOOTHMAN_H__


class NmeaParserEx;
class VarioSentence;
class SerialEx;


/////////////////////////////////////////////////////////////////////////////
// class BluetoothMan

class BluetoothMan
{
public:
	BluetoothMan(SerialEx & serial, NmeaParserEx & nmea, VarioSentence & vario);
	
public:
	void				update();
	
	int					available();
	int					read();

private:
	void				writeVarioSentence();
	void				writeGPSSentence();
	
private:
	//
	uint8				lockState;
	
	//
	SerialEx &			serialBT;
	NmeaParserEx &		nmeaParser;
	VarioSentence &		varioSentence;
};

#endif // __BLUETOOTHMAN_H__
