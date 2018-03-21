#ifndef _ACTION_DATA_H_
#define _ACTION_DATA_H_

#include <ESP8266WiFi.h>
#include <FS.h>

#define AD_HEADER_SIZE			60
#define AD_OFFSET_LEN			2
#define AD_OFFSET_ID   			4
#define AD_OFFSET_NAME  		6
#define AD_NAME_SIZE       		20
#define AD_OFFSET_POSECNT 		28
#define AD_OFFSET_AFFECTSERVO	34


#define AD_DATA_SIZE		8200
#define AD_POSE_SIZE		32
#define AD_MAX_POSE         255

// poseCnt is single byte, so max is 255.
// AD_POSE_SIZE * AD_MAX_POSE = 8160, just reseve extra 32 byte for ending


#define ACTION_FILE "/alpha/action/%03d.dat"

#define AD_POFFSET_STIME	2
#define AD_POFFSET_WTIME	4
#define AD_POFFSET_ANGLE	6
#define AD_POFFSET_LED		22


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