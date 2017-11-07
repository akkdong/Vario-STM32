// PowerOff.ino
//

void setup()
{
	//pinMode(PB14, OUTPUT);
	//digitalWrite(PB14, HIGH);
	
	pinMode(PB15, INPUT);
	
	//
	delay(1000);
	
	//
	Serial.begin(115200);
	while (! Serial);
	
	Serial.println("Self Power off Test!!!");
	Serial.println("");
	Serial.println("Enter any character!!");
}

void loop()
{
	if (Serial.available())
	{
		Serial.println("Now!!!! Power Off.....");
		
		digitalWrite(PB14, LOW);
		while(1);
	}
	else
	{
		if (! digitalRead(PB15))
		{
			Serial.println("Ready for Shutdown!!!");
			while(1);
		}
	}
}