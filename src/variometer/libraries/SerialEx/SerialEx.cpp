// SerialEx.cpp
//

#include "SerialEx.h"

#include <libmaple/libmaple.h>
#include <libmaple/gpio.h>
#include <libmaple/timer.h>
#include <libmaple/usart.h>


////////////////////////////////////////////////////////////////////////////////////
// class SerialEx

SerialEx::SerialEx(struct usart_dev * dev, uint8 tx_pin, uint8 rx_pin)
	: HardwareSerial(dev, tx_pin, rx_pin), usart_device(dev)
{
}
	
int SerialEx::availableForWrite(void)
{
	return ! rb_is_full(this->usart_device->wb);
}

size_t SerialEx::write(uint8_t ch)
{
	return usart_tx(this->usart_device, &ch, 1);
}




////////////////////////////////////////////////////////////////////////////////////
//

DEFINE_SERIALEX(SerialEx1, 1);
DEFINE_SERIALEX(SerialEx2, 2);
DEFINE_SERIALEX(SerialEx3, 3);
