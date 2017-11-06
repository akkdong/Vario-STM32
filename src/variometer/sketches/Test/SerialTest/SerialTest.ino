// SerialTest.ino
//

#include <DefaultSettings.h>


void setup()
{
	//
	delay(1000);
	
	//
	Serial.begin(115200);
	while (! Serial);
	
	Serial1.begin(9600);
	while (! Serial1);

	Serial2.begin(9600);
	while (! Serial2);
	
	Serial.println("GPS Test!!!");
	
	Serial.println("GPS enable...");
	pinMode(PB1, OUTPUT);
	digitalWrite(PB1, LOW);
	
	Serial.println("BT enable...");
	pinMode(PB0, OUTPUT);
	digitalWrite(PB0, LOW);
	
	Serial.println("");
}

void loop()
{
	while (Serial1.available())
		Serial.write(Serial1.read());
	
	while (Serial2.available())
		Serial.write(Serial2.read());

	while (Serial.available())
		Serial1.write(Serial.read());
}
