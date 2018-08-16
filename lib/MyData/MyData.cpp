#include "MyData.h"

MyData::MyData() {
    _type = MD_UNKNOWN;
}


MyData::MyData(String pLabel, String pKey, bool pValue, uint8_t pOffset) 
: _label(pLabel), _key(pKey), _offset(pOffset)
{
    _type = MD_BOOL;
    _default._bool = pValue;
    _value._bool = pValue;
    _size = 1;
}

MyData::MyData(String pLabel, String pKey, int64_t pValue, uint8_t pOffset, uint8_t pSize) 
: _label(pLabel), _key(pKey), _offset(pOffset)
{
    _type = MD_INTEGER;
    _default._int = pValue;
    _value._int = pValue;
    // can only be int8, int16, int32, int64, i.e. 1,2,4,8
    if ((pSize == 1) || (pSize == 2) || (pSize == 4) || (pSize == 8)) {
        _size = pSize;
    } else {
        _size = sizeof(int64_t);
    }
}


MyData::MyData(String pLabel, String pKey, double pValue, uint8_t pOffset, uint8_t pSize) 
: _label(pLabel), _key(pKey), _offset(pOffset)
{
    _type = MD_FLOAT;
    _default._float = pValue;
    _value._float = pValue;
    // can only be float or double
    _size = (pSize == sizeof(float) ? pSize : sizeof(double));
}

MyData::MyData(String pLabel, String pKey, String pValue, uint8_t pOffset, uint8_t pSize) 
: _label(pLabel), _key(pKey), _offset(pOffset)
{
    _type = MD_STRING;
    _default._string = pValue;
    _value._string = pValue;
    _size = pSize;
}

void MyData::reset() {
    clear();
    switch (_type) {
        case MD_BOOL:
            _value._bool = _default._bool;
            break;
        case MD_INTEGER:
            _value._int = _default._int;
            break;
        case MD_FLOAT:
            _value._float = _default._float;
            break;
        case MD_STRING:
            _value._string = _default._string;
            break;
    }
}

void MyData::clear() {
    // Clear the value for any type
    _value._bool = false;
    _value._int = 0;
    _value._float = 0.0;
    _value._string = "";
}


bool MyData::setBool(bool pValue) {
    if (!isBool()) return false;
    _value._bool = pValue; 
    return true;
}

bool MyData::setInt(int64_t pValue) {
    if (!isInt()) return false;
    _value._int = pValue; 
    return true;
}

bool MyData::setFloat(double pValue) { 
    if (!isFloat()) return false;
    _value._float = pValue; 
    return true;
}

bool MyData::setString(String pValue) { 
    if (!isString()) return false;
    _value._string = pValue; 
    return true;
}

String MyData::getPrintable() {
    switch(_type) {
        case MD_BOOL:
            return (_value._bool ? "True" :  "False");
        case MD_INTEGER:
            return myUtil::getInt64String(_value._int);
        case MD_FLOAT:
            return String(_value._float);
        case MD_STRING:
            return _value._string;
    }
    return "<<Unknown>>";
}

bool MyData::toBuffer(uint8_t *buffer) {
    if (_size == 0) return false;  // This data cannot be copied
    uint8_t *target = buffer + _offset;
    switch (_type) {
        case MD_BOOL:
        {
            buffer[_offset] = getBool();
            return true;
        }
        case MD_INTEGER:
        {
            int64_t value = getInt();
            memcpy(target, &value, _size);
            return true;
        }
        case MD_FLOAT:
        {
            double value = getFloat();
            memcpy(target, &value, _size);
            return true;
        }
        case MD_STRING:
        {
            memset(target, 0, _size);
            size_t copySize = getString().length();
            if (copySize > _size) copySize = _size;
            memcpy(target, getString().c_str(), copySize);
            return true;
        }


    }
    return false;
}

bool MyData::fromBuffer(uint8_t *buffer) {
    if (_size == 0) return false;  // This data cannot be copied
    uint8_t *source = buffer + _offset;
    switch (_type) {
        case MD_BOOL:
        {
            bool value = (bool) (*source);
            _value._bool = value;
            return true;
        }
        case MD_INTEGER:
        {
            _value._int = 0;
            memcpy(&_value._int, source, _size);
            return true;
        }
        case MD_FLOAT:
        {
            if (_size == sizeof(float)) {
                float *fptr = (float *) source;
                _value._float = (*fptr);
            } else {
                double *dptr = (double *) source;
                _value._float = (*dptr);
            }
            return true;
        }
        case MD_STRING:
        {
            char buffer[_size+1];
            memset(buffer, 0, _size+1);
            memcpy(buffer, source, _size);
            _value._string = String(buffer);
            return true;
        }
    }
    return false;

}
