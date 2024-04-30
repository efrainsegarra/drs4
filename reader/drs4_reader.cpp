
#include "processEvent.h"




static void XMLCALL
startElement(void *userData, const XML_Char *name, const XML_Char **atts) {
	int i;
	int *const depthPtr = (int *)userData;

	if( strcmp(name,"Event")==0 ){
		//cout << "Starting event reading\n";
	}
	else if( strcmp(name,"CHN1")==0 ){
		thisEvent.storeCh1 = true;
		//cout << "\tChannel 1 data...\n";
	}
	else if( strcmp(name,"CHN3")==0 ){
		thisEvent.storeCh3 = true;
		//cout << "\tChannel 3 data...\n";
	}
	else if( strcmp(name,"CHN4")==0 ){
		thisEvent.storeCh4 = true;
		//cout << "\tChannel 4 data...\n";
	}
	/*
	for (i = 0; i < *depthPtr; i++)
		printf("  ");

	printf("%" XML_FMT_STR, name);

	for (i = 0; atts[i]; i += 2) {
		printf(" %" XML_FMT_STR "='%" XML_FMT_STR "'", atts[i], atts[i + 1]);
	}

	printf("\n");
	*/
	*depthPtr += 1;
}

static void XMLCALL
endElement(void *userData, const XML_Char *name) {
	int *const depthPtr = (int *)userData;
	(void)name;
	process_char_data_buffer();
	reset_char_data_buffer();

	if( strcmp(name,"CHN1")==0 ){
		thisEvent.storeCh1 = false;
		//cout << "\tDone reading channel 1 data...\n";
	}
	else if( strcmp(name,"CHN3")==0 ){
		thisEvent.storeCh3 = false;
		//cout << "\tDone reading channel 3 data...\n";
	}
	else if( strcmp(name,"CHN4")==0 ){
		thisEvent.storeCh4 = false;
		//cout << "\tDone reading channel 4 data...\n";
	}
	if( strcmp(name,"Event")==0 ){
		//cout << "Done with event reading. Clearing out struct\n\n";

		// Process event waveform:
		processEvent( &thisEvent , outtree );

		thisEvent.ch1.t.clear();
		thisEvent.ch1.V.clear();
		thisEvent.ch3.t.clear();
		thisEvent.ch3.V.clear();
		thisEvent.ch4.t.clear();
		thisEvent.ch4.V.clear();

		thisEvent.storeCh1 = false;
		thisEvent.storeCh3 = false;
		thisEvent.storeCh4 = false;

		Mult = 0;
		Delta_t = 0;
		memset(Charge,0,sizeof(Charge));
		memset(Width,0,sizeof(Width));
		memset(Amp,0,sizeof(Amp));
		memset(Rise_time,0,sizeof(Rise_time));
		memset(Baseline,0,sizeof(Baseline));
		memset(Saturated,0,sizeof(Saturated));
		Start_time = 0;
		memset(Channel,0,sizeof(Channel));
		memset(Time,0,sizeof(Time));

		//exit(-1);
	}


	*depthPtr -= 1;
}


static char char_data_buffer[1024];
static size_t offs;
static bool overflow;

void reset_char_data_buffer (void) {
	offs = 0;
	overflow = false;
}
// pastes parts of the node together
void char_data (void *userData, const XML_Char *s, int len) {
	if (!overflow) {
		if (len + offs >= sizeof(char_data_buffer) ) {
			overflow = true;
		} else {
			memcpy(char_data_buffer + offs, s, len);
			offs += len;
		}
	}
}
void process_char_data_buffer (void) {
	if( thisEvent.storeCh1 == false && thisEvent.storeCh3 == false && thisEvent.storeCh4 == false ) return;
	if (offs > 0) {
		char_data_buffer[ offs ] = '\0';

		string buffer(char_data_buffer);
		buffer = buffer.substr(buffer.find('\n')+1,buffer.length()); // trim off the leading '\n'

		//cout << "\t\t new line: " << buffer << "\n";//buffer.substr(buffer.find('\n')+3,buffer.length()) << "\n";
		stringstream ss( buffer );
		string word;
		int ctr = 0;
		double voltage = 0;
		double time = 0;
		while( !ss.eof() ){
			getline(ss, word, ',');
			double val;
			if( word.find_first_not_of(' ')!=std::string::npos ){
				val = stod(word);
				if( ctr == 0 ){
					time = val;
					ctr += 1;
				}
				else if( ctr == 1 ){
					voltage = val;
				}
				//cout << val << "\t";
			}
		}
		if( time != 0 && voltage != 0 ){
			if( thisEvent.storeCh1 ){
				thisEvent.ch1.t.push_back( time );
				thisEvent.ch1.V.push_back( voltage );
			}
			else if( thisEvent.storeCh3 ){
				thisEvent.ch3.t.push_back( time );
				thisEvent.ch3.V.push_back( voltage );
			}
			else if( thisEvent.storeCh4 ){
				thisEvent.ch4.t.push_back( time );
				thisEvent.ch4.V.push_back( voltage );
			}
		}
	}
}


int main(int argc, char** argv) {
	outtree->Branch("event", 	&Event,		"event/I"		);
	outtree->Branch("mult",		&Mult,		"mult/I"		);
	outtree->Branch("delta_t",	&Delta_t,	"delta_t/D"		);
	outtree->Branch("start_t",	&Start_time,	"start_t/D"		);
	outtree->Branch("channel",	Channel,	"channel[mult]/I"	);
	outtree->Branch("time",		Time,		"time[mult]/D"		);
	outtree->Branch("q",		Charge,		"q[mult]/D"		);
	outtree->Branch("a",		Amp,		"a[mult]/D"		);
	outtree->Branch("rise_t",	Rise_time,	"rise_t[mult]/D"	);
	outtree->Branch("width_t",	Width,		"width_t[mult]/D"	);
	outtree->Branch("q_base",	Baseline,	"q_base[mult]/D"	);
	outtree->Branch("saturated",	Saturated,	"saturated[mult]/I"	);


	XML_Parser parser = XML_ParserCreate(NULL);
	int done;
	int depth = 0;

	if (! parser) {
		fprintf(stderr, "Couldn't allocate memory for parser\n");
		return 1;
	}

	XML_SetUserData(parser, &depth);
	XML_SetElementHandler(parser, startElement, endElement);
	XML_SetCharacterDataHandler(parser, char_data);

	reset_char_data_buffer();


	do {
		void *const buf = XML_GetBuffer(parser, BUFSIZ);
		if (! buf) {
			fprintf(stderr, "Couldn't allocate memory for buffer\n");
			XML_ParserFree(parser);
			return 1;
		}

		const size_t len = fread(buf, 1, BUFSIZ, stdin);

		if (ferror(stdin)) {
			fprintf(stderr, "Read error\n");
			XML_ParserFree(parser);
			return 1;
		}

		done = feof(stdin);

		if (XML_ParseBuffer(parser, (int)len, done) == XML_STATUS_ERROR) {
			fprintf(stderr,
					"Parse error at line %" XML_FMT_INT_MOD "u:\n%" XML_FMT_STR "\n",
					XML_GetCurrentLineNumber(parser),
					XML_ErrorString(XML_GetErrorCode(parser)));
			XML_ParserFree(parser);
			return 1;
		}

		if( Event > 100000 ) break;
	} while (! done);

	XML_ParserFree(parser);

	outfile->cd();
	outtree->Write();
	outfile->Close();
	return 0;
}
