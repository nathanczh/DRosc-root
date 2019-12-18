#include "run.h"

TString last_filename;
int run (char fn, TString name, int channel_number, TString plot_attr) {
  TString filename = name.Copy().Append(".root");
	// TFile* file = new TFile(filename, "UPDATE");
  TTree * results;

  if ( 1 || fn == 'd' || fn == 'f' || gSystem->AccessPathName(filename) ){
    results = drsosc_read(name);
    // results->Write();
  }
  // results = (TTree*) file->Get("event_data");
  // if (!results){
  //   printf("NULL POINTER READING FROM FILE");
  //   return 0;
  // }

  if ( fn == 'd') {
    return 1;
  }

  // for( double j = 0.014; j < 10; j+= .001 ) {
    // PEAK_THRESHOLD = .005;
    PEAK_THRESHOLD = .01;
    // PEAK_THRESHOLD = j;
    TTree * stats = process_data(results);
    plot(results, stats, channel_number, plot_attr);
  // }
  return 1;
}


int run (char function, TString filename){
  return run(function, filename, 1, "amplitude");
}

int run(TString filename){
  return run('a', filename);
}

int run(){
  if (!last_filename.IsNull()) {
    run('a', last_filename);
  } else {
    printf("Need to provide filename\n");
    return 0;
  }
  return 1;
}

int run(int channel_number, TString plot_attr){
  if (!last_filename.IsNull()) {
    run('a', last_filename, channel_number, plot_attr);
  } else {
    printf("Need to provide filename\n");
    return 0;
  }
  return 1;
}


int run(TString filename, int channel_number, TString plot_attr){
  return run('a', filename, channel_number, plot_attr);
}
