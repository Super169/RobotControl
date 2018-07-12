#ifndef _BUFFER_H_
#define _BUFFER_H_

#if defined(ESP8266)
	#include <ESP8266WiFi.h>
#else
	#include <WiFi.h>
#endif

class Buffer {
    public:
        Buffer(uint16_t size);
        ~Buffer();
		void reset();
		uint16_t available();
		byte peek();
		byte peek(uint16_t ptr);
		bool peek(byte *storage, uint16_t count);
		byte write();
		byte read();
		bool read(byte *storage, uint16_t count);
		bool skip() { return skip(1); }
		bool skip(uint16_t count);
		bool write(byte data);

		int head() { return _head; }
		int tail() { return _tail; }

	private:
		uint16_t _size;
		byte *_buffer;
		int _head;
		int _tail;

};

#endif