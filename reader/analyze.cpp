#include <iostream>
#include <vector>
#include <algorithm>
#include "TFile.h"
#include "TTree.h"

using namespace std;
int main(int argc, char** argv) {
	
	TFile * inFile = new TFile(argv[1]);
	TTree * inTree = (TTree*) inFile->Get("drs4");
	
	double Wave[3][1024];
	int Coincidence;
	inTree->SetBranchAddress("wave",	Wave);
	inTree->SetBranchAddress("coincidence",	&Coincidence);

	std::vector<double> ch1[1024];
	std::vector<double> ch2[1024];
	std::vector<double> ch3[1024];

	for( int event = 0 ; event < inTree->GetEntries() ; event++ ){
		inTree->GetEntry(event);

		if( Coincidence != 1 ) continue;

		for( int i = 0 ; i < 1024 ; i++ ){
			ch1[i].push_back( Wave[0][i] );
			ch2[i].push_back( Wave[1][i] );
			ch3[i].push_back( Wave[2][i] );
		}
	}

	for( int i = 0 ; i < 1024 ; i++ ){
		std::sort( ch1[i].begin() , ch1[i].end() );
		std::sort( ch2[i].begin() , ch2[i].end() );
		std::sort( ch3[i].begin() , ch3[i].end() );

		// Now we can just grab the 16-50-84 percentiles
		int size = ch1[i].size();
		
		int i16 = 0.16 * size;
		int i50 = 0.5 * size;
		int i84 = 0.84 * size;

		double p16_1 = ch1[i][i16];
		double p50_1 = ch1[i][i50];
		double p84_1 = ch1[i][i84];
		double p16_2 = ch2[i][i16];
		double p50_2 = ch2[i][i50];
		double p84_2 = ch2[i][i84];
		double p16_3 = ch3[i][i16];
		double p50_3 = ch3[i][i50];
		double p84_3 = ch3[i][i84];
		
		cout << i << " " 
			<< p16_1 << " " << p50_1 << " " << p84_1 << " "
			<< p16_2 << " " << p50_2 << " " << p84_2 << " "
			<< p16_3 << " " << p50_3 << " " << p84_3 << "\n";
			
	}


	return 1;
}
