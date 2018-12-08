#ifndef _RESULT_H_
#define _RESULT_H_


class RESULT {
    public:
        static const byte SUCCESS                       = 0;
		
		class ERR {
			public:
				// Generic error 
				static const byte  SPIFFS                   = 1;
				static const byte  NOT_FOUND				= 2;
				static const byte  NOT_MATCH				= 3;
				static const byte  NOT_READY				= 4;
				static const byte  READ						= 5;
				static const byte  WRITE					= 6;
				static const byte  COPY						= 7;
				static const byte  DATA_OVERFLOW            = 8;
				static const byte  VERSION_NOT_MATCH		= 9;


				static const byte  PARM_SIZE                = 11;
				static const byte  PARM_AID_NOT_MATCH       = 12;
				static const byte  PARM_AD_NAME_SIZE	    = 13;
				static const byte  PARM_PID_OUT_RANGE	    = 14;
				static const byte  PARM_COMBO_OUT_RANGE	    = 15;
				static const byte  PARM_INVALID             = 16;
				static const byte  CHECKSUM			        = 19;
				static const byte  FILE_SPIFFS			    = 21;
				static const byte  FILE_NOT_FOUND		    = 22;
				static const byte  FILE_OPEN_READ		    = 23;
				static const byte  FILE_OPEN_WRITE		    = 24;
				static const byte  FILE_OPEN_APPEND	        = 25;
				static const byte  FILE_SIZE			    = 26;
				static const byte  FILE_READ_COUNT		    = 27;
				static const byte  FILE_WRITE_COUNT	        = 28; 
				static const byte  FILE_SEEK			    = 29;
				static const byte  FILE_REMOVE              = 30;
				static const byte  AD_HEADER_CONTENT	    = 31;
				static const byte  AD_HEADER_CHECKSUM	    = 32;
				static const byte  AD_POSE_NOT_READY	    = 35;
				static const byte  AD_POSE_CHECKSUM	        = 36;
				static const byte  UPDATE_CONDIG            = 41;
				static const byte  UNKNOWN                  = 255;
		};
};


#endif