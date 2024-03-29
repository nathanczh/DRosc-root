/*

   Name:           read_binary.C
   Created by:     Stefan Ritt <stefan.ritt@psi.ch>
   Date:           July 30th, 2014
   Modified By:    Abaz Kryemadhi
   Date:           March 7th, 2019

   Purpose:        Example program under ROOT to read a binary data file written
                   by the DRSOsc program. Decode time and voltages from waveforms
                   and display them as a graph. Put values into a ROOT Tree for
                   further analysis.

                   To run it, do:

                   - Crate a file test.dat via the "Save" button in DRSOsc
                   - start ROOT (type root)
                   root [0] .L read_binary.C+
                   root [1] decode("test.dat");

 */


#include <fcntl.h>
#include <unistd.h>
#include <math.h>


#include <string.h>
#include <stdio.h>
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TStopwatch.h"
#include "Getline.h"
#include "TAxis.h"

#define MAX_N_BOARDS 16
#define N_CHANNELS 4
#define N_BINS 1024
#define PEAK_THRESHOLD 0.3
#define RESISTANCE 50
#define EVENT_THRESHOLD .01


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

/*-----------------------------------------------------------------------------*/
void decode(char *filename);
void moving_average(double channel_waveform[], double channel_time[N_BINS]);
double rise_time(double channel_waveform[N_BINS], double channel_time[N_BINS]);
double amplitude(double channel_waveform[N_BINS]);
double charge(double channel_waveform[N_BINS], double channel_time[N_BINS]);

//int main(int argc, const char * argv[])
TTree * decode(TString filename) {

	FHEADER fh;
	THEADER th;
	BHEADER bh;
	EHEADER eh;
	TCHEADER tch;
	CHEADER ch;

	TStopwatch* stopwatch = new TStopwatch();

	unsigned int scaler;
	unsigned short voltage[N_BINS];
	double waveform[MAX_N_BOARDS][N_CHANNELS][N_BINS], time[MAX_N_BOARDS][N_CHANNELS][N_BINS];
	float bin_width[MAX_N_BOARDS][N_CHANNELS][N_BINS];
	int i, j, b, chn, n, chn_index, n_boards;
	double t1, t2, dt;
	//char filename[256];
	char rootfile[256];
	int ndt;
	double threshold, sumdt, sumdt2;
	double amplitude;
	double charge;
	double rise_time;

	TString data_filename = (filename + ".dat");
	// TFile* infile = new TFile(data_filename,"READ");
	TFile* outfile = new TFile(filename + ".root", "RECREATE");
	FILE *f = fopen(data_filename.Data(), "rb");
	printf("Opened file %s\n", data_filename.Data());
	// define the rec tree
	TTree *rec = new TTree("rec","rec");
	rec->Branch("t", time[0][0],"t[1024]/D");
	rec->Branch("w", waveform[0][0],"w[1024]/D");
	rec->Branch("amplitude", &amplitude,"amplitude/D");
	rec->Branch("charge", &charge,"charge/D");
	rec->Branch("rise_time", &rise_time,"rise_time/D");
	// create canvas
	// TCanvas *c1 = new TCanvas();

	// create graph
	// TGraph *g = new TGraph(1024, (double *)time[0][0], (double *)waveform[0][0]);


	// read file header
	fread(&fh, sizeof(fh), 1, f);
	if (fh.tag[0] != 'D' || fh.tag[1] != 'R' || fh.tag[2] != 'S') {
		printf("Found invalid file header in file \'%s\', aborting.\n", filename.Data());
		return 0;
	}

	if (fh.version != '2') {
		printf("Found invalid file version \'%c\' in file \'%s\', should be \'2\', aborting.\n", fh.version, filename.Data());
		return 0;
	}

	// read time header
	fread(&th, sizeof(th), 1, f);
	if (memcmp(th.time_header, "TIME", 4) != 0) {
		printf("Invalid time header in file \'%s\', aborting.\n", filename.Data());
		return 0;
	}

	for (b = 0;; b++) {
		// read board header
		fread(&bh, sizeof(bh), 1, f);
		if (memcmp(bh.bn, "B#", 2) != 0) {
			// probably event header found
			fseek(f, -4, SEEK_CUR);
			break;
		}

		printf("Found data for board #%d\n", bh.board_serial_number);

		// read time bin widths
		memset(bin_width[b], 0, sizeof(bin_width[0]));
		for (chn=0; chn<5; chn++) {
			fread(&ch, sizeof(ch), 1, f);
			if (ch.c[0] != 'C') {
				// event header found
				fseek(f, -4, SEEK_CUR);
				break;
			}
			i = ch.cn[2] - '0' - 1;
			printf("Found timing calibration for channel #%d\n", i+1);
			fread(&bin_width[b][i][0], sizeof(float), 1024, f);
			// fix for 2048 bin mode: double channel
			if (bin_width[b][i][1023] > 10 || bin_width[b][i][1023] < 0.01) {
				for (j=0; j<512; j++)
					bin_width[b][i][j+512] = bin_width[b][i][j];
			}
		}
	}
	n_boards = b;

	// initialize statistics
	ndt = 0;
	sumdt = sumdt2 = 0;

	printf("Processing Events\n");
	stopwatch->Start();
	// loop over all events in the data file
	for (n=0;; n++) {
		// read event header
		i = (int)fread(&eh, sizeof(eh), 1, f);
		if (i < 1)
			break;

		if (n % 1000 == 1 ) {
			printf("Processed %d events (#%d %d %d)\n", n-1, eh.event_serial_number, eh.second, eh.millisecond);
		}


		// loop over all boards in data file
		for (b=0; b<n_boards; b++) {

			// read board header
			fread(&bh, sizeof(bh), 1, f);
			if (memcmp(bh.bn, "B#", 2) != 0) {
				printf("Invalid board header in file \'%s\', aborting.\n", filename.Data());
				return 0;
			}

			// read trigger cell
			fread(&tch, sizeof(tch), 1, f);
			if (memcmp(tch.tc, "T#", 2) != 0) {
				printf("Invalid trigger cell header in file \'%s\', aborting.\n", filename.Data());
				return 0;
			}

			if (n_boards > 1)
				printf("Found data for board #%d\n", bh.board_serial_number);

			// reach channel data
			for (chn=0; chn<4; chn++) {

				// read channel header
				fread(&ch, sizeof(ch), 1, f);
				if (ch.c[0] != 'C') {
					// event header found
					fseek(f, -4, SEEK_CUR);
					break;
				}
				chn_index = ch.cn[2] - '0' - 1;
				fread(&scaler, sizeof(int), 1, f);
				fread(voltage, sizeof(short), 1024, f);

				for (i=0; i<1024; i++) {
					// convert data to volts
					waveform[b][chn_index][i] = (voltage[i] / 65536. + eh.range/1000.0 - 0.5);

					// calculate time for this cell
					for (j=0,time[b][chn_index][i]=0; j<i; j++)
						time[b][chn_index][i] += bin_width[b][chn_index][(j+tch.trigger_cell) % 1024];
				}
			}

			// align cell #0 of all channels
			t1 = time[b][0][(1024-tch.trigger_cell) % 1024];
			for (chn=1; chn<4; chn++) {
				t2 = time[b][chn][(1024-tch.trigger_cell) % 1024];
				dt = t1 - t2;
				for (i=0; i<1024; i++)
					time[b][chn][i] += dt;
			}
			t1 = t2 = 0;

			// for (int c = 0; c < 4; c++) {
			//
			// }

			moving_average(waveform[b][0], time[b][0]);
			rise_time = rise_time(waveform[b][0], time[b][0]);
      amplitude = amplitude(waveform[b][0]);
			charge = charge(waveform[b][0], time[b][0]);

			// calculate distance of peaks with statistics
			// if (t1 > 0 && t2 > 0) {
			// 	ndt++;
			// 	dt = t2 - t1;
			// 	sumdt += dt;
			// 	sumdt2 += dt*dt;
			// }




			// fill root tree
			rec->Fill();
		} //loop over different boards

	} //loop over events
	stopwatch->Stop();
	printf("%d events processed, written in %f seconds.\n", n, stopwatch->CpuTime());

	// save and close root file
	rec->Write();
	outfile->Close();
	return rec;
}

