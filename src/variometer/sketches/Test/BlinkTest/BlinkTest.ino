// BlinkTest.ino
//

#include <LEDFlasher.h>


LEDFlasher flasher;
uint8_t blinkType = BLINK_NONE;

void setup()
{
	//
	Serial.begin();
	while (! Serial);
	delay(400);
	
	//
	flasher.begin(PIN_MCU_STATE, ACTIVE_LOW);
	flasher.turnOn();
}

void loop()
{
	while (Serial.available())
	{
		int c = Serial.read();
		
		switch (c)
		{
		case '0' : // BTYPE_LONG_ON_OFF,		// ----____
			flasher.blink(BTYPE_LONG_ON_OFF);
			break;
		case '1' : // BTYPE_SHORT_ON_OFF,		// -_
			flasher.blink(BTYPE_SHORT_ON_OFF);
			break;
		case '2' : // BTYPE_LONG_ON_SHORT_OFF,	// ----_
			flasher.blink(BTYPE_LONG_ON_SHORT_OFF);
			break;
		case '3' : // BTYPE_SHORT_ON_LONG_OFF,	// -____
			flasher.blink(BTYPE_SHORT_ON_LONG_OFF);
			break;
		case '4' : // BTYPE_BLINK_2_LONG_ON,	// -_-_----_
			flasher.blink(BTYPE_BLINK_2_LONG_ON);
			break;
		case '5' : // BTYPE_BLINK_3_LONG_ON,	// -_-_-_----_
			flasher.blink(BTYPE_BLINK_3_LONG_ON);
			break;
		case '6' : // BTYPE_BLINK_2_LONG_OFF,	// -_-_____
			flasher.blink(BTYPE_BLINK_2_LONG_OFF);
			break;
		case '7' : // BTYPE_BLINK_3_LONG_OFF,	// -_-_-_____
			flasher.blink(BTYPE_BLINK_3_LONG_OFF);
			break;
			
		case 'o' :
		case 'O' :
			flasher.turnOn();
			break;
		case 'f' :
		case 'F' :
			flasher.turnOff();
			break;
		}
	}
	
	flasher.update();
}
