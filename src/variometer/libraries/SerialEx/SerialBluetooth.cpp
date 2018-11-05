// SerialBluetooth.cpp
//

#include "SerialBluetooth.h"

#include <usb_serial.h>
#include <libmaple/libmaple.h>
#include <libmaple/gpio.h>
#include <libmaple/timer.h>
#include <libmaple/usart.h>


////////////////////////////////////////////////////////////////////////////////////
//

const char * response_connect = "CONNECT "; // \xFF --> any character except line-break
const char * response_disconnect = "DISCONNECT";


////////////////////////////////////////////////////////////////////////////////////
// class SerialBluetooth

SerialBluetooth::SerialBluetooth(struct usart_dev * dev, uint8 tx_pin, uint8 rx_pin)
	: SerialEx(dev, tx_pin, rx_pin), connected(false), state(0), index(0)
{
}

int SerialBluetooth::read(void)
{
    int c = SerialEx::read();
    //Serial.write(c);

    // 0 : ready, 
    // 1 : leading linebreak
    // 2 : command parsing (C or D)
    // 3 : connecting
    // 4 : disconnecting
    // 5 : connect-trailing
    // 6 : disconnect-trailing
    if (state == 0)
    {
        if (c == '\r')
        {
            state = 1; // leading linebreak
        }
    }
    else if (state == 1)
    {
        if (c == '\n')
        {
            state = 2; // parse next character: CONNECT or DISCONNECT ?
        }
        else
        {
            state = 0; // reset
        }
    }
    else if (state == 2)
    {
        if (c == '\r')
        {
            state = 1; // go-back
        }
        else if (c == 'C')
        {
            state = 3; // parse connecting
            index = 1;
        }
        else if (c == 'D')
        {
            state = 4; // parse disconnect
            index = 1;
        }
        else
        {
            state = 0; // reset
        }
    }
    else if (state == 3)
    {
        if (c == '\r')
        {
            state = 5; // wait connect-trailing
        }
        else if (response_connect[index] == c )
        {
            index += 1; // compare next character
        }
        else if(response_connect[index] == 0x00)
        {
            // nop : ignore any character
        }
        else
        {
            state = 0; // reset
        }
    }
    else if (state == 4)
    {
        if (c == '\r')
        {
            state = 6; // wait disconnect-trailing
        }
        else if (response_disconnect[index] == c )
        {
            index += 1; // compare next character
        }
        else
        {
            state = 0; // reset
        }
    }
    else if (state == 5)
    {
        if (c == '\n' && response_connect[index] == 0x00)
        {
            //Serial.println("*** CONNECTED ***");
            connected = true;
        }

        state = 0; // reset
        Serial.println("Reset!");
    }
    else if (state == 6)
    {
        if (c == '\n' && response_disconnect[index] == 0x00)
        {
            //Serial.println("*** DISCONNECTED ***");
            connected = false;
        }

        state = 0; // reset
    }

    /*
    if (c == '\r')
    {
        if (state == 0)
        {
            state = 1; // wait linebreak prefix -> \n
        }
        else if (state == 3)
        {
            if (response_connect[index] == 0xFF)
            {
                state = 5; // wait connecting suffix -> \n
            }
            else
            {
                state = 0; // reset
            }
        }
        else if (state == 4)
        {
            if (response_connect[index] == 0x00)
            {
                state = 6; // wait disconnecting suffix -> \n
            }
            else
            {
                state = 0; // reset
            }
        }
        else
        {
            state = 0; // reset
        }
    }
    else if (c == '\n')
    {
        if (state == 1)
        {
            state = 2;
        }
        else
        {
            if (state == 5)
            {
                connected = 1;
            }
            else if (state == 6)
            {
                connected = 0;
            }

            state = 0;
        }
    }
    else
    {
        if (state == 2)
        {
            if (c == 'C')
            {
                state = 3;
                index = 1;
            }
            else if (c == 'D')
            {
                state = 4;
                index = 1;
            }
            else
            {
                state = 0;
            }
        }
        else if (state == 5)
        {
            if (response_connect[index] == c)
            {
                index += 1;
            }
            else if (response_connect[index] == 0xFF)
            {
                // nop
            }
            else
            {
                state = 0;
            }
        }
        else if (state == 6)
        {
            if (response_disconnect[index] == c)
            {
                index += 1;
            }
            else
            {
                state = 0;
            }
        }
        else
        {
            state = 0; // reset
        }
    }
    */

    return c;
}


////////////////////////////////////////////////////////////////////////////////////
//

SerialBluetooth SerialBT(USART1, BOARD_USART1_TX_PIN, BOARD_USART1_RX_PIN);
