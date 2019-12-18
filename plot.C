#include "app.h"

void plot(TTree * results, TTree * event_stats, int channel_number, TString plot_attr){
  // TCanvas * c1 = new TCanvas();
  // c1
  // c1.SetStyle("Plain");
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
    printf("HERRE\n");
    graph->SetTitle("Pulses for Particle Detector");
    // draw graph and wait for user click
    graph->GetXaxis()->SetTitle("Time (ns)");
    graph->GetYaxis()->SetTitle("Voltage (V)");
    graph->Draw("ACP");
    printf("HERRE3\n");

  } else {
    EventStats event;
    int n_events = event_stats->GetEntries();
    double plot_stats[n_events];
    event_stats->GetBranch((new BoardId(0,1))->str())->SetAddress(&event);
    double * value = 0;
    double lower_bound = .01;
    double upper_bound = .12;
    int bins = n_events / 100;
    if (plot_attr == "amplitude") {
      value = &event.amplitude;
    } else if (plot_attr == "rise_time") {
      value = &event.rise_time;
    } else if (plot_attr == "charge") {
      value = &event.charge;
    } else if (plot_attr == "diff") {
      value = &event.difference;
      lower_bound = -100;
      upper_bound = 100;
      bins = 70;
    }


    TH1D * hist = new TH1D("hist", plot_attr + channel_number_str, bins,lower_bound,upper_bound);
    for (int i = 0; i < n_events; i++) {
      event_stats->GetEntry(i);
      if(!std::isnan(*value)){
        hist->Fill(*value);
      }

      // printf("%f\n", *value);
    }

    double mean = hist->GetMean();
    double rms = hist->GetRMS();
    ofstream output;
    // output.open("../Experimental Data/risetime.txt", std::ios_base::app);
    // output << (std::to_string(PEAK_THRESHOLD) + "\t" + std::to_string(mean) + "\t" + std::to_string(rms) + "\t" + std::to_string(hist->GetEntries()) + "\n");
    // output.close();
    double min, max;
    hist->GetMinimumAndMaximum(min, max);

#define RANGE 1.10
    if (false) {
    } else{
      lower_bound = (mean - min) * (1-RANGE) + min;
      upper_bound = max - (max - mean) * (1-RANGE);
    }

    printf("%d bins from [%f, %f]\n", bins, min, max);
    // hist->SetBins(bins, -10000, 12000);

    hist->Draw();
    hist->GetXaxis()->SetTitle("Parameter");
    hist->GetYaxis()->SetTitle("Counts");
  }
}
