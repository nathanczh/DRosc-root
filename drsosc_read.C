#include "drsosc_read.h"
#include <iostream>


TTree * drsosc_read(TString filename) {

	FHEADER fh;
	THEADER th;
	BHEADER bh;
	EHEADER eh;
	TCHEADER tch;
	CHEADER ch;

	TStopwatch* stopwatch = new TStopwatch();

	unsigned int scaler;
	unsigned short voltage[N_BINS];
	TTree * event_data = new TTree("event_data", "event_data");
	std::map<BoardId, EventData> local_event_data;
	float bin_width[MAX_N_BOARDS][N_CHANNELS][N_BINS];
	int i, j, b, chn, n, chn_index, n_boards;

	TString data_filename = (filename + ".dat");
	FILE *f = fopen(data_filename.Data(), "rb");
	printf("Opened file %s\n", data_filename.Data());

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
			EventData * single_event = new EventData();
			BoardId board_id(b, chn);
			local_event_data.insert(std::pair<BoardId, EventData>(board_id, *single_event));
			event_data->Branch(board_id.str(), &(local_event_data[board_id]), "time[1024]/D:waveform[1024]/D");
			fread(&bin_width[b][i][0], sizeof(float), 1024, f);
			// fix for 2048 bin mode: double channel
			if (bin_width[b][i][1023] > 10 || bin_width[b][i][1023] < 0.01) {
				for (j=0; j<512; j++)
					bin_width[b][i][j+512] = bin_width[b][i][j];
			}
		}
	}
	n_boards = b;


	// Actually put events in
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
				BoardId board_id(b, chn);
				EventData * current_event = &(local_event_data[board_id]);
				if (!current_event){
					break;
				}
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
					current_event->waveform[i] = (voltage[i] / 65536. + eh.range/1000.0 - 0.5);


					// calculate time for this cell
					for (j=0, current_event->time[i]=0; j<i; j++)
						current_event->time[i] += bin_width[b][chn_index][(j+tch.trigger_cell) % 1024];
				}
			}

			BoardId board_id(b, 0);
			EventData * ref_event =  &(local_event_data[board_id]);
			// align cell #0 of all channels
			double t1 = ref_event->time[(1024-tch.trigger_cell) % 1024];
			for (chn=1; chn<N_CHANNELS; chn++) {
				BoardId curr_board_id(b, chn);
				EventData * current_event = &(local_event_data[curr_board_id]);
				if(!current_event){
					break;
				}
				double t2 = current_event->time[(1024-tch.trigger_cell) % 1024];
				double dt = t1 - t2;
				for (i=0; i<1024; i++)
					current_event->time[i] += dt;
			}
		} //loop over different boards
		event_data->Fill();
	} //loop over events

	stopwatch->Stop();
	printf("%d events read in %f seconds.\n", n, stopwatch->CpuTime());


	// EventData event;
	// event_data->SetBranchAddress((new BoardId(0,0))->str(), event.time);
	// event_data->GetEntry(0);
	// for (int i = 0; i < 1024; i++) {
	// 	printf("%f\n", event.time[i]);
	// }

	return event_data;
}
