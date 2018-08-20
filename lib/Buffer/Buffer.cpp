#include "Buffer.h"

Buffer::Buffer(uint16_t size)
{
	init(size);
}

Buffer::~Buffer() {
	free(_buffer);
}

void Buffer::reset() {
	_head = _tail = 0;
}

void Buffer::init(uint16_t size)
{
	_size = size;
	if (size == 0) return;
    _buffer = (byte *) malloc(size);
	reset();
}


uint16_t Buffer::available() {
	if (!_size) return 0;
	int count = _tail - _head;
	if (count < 0) count += _size;
	return count;
}

byte Buffer::peek() {
	if ((!_size) || (_head == _tail)) return 0;
	return _buffer[_head];
}

byte Buffer::peek(uint16_t pos) {
	if ((!_size) || (_head == _tail)) return 0;
	if (pos >= available()) return 0;
	int peekPos = (_head + pos) % _size;
	return _buffer[peekPos];
}

bool Buffer::peek(byte *storage, uint16_t count) {
	if (available() < count) return false;
	byte *startPos = _buffer + _head;
	if (_tail > _head) {
		// read staight forward
		memcpy(storage, startPos, count);
		return true;
	}
	// part I, read to the end
	int size1 = (_size - _head);
	int size2 = count - size1;
	memcpy(storage, startPos, size1);

	startPos = storage + size1;
	memcpy(startPos, _buffer, size2);
	_head = size2;
	return true;
}


byte Buffer::read() {
	if ((!_size) || (_head == _tail)) return 0;
	byte data = _buffer[_head];
	_head = (_head + 1) % _size;
	if (_head == _tail) reset();  // reset the point to reduce the chance of multiple read for block read
	return data;
}

bool Buffer::read(byte *storage, uint16_t count) {
	if (!peek(storage, count)) return false;
	_head = (_head + count) % _size;
	if (_head == _tail) reset();  // reset the point to reduce the chance of multiple read for block read
	return true;
}

bool Buffer::skip(uint16_t count) {
	if (available() < count) return false;
	_head = (_head + count) % _size;
	if (_head == _tail) reset();
	return true;
}

bool Buffer::write(byte data) {
	if (!_size) return false;
	int newTail = (_tail + 1) % _size;
	if (newTail == _head) {
		// buffer full, need to do somehting
		return false;
	}
	_buffer[_tail] = data;
	_tail = newTail;
	return true;
}