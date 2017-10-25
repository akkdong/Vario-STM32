// ToneGenerator.cpp
//

#include <Arduino.h>
#include "ToneGenerator.h"

#include <libmaple/gpio.h>
#include <libmaple/timer.h>
#include <boards.h>


#define PIN_UNDEFINED		((uint8)-1)


////////////////////////////////////////////////////////////////////////////////////
// class ToneGenerator

ToneGenerator::ToneGenerator() : tonePin(PIN_UNDEFINED)
{
}


void ToneGenerator::begin(uint8 pin)
{
	//
	timer_dev * dev = PIN_MAP[pin].timer_device;

	//
	timer_pause(dev);
	timer_set_mode(dev, PIN_MAP[pin].timer_channel, TIMER_PWM);
	timer_set_prescaler(dev, (uint16)(72 - 1)); // 72MHz / 72 = 1MHz
	timer_set_reload(dev, 1000000/260); // default tone
	timer_set_compare(dev, PIN_MAP[pin].timer_channel, 0); // mute
	timer_generate_update(dev);
	timer_resume(dev);
	//
	gpio_set_mode(PIN_MAP[pin].gpio_device, PIN_MAP[pin].gpio_bit, GPIO_INPUT_FLOATING);
	
	//
	tonePin = pin;
}

void ToneGenerator::end()
{
	//
	timer_dev * dev = PIN_MAP[tonePin].timer_device;
	
	//
	timer_cc_disable(PIN_MAP[tonePin].timer_device, PIN_MAP[tonePin].timer_channel); 
	gpio_set_mode(PIN_MAP[tonePin].gpio_device, PIN_MAP[tonePin].gpio_bit, GPIO_INPUT_FLOATING);
	
	//
	tonePin = PIN_UNDEFINED;
}

void ToneGenerator::setTone(uint16 hz, uint8 volume)
{
	if (tonePin == PIN_UNDEFINED)
		return;
	
	//
	timer_dev * dev = PIN_MAP[tonePin].timer_device;
	
	if (hz > 0)
	{
		uint16 value = 1000000 / hz;
		
		//
		timer_set_reload(dev, value);
		timer_set_compare(dev, PIN_MAP[tonePin].timer_channel, value * (volume / 2) / 100);
		//
		gpio_set_mode(PIN_MAP[tonePin].gpio_device, PIN_MAP[tonePin].gpio_bit, GPIO_AF_OUTPUT_PP);
	}
	else
	{
		timer_set_compare(dev, PIN_MAP[tonePin].timer_channel, 0);
		gpio_set_mode(PIN_MAP[tonePin].gpio_device, PIN_MAP[tonePin].gpio_bit, GPIO_INPUT_FLOATING);
	}
}
