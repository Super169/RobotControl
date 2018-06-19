#ifndef _ACTION_DATA_H_
#define _ACTION_DATA_H_

#include <ESP8266WiFi.h>
#include <FS.h>
#include "RESULT.h"

#define AD_HEADER_SIZE			60
#define AD_OFFSET_LEN			2
#define AD_OFFSET_COMMAND       3
#define AD_OFFSET_ID   			4
#define AD_OFFSET_NAME  		6
#define AD_NAME_SIZE       		20
#define AD_OFFSET_POSECNT_LOW	28
#define AD_OFFSET_POSECNT_HIGH	29
#define AD_OFFSET_AFFECTSERVO	34

#define AD_COMMAND 				0x61

#define AD_POFFSET_ACTION		4
#define AD_POFFSET_SEQ 			5
#define AD_POFFSET_ENABLE		6
#define AD_POFFSET_STIME		7
#define AD_POFFSET_WTIME		9
#define AD_POFFSET_ANGLE		11
#define AD_POFFSET_LED			43
#define AD_POFFSET_HEAD			51
#define AD_POFFSET_MP3_FOLDER 	52
#define AD_POFFSET_MP3_FILE   	53
#define AD_POFFSET_MP3_VOL	  	54
#define AD_POFFSET_SEQ_HIGH		55

#define AD_MP3_STOP_VOL		  	0xFE

#define AD_POSE_SIZE			60
#define AD_PBUFFER_COUNT		10
#define AD_PBUFFER_SIZE			600		// Make sure AD_PBUFFER_SIZE = AD_PBUFFER_COUNT * AD_POSE_SIZE
#define AD_MAX_POSE         	65535



// poseCnt is single byte, so max is 255.
// But for safety, due to memory issue, only 12000 byte is used

#define ACTION_PATH "/alpha/action"
#define ACTION_FILE "/alpha/action/%03d.dat"
#define ACTION_POS  14

class ActionData {
    public:
        ActionData();
		~ActionData();
		void InitObject(byte actionId);
		bool SetActionName(char *actionName, byte len);
		bool SetActionName(String actionName);
		byte UpdatePose(byte actionId, byte poseId, byte *data);
		byte ReadSPIFFS(byte actionId);
		byte WriteSPIFFS();
		byte WriteHeader();
		byte WritePoseData();
		byte SpiffsWritePoseData();
		byte DeleteActionFile(byte actionId);
		// bool DumpActionHeader(byte actionId);
		// bool DumpActionData(byte actionId);
		byte id() { return _id; }

		byte * Header() { return (byte *) _header; }
		byte * Data() { return (byte *) _data; }

		uint16_t PoseCnt() { return (_header[AD_OFFSET_POSECNT_HIGH] << 8 | _header[AD_OFFSET_POSECNT_LOW]); }
		uint16_t PoseOffset() { return _poseOffset; }
		uint16_t BufferEndPose() { return (_poseOffset + AD_PBUFFER_COUNT - 1);}
		void RefreshActionInfo();
		bool IsPoseReady(uint16_t poseId);
		bool IsPoseReady(uint16_t poseId, uint16_t &offset);
		bool PoseOffsetInBuffer(uint16_t poseId, uint16_t &offset);

		void GenSample(byte actionId);


	private:

		byte ReadActionFile(int actionId);
		byte SpiffsReadActionFile(int actionId);
		byte ReadActionHeader(int actionId);
		byte SpiffsReadActionHeader(int actionId);
		byte ReadActionPose();
		byte SpiffsReadActionPose();

		byte _id;
		byte _len;
		char *_name;
		byte *_action;
		char _filename[25];
		uint16_t _poseOffset;
		byte _header[AD_HEADER_SIZE];
		byte _data[AD_PBUFFER_SIZE];
		
};

#endif