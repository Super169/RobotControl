#ifndef _ACTION_DATA_H_
#define _ACTION_DATA_H_

#include <ESP8266WiFi.h>
#include <FS.h>

#define AD_MAX_SERVO			16

#define AD_HEADER_SIZE			60
#define AD_OFFSET_LEN			2
#define AD_OFFSET_ID   			4
#define AD_OFFSET_NAME  		6
#define AD_NAME_SIZE       		20
#define AD_OFFSET_POSECNT 		28
#define AD_OFFSET_AFFECTSERVO	34

#define AD_POFFSET_STIME	7
#define AD_POFFSET_WTIME	9
#define AD_POFFSET_ANGLE	11
#define AD_POFFSET_LED		43
#define AD_POFFSET_HEAD		51
#define AD_POFFSET_MP3_FOLDER 52
#define AD_POFFSET_MP3_FILE   53
#define AD_POFFSET_MP3_VOL	  54
#define AD_MP3_STOP_VOL		  0xFE



#define AD_POSE_SIZE		60
#define AD_MAX_POSE         200
#define AD_DATA_SIZE		12000


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
		bool ReadSPIFFS(byte actionId);
		byte WriteSPIFFS();
		byte DeleteSPIFFS(byte actionId);
		// bool DumpActionHeader(byte actionId);
		// bool DumpActionData(byte actionId);
		byte id() { return _id; }

		byte * Header() { return (byte *) _header; }
		byte * Data() { return (byte *) _data; }

		byte PoseCnt() { return _header[AD_OFFSET_POSECNT]; }
		void RefreshActionInfo();

		void GenSample(byte actionId);

	private:

		bool ReadActionFile(int actionId);

		byte _header[AD_HEADER_SIZE];
		byte _data[AD_DATA_SIZE];
		byte _id;
		byte _len;
		char *_name;
		byte *_action;
		char _filename[25];
};

#endif