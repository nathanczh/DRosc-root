#include "process_data.h"

TTree * process_data(TTree * event_data){
  TObjArray * branches = event_data->GetListOfBranches();

  int n_branches = branches->GetEntries();
  TTree * event_stats = new TTree("event_stats", "event_stats");
  TMap * local_event_data = new TMap();
  TMap * local_event_stats = new TMap();

  for (int i = 0; i < n_branches; i++) {
    TBranch * branch = (TBranch *) branches->At(i);
    TObjString * branch_name = new TObjString(branch->GetName());

    local_event_stats->Add(branch_name, new EventStats());

    event_stats->Branch(branch_name->GetString(), (EventStats *) local_event_stats->GetValue(branch_name), "amplitude/D:charge/D:rise_time/D:difference/D");

    local_event_data->Add(branch_name, new EventData());
    branch->SetAddress((EventData *) local_event_data->GetValue(branch_name));
  }


  int n_leaves = event_data->GetEntries();
  for (int j = 0; j < n_leaves; j++){
    for (int i = 0; i < n_branches; i++) {
      event_data->GetEntry(j);
      TObjString * branch_name = new TObjString(branches->At(i)->GetName());
      EventData * current_event = (EventData *) local_event_data->GetValue(branch_name);
      EventStats * current_stats = (EventStats *) local_event_stats->GetValue(branch_name);

      /* Data Processing */
      moving_average(current_event->time,current_event->waveform);

      /* Statistic Gathering */
      current_stats->rise_time = rise_time(current_event->waveform, current_event->time);
      current_stats->amplitude = amplitude(current_event->waveform);
      current_stats->charge = charge(current_event->waveform, current_event->time);
      current_stats->difference = ((EventStats *) local_event_stats->GetValue(BoardId(0,0).str()))->rise_time - current_stats->rise_time;
      // printf("%f - %f\n", ((EventStats *) local_event_stats->GetValue(BoardId(0,0).str()))->rise_time, current_stats->rise_time);
    }
    event_stats->Fill();
  };

  EventStats event;
  int n_events = event_stats->GetEntries();
  event_stats->GetBranch((new BoardId(0,1))->str())->SetAddress(&event);
  double * value = 0;
  value = &(event.difference);

  for (int i = 0; i < n_events; i++) {
    event_stats->GetEntry(i);
  }
  printf("Diff: %f\n", *value);

  return event_stats;
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
//
// double rise_time(double channel_waveform[N_BINS], double channel_time[N_BINS]) {
//   int i;
// 	for (i=0; i<N_BINS - 2; i++){
// 		if ((channel_waveform[i] < PEAK_THRESHOLD) && channel_waveform[i+1] >= PEAK_THRESHOLD) {
// 			return (
// 				(PEAK_THRESHOLD-channel_waveform[i])
// 				/ (channel_waveform[i+1]-channel_waveform[i])
// 				* (channel_time[i+1]-channel_time[i])
// 				) + channel_time[i];
// 		}
//   }
// 	return 0;
// }


// Assumes curve has been smoothened.
double rise_time(double wave[N_BINS], double time[N_BINS]) {
  int i;
  double max_dertime = 0;
  double max_derivative = 0;
  double derivatives[N_BINS];
	for (i=0; i<N_BINS - 1; i++){
    derivatives[i] = (wave[i + 1] - wave[i]) / (time[i+1] - time[i]);
    if ( i  >= 9 ) {
      double avg_derivative = 0;
      for ( int j = 0; j < 10; j++) {
        avg_derivative += derivatives[i - j];
      }
      if ( max_derivative < avg_derivative ) {
        max_derivative = avg_derivative;
        max_dertime = time[ i - 4 ];
      }
    }
  }
	return max_dertime;
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
