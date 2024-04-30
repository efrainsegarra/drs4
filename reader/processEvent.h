#ifndef PROCESSEVENT_H
#define PROCESSEVENT_H

#include <expat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "TFile.h"
#include "TTree.h"

#ifdef XML_LARGE_SIZE
#  define XML_FMT_INT_MOD "ll"
#else
#  define XML_FMT_INT_MOD "l"
#endif

#ifdef XML_UNICODE_WCHAR_T
#  define XML_FMT_STR "ls"
#else
#  define XML_FMT_STR "s"
#endif

using namespace std;
void reset_char_data_buffer ();
void process_char_data_buffer ();


TFile * outfile = new TFile("test.root","RECREATE");
TTree * outtree = new TTree("drs4","Test skim of DRS4 evaluation board");
const int Max_mult 	= 3;
int 		Event = 0;
int 	 	Mult;
double  	Delta_t;
double	 	Charge[Max_mult];
double		Width[Max_mult];
double		Amp[Max_mult];
double		Rise_time[Max_mult];
double		Baseline[Max_mult];
int		Saturated[Max_mult];
double		Start_time;
int		Channel[Max_mult];
double		Time[Max_mult];

struct waveform {
	std::vector<double> t;
	std::vector<double> V;

};
struct event {
	waveform ch1;
	waveform ch3;
	waveform ch4;
	bool storeCh1 = false;
	bool storeCh3 = false;
	bool storeCh4 = false;
} thisEvent;


const double baseline_t = 175; // ns
const double gain = -1; // flip waveform
const double saturation_guess = 450; // mV
const double threshold = 15; // mV
void processWaveform( waveform * thisWave, TTree * outtree ){
	
	std::vector<double>* t = &(thisWave->t);
	std::vector<double>* V = &(thisWave->V);

	// Define the start time
	double start_time = t->at(0);

	double q_baseline = 0;
	double num_baseline = 0;
	double q = 0;
	double num = 0;
	double A = 0;
	double peak_time = 0;
	double prev_val = V->at(0)*gain;
	bool saturated = false;

	for( int i = 0; i < t->size() ; i++ ){
		double thisT = t->at(i) - start_time;
		double thisV = V->at(i) * gain;

		if( thisT > baseline_t ){
			q_baseline += thisV; 		// add baseline for long-t values of waveform
			num_baseline++;			// tracker for counts so we know how to scale baseline
		}

		//if( thisV < threshold ) continue;	// don't count below threshold integral

		if( thisV > A ){
			A = thisV;
			peak_time = thisT;
		}
		if( i > 0 && thisV == prev_val && thisV > saturation_guess ){
			saturated = true;
		}

		q += thisV; 		// sum up entire waveform
		num += 1; 		// tracker for counts so we know how to scale baseline later
	}
	// Now we need to correct for the baseline to Q,A:
	A -= (q_baseline / num_baseline); 	// corrects by average baseline
	q -= (q_baseline / num_baseline * num); // corrects by average baseline under entire waveform

	Amp[Mult] = A;
	Charge[Mult] = q;
	Saturated[Mult] = saturated;
	Baseline[Mult] = q_baseline/num_baseline;
	Rise_time[Mult] = peak_time;

	//cout << A << " " << q << " " << peak_time << " " << q_baseline/num_baseline << " " << saturated << " ";
	

	return;
};

void processEvent( event * thisEvent , TTree * outtree ){

	waveform * ch1 = &(thisEvent->ch1);
	waveform * ch3 = &(thisEvent->ch3);
	waveform * ch4 = &(thisEvent->ch4);

	processWaveform( ch1 , outtree );
	Channel[Mult]=1;
	Mult++;
	processWaveform( ch3 , outtree );
	Channel[Mult]=3;
	Mult++;
	processWaveform( ch4 , outtree );
	Channel[Mult]=4;
	Mult++;

	Event++;

	if( Event % 1000 == 0 ) cout << "On event " << Event << "\n";

	outtree->Fill();

	//cout << "\n";
	
	return;
}



#endif
