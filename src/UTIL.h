#ifndef _UTIL_H_
#define _UTIL_H_

void SetDebug(bool mode);
void SetHeadLed(bool status);

byte CheckSum(byte *cmd);
byte CheckVarSum(byte *cmd);
byte CheckFullSum(byte *cmd);

void clearInputBuffer();
bool cmdSkip(bool flag);

void DebugShowSkipByte();

#endif