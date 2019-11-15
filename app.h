#ifndef APP_H
#define APP_H

#include <TH1D.h>
#include <TGraph.h>
#include <TTree.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TTreeReaderArray.h>
#include <TCanvas.h>
#include <TString.h>
#include <TObject.h>
#include <TBranch.h>


#define MAX_N_BOARDS 16
#define N_CHANNELS 4
#define MAX_CHANNELS MAX_N_BOARDS * N_CHANNELS
#define N_BINS 1024
#define PEAK_THRESHOLD 0.3


class EventData : public TObject {
	public:
		double time[N_BINS];
		double waveform[N_BINS];
};

// typedef struct {
// 	double time[N_BINS];
// 	double waveform[N_BINS];
// } EventData;


class EventStats : public TObject {
public:
double amplitude, charge, rise_time, difference;
};


class BoardId : public TObject{
public:
	int channel;
	int board;
	BoardId(int board, int channel) {
		this->channel = channel;
		this->board = board;
	}
	virtual ULong_t Hash() const{
		return channel + board * N_CHANNELS;
	}

	TString str(TString type){
		TString s; s.Form("%lu" + type,this->Hash());
		return s;
	}

	TString str(){
		TString s; s.Form("%lu",this->Hash());
		return s;
	}
};

#endif
