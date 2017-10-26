#include <IGCLogger.h>

IGCLogger logger;


void setup()
{
	//
	delay(1000);
	
	//
	Serial.begin(115200);
	while(! Serial);
	
	Serial.println("IGC Logger Test!!!");
	
	// FixedLenDigit test
	{
		FixedLenDigit d;
		
		d.begin(2016, 5);
		while (d.available())
			Serial.print((char)d.read());
		Serial.println("");
	}
	
	if (logger.init())
	{
		if (logger.begin(251017))
		{
			char * line = "B1602405407121N00249342WAPPPPPGGGGG\r\n";
			char * ptr;
			
			logger.update(2016);
			for (ptr = line; *ptr; ptr++)
			{
				Serial.print(*ptr);
				logger.write(*ptr);
			}
			Serial.println("");
			
			logger.update(1987);
			for (ptr = line; *ptr; ptr++)
			{
				Serial.print(*ptr);
				logger.write(*ptr);
			}
			Serial.println("");
			
			logger.update(242);
			for (ptr = line; *ptr; ptr++)
			{
				Serial.print(*ptr);
				logger.write(*ptr);
			}
			Serial.println("");
			
			Serial.println("close file");
			logger.end();
		}
		else
		{
			Serial.println("file creation failed!!");
		}
	}
	else
	{
		Serial.println("logger initialization failed!!");
	}
}

void loop()
{
}
