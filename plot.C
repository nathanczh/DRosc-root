#include "app.h"

void plot(TTree * results, int channel_number, TString plot_attr){
  TCanvas *c1 = new TCanvas();
  // c1->Reset();
  // c1->SetStyle("Plain");
  // gStyle->SetOptFit(1);
    TString channel_number_str;
    channel_number_str.Form("%d", channel_number);

  if (plot_attr == "waveform") {
    int n = 1024;
    Double_t x[n], y[n];
    BoardId board_id(0,0);
    TBranch * branch = results->GetBranch(board_id.str());
    EventData event;
    branch->SetAddress(&event);
    results->GetEvent(10);
    TGraph *graph  = new TGraph(n, event.time, event.waveform);
    graph->SetTitle("Pulses for Particle Detector");
    // draw graph and wait for user click
    graph->GetXaxis()->SetTitle("Time (ns)");
    graph->GetYaxis()->SetTitle("Voltage (V)");
    graph->Draw("ACP");

  }
  if ( plot_attr == "cmp" ) {

  } else {

    TTree * event_stats = results;
    EventStats event;
    int n_events = event_stats->GetEntries();
    double plot_stats[n_events];
    TH1D* hE3 = new TH1D("hE3", plot_attr + channel_number_str, 200,-1,+1);
    event_stats->GetBranch((new BoardId(0,1))->str())->SetAddress(&event);
    double * value = 0;
    if (plot_attr == "amplitude") {
      value = &event.amplitude;
    } else if (plot_attr == "rise_time") {
      value = &event.rise_time;
    } else if (plot_attr == "charge") {
      value = &event.charge;
    } else if (plot_attr == "diff") {
      value = &event.difference;
    }

    for (int i = 0; i < n_events; i++) {
      event_stats->GetEntry(i);
      hE3->Fill(*value);
    }



    // results->Draw( (new BoardId(0,0))->str() + "." + plot_attr + ">>hE3");
    hE3->Draw();
    hE3->GetXaxis()->SetTitle("Voltage caused (mV)");
    hE3->GetYaxis()->SetTitle("Counts");
  }
}
