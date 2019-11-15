#ifndef DRSOSC_READ_H
#define DRSOSC_READ_H
#include "TStopwatch.h"
#include "TMap.h"
#include "app.h"

typedef struct {
	char tag[3];
	char version;
} FHEADER;

typedef struct {
	char time_header[4];
} THEADER;

typedef struct {
	char bn[2];
	unsigned short board_serial_number;
} BHEADER;

typedef struct {
	char event_header[4];
	unsigned int event_serial_number;
	unsigned short year;
	unsigned short month;
	unsigned short day;
	unsigned short hour;
	unsigned short minute;
	unsigned short second;
	unsigned short millisecond;
	unsigned short range;
} EHEADER;

typedef struct {
	char tc[2];
	unsigned short trigger_cell;
} TCHEADER;

typedef struct {
	char c[1];
	char cn[3];
} CHEADER;

#endif
