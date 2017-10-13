#include <Arduino.h>
#include <HardwareSerial.h>

#include <libmaple/libmaple.h>
#include <libmaple/gpio.h>
#include <libmaple/timer.h>
#include <libmaple/usart.h>

DEFINE_HWSERIAL(serial, 1);

void setup() {
  // put your setup code here, to run once:
  serial.begin(115200);
  while(! serial);
}

void loop() {
  // put your main code here, to run repeatedly:
  serial.println("Hello...");
  delay(1);
}
