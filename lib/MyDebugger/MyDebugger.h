#ifndef _MY_DEBUGGER_H_
#define _MY_DEBUGGER_H_

#ifdef ESP32
	#include <Esp.h>
#else
	#include <ESP8266WiFi.h>
#endif	

#include <Stream.h>
#include <Print.h>

class MyDebugger : public Stream {
	public: 
		MyDebugger();
		~MyDebugger();
		void setOutput(Stream *output, bool enableDebug = true);
		void enableDebug(bool value);
		bool isEnabled() { return ((_enableDebug) && (_dbg != NULL)); }

		// Override pure virtual function in Stream
		int available() {return 0;}
		int read() {return 0;}
		int peek() { return 0; }
		void flush() { return; }

		void msg(const char * format, ...) __attribute__ ((format (printf, 2, 3)));
		void msgf(const char * format, ...) __attribute__ ((format (printf, 2, 3)));
		void msgh(const char * format, ...) __attribute__ ((format (printf, 2, 3)));  // Shoudl be combined with msg, but need to change program, do it in next version

		size_t write(uint8_t byte);
		using Print::write;		// Since Strem has not define for write, must bring from Print

	private:
		bool _enableDebug;
		Stream *_dbg;		

};

#endif