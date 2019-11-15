#ifndef PROCESS_DATA_H
#define PROCESS_DATA_H

#include "TTree.h"
#include "TBranch.h"
#include "TObjArray.h"
#include "app.h"

#define PEAK_THRESHOLD 0.3
#define RESISTANCE 50
#define EVENT_THRESHOLD .01

void moving_average(double channel_waveform[], double channel_time[N_BINS]);
double rise_time(double channel_waveform[N_BINS], double channel_time[N_BINS]);
double amplitude(double channel_waveform[N_BINS]);
double charge(double channel_waveform[N_BINS], double channel_time[N_BINS]);

#endif
