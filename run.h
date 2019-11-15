#ifndef RUN_H
#define RUN_H

#include <string.h>

#include <TSystem.h>
#include "TTree.h"
#include "TFile.h"

#include "drsosc_read.C"
#include "process_data.C"
#include "plot.C"


void write_to_file(TString * name, TTree * tree);
int run (char fn, TString filename, int channel_number, TString plot_attr);
int run (char function, TString filename);
int run(TString filename);
int run();
int run(int channel_number, TString plot_attr);
int run(TString filename, int channel_number, TString plot_attr);

#endif
