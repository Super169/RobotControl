#ifndef _MY_DATA_H_
#define _MY_DATA_H_

#include <ESP8266WiFi.h>
#include "myUtil.h"

#define MD_UNKNOWN  (0)
#define MD_BOOL     (1 << 0)
#define MD_INTEGER   (1 << 1)
#define MD_FLOAT    (1 << 2)
#define MD_STRING   (1 << 3)

class MyData {
    public:

        // Becarefule, it seems that bool, int64_t cannot be distinguished
        MyData();
        MyData(String pLabel, String pKey, bool pValue, uint8_t pOffset = 0);
        MyData(String pLabel, String pKey, int64_t pValue, uint8_t pOffset = 0, uint8_t pSize = 0);
        MyData(String pLabel, String pKey, double pValue, uint8_t pOffset = 0, uint8_t pSize = 0);
        MyData(String pLabel, String pKey, String pValue, uint8_t pOffset = 0, uint8_t pSize = 0);

        void reset();
        void clear();

        inline uint8_t type() { return _type; }
        inline String label() { return _label; }
        inline String key() { return _key; }
        inline uint8_t offset() { return _offset; }
        inline uint8_t size() { return _size; }


        inline bool getBool() { return _value._bool; }
        inline int64_t getInt() { return _value._int; }
        inline double getFloat() { return _value._float; }
        inline String getString() { return _value._string; }

        inline bool getBoolDefault() { return _default._bool; }
        inline int64_t getIntDefault() { return _default._int; }
        inline double getFloatDefault() { return _default._float; }
        inline String getStringDefault() { return _default._string; }
        String getPrintable();


        // Becarefule, it seems that bool, int64_t cannot be distinguished
        inline bool set(bool pValue) { return setBool(pValue); }
        inline bool set(int64_t pValue) { return setInt(pValue); }
        inline bool set(double pValue) { return setFloat(pValue); }
        inline bool set(String pValue) { return setString(pValue); }

        bool setBool(bool pValue);
        bool setInt(int64_t pValue);
        bool setFloat(double pValue);
        bool setString(String pValue);

        inline bool isBool() { return (_type == MD_BOOL); }
        inline bool isInt() { return (_type == MD_INTEGER); }
        inline bool isFloat() { return (_type == MD_FLOAT); }
        inline bool isString() { return (_type == MD_STRING); }

        bool toBuffer(uint8_t *buffer);
        bool fromBuffer(uint8_t *buffer);


    private:
        struct CONTENT{
            bool _bool;
            int64_t _int;
            double _float;
            String _string;
        };

        uint8_t _type;
        String _label;
        String _key;
        CONTENT _default;
        CONTENT _value;
        uint8_t _offset;
        uint8_t _size;
};

#endif
