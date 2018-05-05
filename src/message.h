#ifndef _message_h_
#define _message_h_

#define __PROG_TYPES_COMPAT__

#if defined( ESP8266 )
  #include <pgmspace.h>
#else
  #include <avr/pgmspace.h>
#endif

const char msg_welcomeHeader[] PROGMEM = "Robot Control v2";
const char msg_author[] PROGMEM = "by Super169";
const char msg_welcomeMsg[] PROGMEM = "Starting robot.......";

#endif