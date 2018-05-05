#include <ESP8266WiFi.h>
#include "Buffer.h"

Buffer buffer(64);

void setup() {
	delay(1000);
	Serial.begin(115200);

	if (!buffer.available()) Serial.println(F("Buffer empty"));

	for (int i = 0; i < 100; i++) buffer.write(i);

	Serial.print("\nData in buffer: ");
	Serial.println(buffer.available());

	Serial.print("\nRead data: ");
	Serial.println(buffer.read());;
	Serial.print("Data in buffer: ");
	Serial.println(buffer.available());

	Serial.print("\nPeek data: ");
	Serial.println(buffer.peek());;
	Serial.print("Data in buffer: ");
	Serial.println(buffer.available());

	byte data[50];
	if (buffer.read(data, 50)) {
		Serial.print("\nRead data: ");
    ShowData(data, 50);
		Serial.print("Data in buffer: ");
		Serial.println(buffer.available());
	}

	if (buffer.peek(data, 50)) {
		Serial.print("\nPeek data: ");
    ShowData(data, 50);
		Serial.print("Data in buffer: ");
		Serial.println(buffer.available());
	}

	if (buffer.read(data, 50)) {
		Serial.print("\nAnother 50 data read");
	} else {
		Serial.print("\nNot enough data for peek");
	}

	if (buffer.peek(data, 50)) {
		Serial.print("\nAnother 50 data peek");
	} else {
		Serial.print("\nNot enough data for peek");
	}


	for (int i = 0; i < 100; i++) buffer.write(i);

	Serial.print("\nData in buffer: ");
	Serial.println(buffer.available());

	Serial.print("\nPeek data: ");
	Serial.println(buffer.peek());;
	Serial.print("Data in buffer: ");
	Serial.println(buffer.available());

	Serial.print("\nRead data: ");
	Serial.println(buffer.read());;
	Serial.print("Data in buffer: ");
	Serial.println(buffer.available());

	if (buffer.peek(data, 50)) {
		Serial.print("\nPeek data: ");
    ShowData(data, 50);
		Serial.print("Data in buffer: ");
		Serial.println(buffer.available());
	}

	if (buffer.read(data, 50)) {
		Serial.print("\nRead data: ");
    ShowData(data, 50);
		Serial.print("Data in buffer: ");
		Serial.println(buffer.available());
	}

	Serial.print("\nHead: ");
	Serial.print(buffer.head());
	Serial.print(" ; Tail: ");
	Serial.print(buffer.tail());
	Serial.print(" => available: ");
	Serial.print(buffer.available());

  int count = buffer.available();
	if (buffer.read(data, count)) {
		Serial.print("\nRead all data out!");
    ShowData(data, count);
    Serial.print("Data in buffer: ");
		Serial.print("\nData in buffer: ");
		Serial.println(buffer.available());
	}

	Serial.print("\nHead: ");
	Serial.print(buffer.head());
	Serial.print(" ; Tail: ");
	Serial.print(buffer.tail());
	Serial.print(" => available: ");
	Serial.print(buffer.available());

}

void loop() {
  
}

void ShowData(byte *data, uint16_t count) {
    for (int i = 0; i < count; i++) {
      Serial.print(data[i]);
      Serial.print(" ");
    } 
    Serial.println();
}

