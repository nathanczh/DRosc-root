#include <string.h>

#include <TSystem.h>
#include "TTree.h"

#include "read_binary.C"
#include "plot.C"

TString last_filename;
int run (char function, TString filename) {
  TTree * results;
  TFile *file;
  // char const * str = "Run_Light_29_5V_5mV_10mV_4_2mm_Fibers";
  last_filename = filename;
  switch (function) {
    case 'd':
      decode(filename);
    break;
    case 'p':
      file = new TFile(filename + ".root", "READ");
      results = (TTree*) file->Get("rec");
      plot(results);
    break;
    case 'a':
    default:
      if (gSystem->AccessPathName(filename + ".root")){ // Bizzare return value - AccessPathName
        results = decode(filename);
      } else {
        file = new TFile(filename + ".root", "READ");
        results = (TTree*) file->Get("rec");
      }
      plot(results);
    break;
  }


  return 1;
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
