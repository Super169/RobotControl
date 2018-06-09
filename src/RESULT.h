#ifndef _RESULT_H_
#define _RESULT_H_

class RESULT {
    public:
        static const byte SUCCSSS                       = 0;
        static const byte ERR_SPIFFS                    = 1;
        static const byte ERR_PARM_SIZE                 = 11;
        static const byte ERR_PARM_AID_NOT_MATCH        = 12;
        static const byte ERR_PARM_AD_NAME_SIZE	        = 13;
        static const byte ERR_PARM_PID_OUT_RANGE	    = 14;
        static const byte ERR_CHECKSUM			        = 19;
        static const byte ERR_FILE_SPIFFS			    = 21;
        static const byte ERR_FILE_NOT_FOUND		    = 22;
        static const byte ERR_FILE_OPEN_READ		    = 23;
        static const byte ERR_FILE_OPEN_WRITE		    = 24;
        static const byte ERR_FILE_OPEN_APPEND	        = 25;
        static const byte ERR_FILE_SIZE			        = 26;
        static const byte ERR_FILE_READ_COUNT		    = 27;
        static const byte ERR_FILE_WRITE_COUNT	        = 28; 
        static const byte ERR_FILE_SEEK			        = 29;
        static const byte ERR_FILE_REMOVE               = 30;
        static const byte ERR_AD_HEADER_CONTENT	        = 31;
        static const byte ERR_AD_HEADER_CHECKSUM	    = 32;
        static const byte ERR_AD_POSE_NOT_READY	        = 35;
        static const byte ERR_AD_POSE_CHECKSUM	        = 36;
        static const byte ERR_UNKNOWN                   = 255;
};


#endif