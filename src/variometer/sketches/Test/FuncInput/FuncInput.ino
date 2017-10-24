// FuncInput.ino
//

#include <DigitalInput.h>
#include <VarioBeeper.h>

DigitalInput	funcInput;


void setup()
{
	//
	delay(1000);
	
	//
	Serial.begin(115200);
	Serial.println("Function-Key Input Test!!!");
	
	//
	funcInput.begin(PB5);	
}

void loop()
{
	funcInput.update();

	if (funcInput.fired())
	{
		// value format :
		// b7 b6 b5 b4 b3 b2 b1 b0
		// C2 C1 C0 I4 I3 I2 I1 I0
		//
		// Cx : count of bits (valid: 1~5, forbidden(ignore): 0, 6, 7)
		// Ix : input value
		//      each bit represents LONG(1) or SHORT(0) input, default is SHORT
		//		MSB first, RIGHT aligned
		//
		
		uint8_t value = funcInput.getValue();
		Serial.print("Input = "); Serial.print(value >> 5); Serial.print(", "); Serial.println(value & 0x1F);
	}	
}
