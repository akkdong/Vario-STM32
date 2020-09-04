// SerialEx.h
//

#ifndef __SERIALEX_H__
#define __SERIALEX_H__

#include <HardwareSerial.h>


#define DEFINE_SERIALEX(name, n)            \
	SerialEx name(USART##n,                 \
				BOARD_USART##n##_TX_PIN,	\
				BOARD_USART##n##_RX_PIN)


struct usart_dev;

				
////////////////////////////////////////////////////////////////////////////////////
// class SerialEx

class SerialEx : public HardwareSerial
{
public:
	SerialEx(struct usart_dev * dev, uint8 tx_pin, uint8 rx_pin);
	
public:
	//
	int 				availableForWrite(void);
	virtual size_t 		write(uint8_t);
	
private:
	struct usart_dev *	usart_device;
};


extern SerialEx SerialEx1;
extern SerialEx SerialEx2;
extern SerialEx SerialEx3;


#endif // __SERIALEX_H__
