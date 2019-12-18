#include "process_data.h"

TTree * process_data(TTree * event_data){
  printf("%f\n", PEAK_THRESHOLD);
  TObjArray * branches = event_data->GetListOfBranches();

  int n_branches = branches->GetEntries();
  TTree * event_stats = new TTree("event_stats", "event_stats");
  std::map<TString, EventData> local_event_data;
  std::map<TString, EventStats> local_event_stats;

  for (int i = 0; i < n_branches; i++) {
    TBranch * branch = (TBranch *) branches->At(i);
    TString branch_name(branch->GetName());

    local_event_stats.insert(std::pair<TString, EventStats>(branch_name, *(new EventStats())));

    event_stats->Branch(
      branch_name.Data(), &local_event_stats[branch_name],
      "amplitude/D:peak_time:charge:rise_time:difference"
    );

    local_event_data.insert(std::pair<TString, EventData>(branch_name, *(new EventData())));
    branch->SetAddress(&local_event_data[branch_name]);
  }


  int n_leaves = event_data->GetEntries();
  for (int j = 0; j < n_leaves; j++){
    for (int i = 0; i < n_branches; i++) {
      event_data->GetEntry(j);
      TString branch_name(branches->At(i)->GetName());
      EventData * current_event = &local_event_data[branch_name];
      EventStats * current_stats = &local_event_stats[branch_name];

      /* Data Processing */
      // baseline(current_event->waveform);
      moving_average(current_event->time, current_event->waveform);

      /* Statistic Gathering */
      current_stats->rise_time = rise_time(current_event->waveform, current_event->time);
      current_stats->peak_time = peak_time(current_event->waveform, current_event->time);
      current_stats->amplitude = amplitude(current_event->waveform);
      current_stats->charge = charge(current_event->waveform, current_event->time);
      current_stats->difference = (local_event_stats[*(BoardId(0,0).str())]).peak_time - current_stats->peak_time;
      current_stats->difference = (local_event_stats[*(BoardId(0,0).str())]).rise_time - current_stats->rise_time;
    }

    event_stats->Fill();
  };

  EventStats event;
  event_stats->GetBranch((new BoardId(0,1))->str())->SetAddress(&event);
  double * diff =  &event.difference;
  double * rise =  &event.rise_time;
  event_stats->GetEntry(120);
  printf("Diff: %f; Rise: %f;;;%f\n", *diff, *rise, PEAK_THRESHOLD);

  return event_stats;
}

// void baseline(double channel_waveform[]){
//   double avg = 0;
//   for (int i = 0; i < 20; i++) {
//     avg += channel_waveform[i];
//   }
// }

void moving_average(double channel_time[], double channel_waveform[N_BINS]) {
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
	return std::numeric_limits<double>::quiet_NaN();
}


// Assumes curve has been smoothened.
double peak_time(double wave[N_BINS], double time[N_BINS]) {
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
