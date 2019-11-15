#include "run.h"

TString last_filename;
int run (char fn, TString filename, int channel_number, TString plot_attr) {
  TTree * results;
  TFile *file;
  // char const * str = "Run_Light_29_5V_5mV_10mV_4_2mm_Fibers";
  last_filename = filename;

  if ( fn == 'd' || fn == 'f' || gSystem->AccessPathName(filename + ".root") ){
    results = drsosc_read(filename);
  } else {
    file = new TFile(filename + ".root", "READ");
    results = (TTree*) file->Get("event_data");
  }

  if ( fn == 'd') {
    return 1;
  }

  // write_to_file(&filename, results);
  TTree * stats = process_data(results);
  plot(stats, channel_number, plot_attr);

  return 1;
}

void write_to_file(TString * name, TTree * tree) {
	TFile* outfile = new TFile(name->Append(".root"), "RECREATE");
  tree->Write();
  outfile->Close();
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
