#ifndef _ACTION_DATA_H_
#define _ACTION_DATA_H_

#include <ESP8266WiFi.h>
#include <FS.h>

#define AD_HEADER_SIZE		128
#define AD_OFFSET_LEN		2
#define AD_OFFSET_ID   		4
#define AD_OFFSET_NAMESIZE	10
#define AD_OFFSET_NAME  	11
#define AD_OFFSET_STEP  	48
#define AD_DATA_SIZE		8200
#define AD_POSE_SIZE		32


class ActionData {
    public:
        ActionData();
		~ActionData();
		void InitObject(byte actionId);
		bool ReadSPIFFS(byte actionId);
		byte WriteSPIFFS();
		// bool DumpActionHeader(byte actionId);
		// bool DumpActionData(byte actionId);
	
	private:
		byte *_header;
		byte *_data;
		byte _id;
		byte _len;
		char *_name;
		byte *_action;
};

#endif