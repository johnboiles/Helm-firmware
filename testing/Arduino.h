#ifndef Arduino_h
#define Arduino_h

#include "inttypes.h"
#include "stdlib.h"

#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2

#ifndef min
#define min(a, b) (a < b ? a : b)
#endif

class MockSerial
{
public:
	MockSerial() { }
	static void begin(uint32_t baud) { }
	static void end();
	int peek() { return 0; }
	int read() { return 0; }
	int available() { return 0; }
	size_t write(uint8_t byte) { return 0; }
	void flush() {  }
	static void flushInput();
	static void flushOutput();
	bool overflow() { return false; }
	static int library_version() { return 0; }
	static void enable_timer0(bool enable) { }
	static bool timing_error;

    // size_t print(const __FlashStringHelper *) { return 0; }
    // size_t print(const String &) { return 0; }
    size_t print(const char[]) { return 0; }
    size_t print(char) { return 0; }
    size_t print(unsigned char, int = DEC) { return 0; }
    size_t print(int, int = DEC) { return 0; }
    size_t print(unsigned int, int = DEC) { return 0; }
    size_t print(long, int = DEC) { return 0; }
    size_t print(unsigned long, int = DEC) { return 0; }
    size_t print(double, int = 2) { return 0; }
    // size_t print(const Printable&) { return 0; }

    // size_t println(const __FlashStringHelper *) { return 0; }
    // size_t println(const String &s) { return 0; }
    size_t println(const char[]) { return 0; }
    size_t println(char) { return 0; }
    size_t println(unsigned char, int = DEC) { return 0; }
    size_t println(int, int = DEC) { return 0; }
    size_t println(unsigned int, int = DEC) { return 0; }
    size_t println(long, int = DEC) { return 0; }
    size_t println(unsigned long, int = DEC) { return 0; }
    size_t println(double, int = 2) { return 0; }
    // size_t println(const Printable&) { return 0; }
    size_t println(void) { return 0; }

	int printf(const char *format, ...) { return 0; }
};

extern MockSerial Serial;

#endif
