#ifndef _COMBO_DATA_H_
#define _COMBO_DATA_H_

#include <ESP8266WiFi.h>
#include <FS.h>
#include "RESULT.h"

#define CD_MAX_COMBO        10
#define CD_COMBO_DATA_SIZE  60
#define CD_COMBO_SIZE       2

#define COMBO_PATH "/alpha/combo"
#define COMBO_FILE "/alpha/combo/%03d.dat"
#define COMBO_POS  13

#define CD_OFFSET_SEQ		    4
#define CD_OFFSET_DATA 			10
#define CD_COFFSET_ACTION       0
#define CD_COFFSET_COUNT        1

class ComboData {
    public:
        ComboData();
		~ComboData();
        void InitCombo(byte seq);

        inline byte ReadSPIFFS() { return ReadSPIFFS(_seq); }
        byte ReadSPIFFS(byte seq);

        byte * Data() { return (byte *) _data; }

    private:
        byte SpiffsReadComboFile(byte seq);

        byte _seq;
        char _data[CD_COMBO_DATA_SIZE];
};

#endif