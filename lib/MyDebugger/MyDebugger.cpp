#include "MyDebugger.h"

MyDebugger::MyDebugger() {
	_enableDebug = false;
	_dbg = NULL;
}

MyDebugger::~MyDebugger() {

}

void MyDebugger::setOutput(Stream *output, bool enableDebug) {
	_dbg = output;
	_enableDebug = (enableDebug && (_dbg != NULL));
}

void MyDebugger::enableDebug(bool value) {
	_enableDebug = value;
}


size_t MyDebugger::write(uint8_t b) {
	if ((!_enableDebug) || (_dbg == NULL)) return 0;
	return _dbg->write(b);
}

// To provide standard message for debug
// It can still use _dbg to send debug message if standard message is not requried.
void MyDebugger::msg(const char *format, ...) {

	if (!isEnabled()) {
		return;
	}

	_dbg->printf("%08ld: ", millis());

	// source from Print::pritnf, as it cannot call printf wihtin the method
	// just modify write to _dbg->write as it's now not under Strem object
    char loc_buf[64];
    char * temp = loc_buf;
    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);
    size_t len = vsnprintf(NULL, 0, format, arg);
    va_end(copy);
    if(len >= sizeof(loc_buf)){
        temp = new char[len+1];
        if(temp == NULL) {
			Serial.println("temp is null");
            return;
        }
    }
    len = vsnprintf(temp, len+1, format, arg);
    _dbg->write((uint8_t*)temp, len);
    va_end(arg);
    if(len > 64){
        delete[] temp;
    }
	// -- end of Print::print

	_dbg->println();
	delay(1);
    return;	
}