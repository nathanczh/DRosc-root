#include <TH1D.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TString.h>

void plot(TTree * results){
  TCanvas *c1 = new TCanvas();
  // c1->Reset();
  // c1->SetStyle("Plain");
  // gStyle->SetOptFit(1);

  TTree *stats = new TTree("stats","Event Statistics");

  TH1D* hE3 = new TH1D("hE3","Pulse height distribution (energy)",50,0.015,0.11);
  TString channel_number;
  channel_number.Form("%d", 1);
  results->Draw("amplitude" + channel_number + ">>hE3");
  // rec->Draw("amplitude2>>hE3");
  hE3->Draw();
  hE3->GetXaxis()->SetTitle("Voltage caused (mV)");
  hE3->GetYaxis()->SetTitle("Counts");
}
