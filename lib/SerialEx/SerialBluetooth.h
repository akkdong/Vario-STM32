// SerialBluetooth.h
//

#ifndef __SERIALBLUETOOTH_H__
#define __SERIALBLUETOOTH_H__

#include <SerialEx.h>

				
////////////////////////////////////////////////////////////////////////////////////
// class SerialBluetooth

class SerialBluetooth : public SerialEx
{
public:
	SerialBluetooth(struct usart_dev * dev, uint8 tx_pin, uint8 rx_pin);
	
public:
	//
	virtual int 		read(void);
	virtual size_t		writeEx(uint8_t);

	bool				isConnected();
	
private:
    //
    bool                connected;

	//
	int					state; // 0 ~ 6
	int					index; // 0 ~ 
};


////////////////////////////////////////////////////////////////////////////////////
//

inline bool SerialBluetooth::isConnected()
	{ return connected; }


////////////////////////////////////////////////////////////////////////////////////
//

extern SerialBluetooth	SerialBT;


#endif // __SERIALBLUETOOTH_H__