void moving_average(double channel_waveform[], double channel_time[N_BINS]) {
	int period = 50;
	for (int i=0; i<N_BINS - (period + 2); i++){
		for ( int n = 1; n<period; n++ ){
			channel_waveform[i] += channel_waveform[i + n];
		}
		channel_waveform[i] = channel_waveform[i] / period;

	}
	//return channel_waveform;
}

double rise_time(double channel_waveform[N_BINS], double channel_time[N_BINS]) {
  int i;
	for (i=0; i<N_BINS - 2; i++){
		if ((channel_waveform[i] < PEAK_THRESHOLD) && channel_waveform[i+1] >= PEAK_THRESHOLD) {
			return (
				(PEAK_THRESHOLD-channel_waveform[i])
				/ (channel_waveform[i+1]-channel_waveform[i])
				* (channel_time[i+1]-channel_time[i])
				) + channel_time[i];
		}
  }
	return 0;
}

double amplitude(double channel_waveform[N_BINS]){
  int i;
	//Find baseline for channel 3 to get amplitude for ch3
	double sum = 0.0;
	for (i=0; i<10; i++) {
		sum+=channel_waveform[i];
	}
	double baseline = sum/10;
	//Find amplitude for channel 3 (this is example channel )
	double max = -10000.0;
	for (i=0; i<N_BINS - 2; i++) {
		if (channel_waveform[i]>max) {
			max=channel_waveform[i];
		}
	}
	return max;
}

double charge(double channel_waveform[N_BINS], double channel_time[N_BINS]){ //Perform reimann sum
	int i;
	double sum = 0.0;
	for (i=0; i<N_BINS - 2; i++) {
		sum += (channel_waveform[i] + channel_waveform[i+1]) * (channel_time[i] + channel_time[i+1]) / 2;
	}
	return sum / RESISTANCE;
}
