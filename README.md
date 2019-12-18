# Crystal Scintillator Characterization Analysis Tool

This is an application written with ROOT CERN with the objective of providing easy access to the statstical gathering processes the program executes.

## Execution formatting

`run (char fn, TString filename, int channel_number, TString plot_attr)`
`run(TString filename, int channel_number, TString plot_attr)`
`run(int channel_number, TString plot_attr)`
`run (char function, TString filename)`
`run(TString filename)`
`run()`

The code can be run with the command:

`x run.C+([parameters])`, with the parameters listed below:

`char fn` Can be one of the following:
 - `'a'` (default) Regular: Create root file if it does not exist, and run analysis
 - `'d'` Decode only: Only read TTree from .dat file
 - `'f'` Force: Force entire process

`TString filename` filename of dat file, without .dat extension
`int channel_number` channel to print analysis of
`TString plot_attr` statistic to plot on diagram

## File Structure

The program consists of the following files:

`drsosc_read.C`: Code to read and create TTree from .dat file
`process_data.C`: Calculate statistics from TTree
