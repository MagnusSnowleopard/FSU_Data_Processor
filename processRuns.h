#ifndef PROCESSRUNS_H
#define PROCESSRUNS_H

#include <vector>
#include <map>
#include <iostream>

#include "TFile.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TTreeReaderArray.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCutG.h"
#include "TMath.h"

#include "mapinit.h"
#include "detcal.h"
#include "detectorshifts.h"
#include "globals.h"

//DO NOT CHANGE
#define NDET 15

struct DetectorHistograms {
	TH1F* tDiff   = nullptr;
	TH1F* tDiffg  = nullptr;
	TH1F* tDiffn  = nullptr;
	TH1F* ENeutron = nullptr;
	// TH1F* QVAL   = nullptr;
	// TH1F* EX     = nullptr;
	TH2F* PSD     = nullptr;
	TH2F* PSDg    = nullptr;
	TH2F* PSDn    = nullptr;
};

struct RunHistograms {
	std::vector<DetectorHistograms> detectors; // size = NDET
	bool isPrompt;
};

// Global TCutG arrays (consider wrapping in namespace instead of globals)
inline TCutG* gcuts[NDET] = {nullptr};
inline TCutG* ncuts[NDET] = {nullptr};

// You must provide these maps somewhere (mapinit.h)
extern const std::map<unsigned short, int> SN2Bd;
extern const std::map<unsigned short, int> Index2ID;
extern const int mapping[2][16];

//   detCalMap, Distance, Angle
extern std::map<int, std::pair<float,float>> detCalMap;
extern std::map<int, float> Distance;
extern std::map<int, float> Angle;

RunHistograms processRuns( int RunNumber, bool isPrompt, const std::vector<double>& detectorShifts){

	int even_counter =0;
	int odd_counter = 0;


	TString fileName = Form("run%d.root", runNumber);
	TFile file(fileName);

	if(file.IsZombie()){return {{},isPrompt}};

	TTree* tree = (TTree*)file.Get("tree");
	if(!tree){std::cerr << "No tree in file:" << fileName << std::endl;
		return {{}, isPrompt}};


	TTreeReader reader(tree);

	TTreeReaderValue<ULong64_t>  evID = {reader, "evID"};
	TTreeReaderValue<UInt_t>    multi = {reader, "multi"};
	TTreeReaderArray<UShort_t>     sn = {reader, "sn"}; // serial no. 
	TTreeReaderArray<UShort_t>     ch = {reader, "ch"}; // channel
	TTreeReaderArray<UShort_t>      e = {reader, "e"};  //long
	TTreeReaderArray<UShort_t>     e2 = {reader, "e2"}; //short
	TTreeReaderArray<ULong64_t>   e_t = {reader, "e_t"}; // in ns
	TTreeReaderArray<UShort_t>    e_f = {reader, "e_f"}; // in ps

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//MAKE TCUTG* HERE// Set a min x-val that gives a clear TOF for all detectors.  
	//STARTING ENERGY === 500 CHANGE HERE IF WANT TO CHANGE ALL ENERGY START/END FOR CUTS
	double E_START = 500.; // only valid with E_CAL present
	int i = 0;
	for(i = 0; i < NDET;i++){
		if(i==0){
			ncuts[i] = new TCutG(Form("ncut%d",i),7);
			ncuts[i]->SetVarX("eLong");
			ncuts[i]->SetVarY("PSDVAL");
			ncuts[i]->SetPoint(0,E_START,.23); //index , Energy_keV_start, psdval_start
			ncuts[i]->SetPoint(1,E_START,.15);
			ncuts[i]->SetPoint(2,1500.,.12);
			ncuts[i]->SetPoint(3,2500.,.12);
			ncuts[i]->SetPoint(4,2500.,.16);
			ncuts[i]->SetPoint(5,1000.,.20);
			ncuts[i]->SetPoint(6,E_START,.23); //final index, E_end = E_start, p_end = p_start
		}
		if(i==0){
			gcuts[i] = new TCutG(Form("gcut%d",i),7);
			gcuts[i]->SetVarX("eLong");
			gcuts[i]->SetVarY("PSDVAL");
			gcuts[i]->SetPoint(0,E_START,.1); //index , Energy_keV_start, psdval_start
			gcuts[i]->SetPoint(1,E_START,.04);
			gcuts[i]->SetPoint(2,2000.,.06);
			gcuts[i]->SetPoint(3,3000.,.06);
			gcuts[i]->SetPoint(4,3000.,.09);
			gcuts[i]->SetPoint(5,1000.,.1);
			gcuts[i]->SetPoint(6,E_START,.1); //final index, E_end = E_start, p_end = p_start

		}
		if(i==1){
			ncuts[i] = new TCutG(Form("ncut%d",i),7);
			ncuts[i]->SetVarX("eLong");
			ncuts[i]->SetVarY("PSDVAL");
			ncuts[i]->SetPoint(0,E_START,.23);
			ncuts[i]->SetPoint(1,E_START,.15);
			ncuts[i]->SetPoint(2,1000.,.17);

			ncuts[i]->SetPoint(3,1250.,.2);
			ncuts[i]->SetPoint(4,1250.,.24);
			ncuts[i]->SetPoint(5,800.,.21);
			ncuts[i]->SetPoint(6,E_START,.23);
		}

		if(i==1){
			gcuts[i] = new TCutG(Form("gcut%d",i),7);
			gcuts[i]->SetVarX("eLong");
			gcuts[i]->SetVarY("PSDVAL");
			gcuts[i]->SetPoint(0,E_START,.1); //index , Energy_keV_start, psdval_start
			gcuts[i]->SetPoint(1,E_START,.05);
			gcuts[i]->SetPoint(2,1000.,.08);
			gcuts[i]->SetPoint(3,1000.,.13);
			gcuts[i]->SetPoint(4,750.,.11);
			gcuts[i]->SetPoint(5,650.,.1);
			gcuts[i]->SetPoint(6,E_START,.1); //final index, E_end = E_start, p_end = p_start

		}
		if(i==2){
			ncuts[i] = new TCutG(Form("ncut%d",i),7);
			ncuts[i]->SetVarX("eLong");
			ncuts[i]->SetVarY("PSDVAL");
			ncuts[i]->SetPoint(0,E_START,.23);
			ncuts[i]->SetPoint(1,E_START,.15);
			ncuts[i]->SetPoint(2,1000.,.14);
			ncuts[i]->SetPoint(3,2000.,.125);
			ncuts[i]->SetPoint(4,2000.,.17);
			ncuts[i]->SetPoint(5,1000.,.20);
			ncuts[i]->SetPoint(6,E_START,.23);
		}

		if(i==2){
			gcuts[i] = new TCutG(Form("gcut%d",i),7);
			gcuts[i]->SetVarX("eLong");
			gcuts[i]->SetVarY("PSDVAL");
			gcuts[i]->SetPoint(0,E_START,.11); //index , Energy_keV_start, psdval_start
			gcuts[i]->SetPoint(1,E_START,.04);
			gcuts[i]->SetPoint(2,1500.,.06);
			gcuts[i]->SetPoint(3,2500.,.08);
			gcuts[i]->SetPoint(4,2500.,.11);
			gcuts[i]->SetPoint(5,1500.,.10);
			gcuts[i]->SetPoint(6,E_START,.11); //final index, E_end = E_start, p_end = p_start

		}
		if(i==3){
			ncuts[i] = new TCutG(Form("ncut%d",i),7);
			ncuts[i]->SetVarX("eLong");
			ncuts[i]->SetVarY("PSDVAL");
			ncuts[i]->SetPoint(0,E_START,.23);
			ncuts[i]->SetPoint(1,E_START,.15);
			ncuts[i]->SetPoint(2,1000.,.15);
			ncuts[i]->SetPoint(3,1250.,.2);
			ncuts[i]->SetPoint(4,1250.,.25);
			ncuts[i]->SetPoint(5,800.,.22);
			ncuts[i]->SetPoint(6,E_START,.23);
		}

		if(i==3){
			gcuts[i] = new TCutG(Form("gcut%d",i),7);
			gcuts[i]->SetVarX("eLong");
			gcuts[i]->SetVarY("PSDVAL");
			gcuts[i]->SetPoint(0,E_START,.1); //index , Energy_keV_start, psdval_start
			gcuts[i]->SetPoint(1,E_START,.04);
			gcuts[i]->SetPoint(2,1000.,.08);
			gcuts[i]->SetPoint(3,1000.,.13);
			gcuts[i]->SetPoint(4,750.,.11);
			gcuts[i]->SetPoint(5,650.,.1);
			gcuts[i]->SetPoint(6,E_START,.1); //final index, E_end = E_start, p_end = p_start

		}
		if(i==4){
			ncuts[i] = new TCutG(Form("ncut%d",i),7);
			ncuts[i]->SetVarX("eLong");
			ncuts[i]->SetVarY("PSDVAL");
			ncuts[i]->SetPoint(0,E_START,.22);
			ncuts[i]->SetPoint(1,E_START,.13);
			ncuts[i]->SetPoint(2,1000.,.16);
			ncuts[i]->SetPoint(3,1400.,.22);
			ncuts[i]->SetPoint(4,1400.,.265);
			ncuts[i]->SetPoint(5,1000.,.23);
			ncuts[i]->SetPoint(6,E_START,.22);
		}

		if(i==4){
			gcuts[i] = new TCutG(Form("gcut%d",i),7);
			gcuts[i]->SetVarX("eLong");
			gcuts[i]->SetVarY("PSDVAL");
			gcuts[i]->SetPoint(0,E_START,.10); //index , Energy_keV_start, psdval_start
			gcuts[i]->SetPoint(1,E_START,.04);
			gcuts[i]->SetPoint(2,1000.,.07);
			gcuts[i]->SetPoint(3,1200.,.1);
			gcuts[i]->SetPoint(4,1200.,.15);
			gcuts[i]->SetPoint(5,1000.,.12);
			gcuts[i]->SetPoint(6,E_START,.10); //final index, E_end = E_start, p_end = p_start

		}
		if(i==5){
			ncuts[i] = new TCutG(Form("ncut%d",i),7);
			ncuts[i]->SetVarX("eLong");
			ncuts[i]->SetVarY("PSDVAL");
			ncuts[i]->SetPoint(0,E_START,.23);
			ncuts[i]->SetPoint(1,E_START,.13);
			ncuts[i]->SetPoint(2,1000.,.13);
			ncuts[i]->SetPoint(3,1600.,.13);
			ncuts[i]->SetPoint(4,1600.,.18);
			ncuts[i]->SetPoint(5,800.,.20);
			ncuts[i]->SetPoint(6,E_START,.23);
		}

		if(i==5){
			gcuts[i] = new TCutG(Form("gcut%d",i),7);
			gcuts[i]->SetVarX("eLong");
			gcuts[i]->SetVarY("PSDVAL");
			gcuts[i]->SetPoint(0,E_START,.10); //index , Energy_keV_start, psdval_start
			gcuts[i]->SetPoint(1,E_START,.04);
			gcuts[i]->SetPoint(2,1000.,.045);
			gcuts[i]->SetPoint(3,1500.,.061);
			gcuts[i]->SetPoint(4,1500.,.096);
			gcuts[i]->SetPoint(5,1000.,.09);
			gcuts[i]->SetPoint(6,E_START,.10); //final index, E_end = E_start, p_end = p_start

		}
		if(i==6){
			ncuts[i] = new TCutG(Form("ncut%d",i),7);
			ncuts[i]->SetVarX("eLong");
			ncuts[i]->SetVarY("PSDVAL");
			ncuts[i]->SetPoint(0,E_START,.23);
			ncuts[i]->SetPoint(1,E_START,.14);
			ncuts[i]->SetPoint(2,1000.,.13);
			ncuts[i]->SetPoint(3,1700.,.125);
			ncuts[i]->SetPoint(4,1700.,.185);
			ncuts[i]->SetPoint(5,1000.,.21);
			ncuts[i]->SetPoint(6,E_START,.23);
		}

		if(i==6){
			gcuts[i] = new TCutG(Form("gcut%d",i),7);
			gcuts[i]->SetVarX("eLong");
			gcuts[i]->SetVarY("PSDVAL");
			gcuts[i]->SetPoint(0,E_START,.11); //index , Energy_keV_start, psdval_start
			gcuts[i]->SetPoint(1,E_START,.04);
			gcuts[i]->SetPoint(2,1000.,.05);
			gcuts[i]->SetPoint(3,1800.,.07);
			gcuts[i]->SetPoint(4,1800.,.107);
			gcuts[i]->SetPoint(5,1000.,.1);
			gcuts[i]->SetPoint(6,E_START,.11); //final index, E_end = E_start, p_end = p_start

		}
		if(i==7){
			ncuts[i] = new TCutG(Form("ncut%d",i),7);
			ncuts[i]->SetVarX("eLong");
			ncuts[i]->SetVarY("PSDVAL");
			ncuts[i]->SetPoint(0,E_START,.25);
			ncuts[i]->SetPoint(1,E_START,.15);
			ncuts[i]->SetPoint(2,1000.,.145);
			ncuts[i]->SetPoint(3,1400.,.180);
			ncuts[i]->SetPoint(4,1400.,.23);
			ncuts[i]->SetPoint(5,1000.,.21);
			ncuts[i]->SetPoint(6,E_START,.25);
		}

		if(i==7){
			gcuts[i] = new TCutG(Form("gcut%d",i),7);
			gcuts[i]->SetVarX("eLong");
			gcuts[i]->SetVarY("PSDVAL");
			gcuts[i]->SetPoint(0,E_START,.11); //index , Energy_keV_start, psdval_start
			gcuts[i]->SetPoint(1,E_START,.04);
			gcuts[i]->SetPoint(2,1000.,.064);
			gcuts[i]->SetPoint(3,1200.,.08);
			gcuts[i]->SetPoint(4,1200.,.125);
			gcuts[i]->SetPoint(5,1000.,.11);
			gcuts[i]->SetPoint(6,E_START,.11); //final index, E_end = E_start, p_end = p_start

		}
		if(i==8){
			ncuts[i] = new TCutG(Form("ncut%d",i),7);
			ncuts[i]->SetVarX("eLong");
			ncuts[i]->SetVarY("PSDVAL");
			ncuts[i]->SetPoint(0,E_START,.24);
			ncuts[i]->SetPoint(1,E_START,.15);
			ncuts[i]->SetPoint(2,1000.,.19);
			ncuts[i]->SetPoint(3,1400.,.255);
			ncuts[i]->SetPoint(4,1400.,.3);
			ncuts[i]->SetPoint(5,1000.,.25);
			ncuts[i]->SetPoint(6,E_START,.24);
		}

		if(i==8){
			gcuts[i] = new TCutG(Form("gcut%d",i),7);
			gcuts[i]->SetVarX("eLong");
			gcuts[i]->SetVarY("PSDVAL");
			gcuts[i]->SetPoint(0,E_START,.11); //index , Energy_keV_start, psdval_start
			gcuts[i]->SetPoint(1,E_START,.05);
			gcuts[i]->SetPoint(2,800.,.08);
			gcuts[i]->SetPoint(3,1100.,.115);
			gcuts[i]->SetPoint(4,1100.,.17);
			gcuts[i]->SetPoint(5,800.,.14);
			gcuts[i]->SetPoint(6,E_START,.11); //final index, E_end = E_start, p_end = p_start

		}
		if(i==9){
			ncuts[i] = new TCutG(Form("ncut%d",i),7);
			ncuts[i]->SetVarX("eLong");
			ncuts[i]->SetVarY("PSDVAL");
			ncuts[i]->SetPoint(0,E_START,.25);
			ncuts[i]->SetPoint(1,E_START,.15);
			ncuts[i]->SetPoint(2,1000.,.16);
			ncuts[i]->SetPoint(3,1400.,.235);
			ncuts[i]->SetPoint(4,1400.,.28);
			ncuts[i]->SetPoint(5,1000.,.23);
			ncuts[i]->SetPoint(6,E_START,.25);
		}

		if(i==9){
			gcuts[i] = new TCutG(Form("gcut%d",i),7);
			gcuts[i]->SetVarX("eLong");
			gcuts[i]->SetVarY("PSDVAL");
			gcuts[i]->SetPoint(0,E_START,.11); //index , Energy_keV_start, psdval_start
			gcuts[i]->SetPoint(1,E_START,.05);
			gcuts[i]->SetPoint(2,800.,.06);
			gcuts[i]->SetPoint(3,1100.,.1);
			gcuts[i]->SetPoint(4,1100.,.15);
			gcuts[i]->SetPoint(5,800.,.12);
			gcuts[i]->SetPoint(6,E_START,.11); //final index, E_end = E_start, p_end = p_start

		}
		//there is no detector 11 (index 10)
		if(i==10){
			ncuts[i] = new TCutG(Form("ncut%d",i),7);
			ncuts[i]->SetVarX("eLong");
			ncuts[i]->SetVarY("PSDVAL");
			ncuts[i]->SetPoint(0,E_START,.25);
			ncuts[i]->SetPoint(1,E_START,.15);
			ncuts[i]->SetPoint(2,1000.,.16);
			ncuts[i]->SetPoint(3,1400.,.235);
			ncuts[i]->SetPoint(4,1400.,.28);
			ncuts[i]->SetPoint(5,1000.,.23);
			ncuts[i]->SetPoint(6,E_START,.25);
		}

		if(i==10){
			gcuts[i] = new TCutG(Form("gcut%d",i),7);
			gcuts[i]->SetVarX("eLong");
			gcuts[i]->SetVarY("PSDVAL");
			gcuts[i]->SetPoint(0,E_START,.11); //index , Energy_keV_start, psdval_start
			gcuts[i]->SetPoint(1,E_START,.05);
			gcuts[i]->SetPoint(2,800.,.06);
			gcuts[i]->SetPoint(3,1100.,.1);
			gcuts[i]->SetPoint(4,1100.,.15);
			gcuts[i]->SetPoint(5,800.,.12);
			gcuts[i]->SetPoint(6,E_START,.11); //final index, E_end = E_start, p_end = p_start

		}
		if(i==11){
			ncuts[i] = new TCutG(Form("ncut%d",i),7);
			ncuts[i]->SetVarX("eLong");
			ncuts[i]->SetVarY("PSDVAL");
			ncuts[i]->SetPoint(0,E_START,.24);
			ncuts[i]->SetPoint(1,E_START,.15);
			ncuts[i]->SetPoint(2,1000.,.14);
			ncuts[i]->SetPoint(3,1600.,.14);
			ncuts[i]->SetPoint(4,1600.,.20);
			ncuts[i]->SetPoint(5,1000.,.20);
			ncuts[i]->SetPoint(6,E_START,.24);
		}

		if(i==11){
			gcuts[i] = new TCutG(Form("gcut%d",i),7);
			gcuts[i]->SetVarX("eLong");
			gcuts[i]->SetVarY("PSDVAL");
			gcuts[i]->SetPoint(0,E_START,.115); //index , Energy_keV_start, psdval_start
			gcuts[i]->SetPoint(1,E_START,.047);
			gcuts[i]->SetPoint(2,1000.,.06);
			gcuts[i]->SetPoint(3,1400.,.07);
			gcuts[i]->SetPoint(4,1400.,.105);
			gcuts[i]->SetPoint(5,800.,.10);
			gcuts[i]->SetPoint(6,E_START,.115); //final index, E_end = E_start, p_end = p_start

		}
		if(i==12){
			ncuts[i] = new TCutG(Form("ncut%d",i),7);
			ncuts[i]->SetVarX("eLong");
			ncuts[i]->SetVarY("PSDVAL");
			ncuts[i]->SetPoint(0,E_START,.24);
			ncuts[i]->SetPoint(1,E_START,.14);
			ncuts[i]->SetPoint(2,1000.,.13);
			ncuts[i]->SetPoint(3,2000.,.14);
			ncuts[i]->SetPoint(4,2000.,.19);
			ncuts[i]->SetPoint(5,1000.,.19);
			ncuts[i]->SetPoint(6,E_START,.24);
		}

		if(i==12){
			gcuts[i] = new TCutG(Form("gcut%d",i),7);
			gcuts[i]->SetVarX("eLong");
			gcuts[i]->SetVarY("PSDVAL");
			gcuts[i]->SetPoint(0,E_START,.115); //index , Energy_keV_start, psdval_start
			gcuts[i]->SetPoint(1,E_START,.04);
			gcuts[i]->SetPoint(2,1000.,.048);
			gcuts[i]->SetPoint(3,1800.,.07);
			gcuts[i]->SetPoint(4,1800.,.105);
			gcuts[i]->SetPoint(5,1000.,.10);
			gcuts[i]->SetPoint(6,E_START,.115); //final index, E_end = E_start, p_end = p_start

		}
		if(i==13){
			ncuts[i] = new TCutG(Form("ncut%d",i),7);
			ncuts[i]->SetVarX("eLong");
			ncuts[i]->SetVarY("PSDVAL");
			ncuts[i]->SetPoint(0,E_START,.24);
			ncuts[i]->SetPoint(1,E_START,.135);
			ncuts[i]->SetPoint(2,1000.,.125);
			ncuts[i]->SetPoint(3,1500.,.145);
			ncuts[i]->SetPoint(4,1500.,.19);
			ncuts[i]->SetPoint(5,1000.,.19);
			ncuts[i]->SetPoint(6,E_START,.24);
		}

		if(i==13){
			gcuts[i] = new TCutG(Form("gcut%d",i),7);
			gcuts[i]->SetVarX("eLong");
			gcuts[i]->SetVarY("PSDVAL");
			gcuts[i]->SetPoint(0,E_START,.11); //index , Energy_keV_start, psdval_start
			gcuts[i]->SetPoint(1,E_START,.04);
			gcuts[i]->SetPoint(2,1000.,.05);
			gcuts[i]->SetPoint(3,1400.,.07);
			gcuts[i]->SetPoint(4,1400.,.11);
			gcuts[i]->SetPoint(5,1000.,.095);
			gcuts[i]->SetPoint(6,E_START,.11); //final index, E_end = E_start, p_end = p_start

		}
		if(i==14){
			ncuts[i] = new TCutG(Form("ncut%d",i),7);
			ncuts[i]->SetVarX("eLong");
			ncuts[i]->SetVarY("PSDVAL");
			ncuts[i]->SetPoint(0,E_START,.25);
			ncuts[i]->SetPoint(1,E_START,.145);
			ncuts[i]->SetPoint(2,1000.,.14);
			ncuts[i]->SetPoint(3,1900.,.145);
			ncuts[i]->SetPoint(4,1900.,.19);
			ncuts[i]->SetPoint(5,1000.,.21);
			ncuts[i]->SetPoint(6,E_START,.25);
		}
		if(i==14){
			gcuts[i] = new TCutG(Form("gcut%d",i),7);
			gcuts[i]->SetVarX("eLong");
			gcuts[i]->SetVarY("PSDVAL");
			gcuts[i]->SetPoint(0,E_START,.11); //index , Energy_keV_start, psdval_start
			gcuts[i]->SetPoint(1,E_START,.04);
			gcuts[i]->SetPoint(2,1000.,.056);
			gcuts[i]->SetPoint(3,1800.,.075);
			gcuts[i]->SetPoint(4,1800.,.11);
			gcuts[i]->SetPoint(5,1000.,.11);
			gcuts[i]->SetPoint(6,E_START,.11); //final index, E_end = E_start, p_end = p_start

		}
	}
	/////////////End of cuts formation ///////////////////////////////////////////

	std::vector<DetectorHistograms> detectors(NDET);

	for(int i = 0; i < NDET; ++i){
		detectors[i].tDiff = new TH1F(Form("Tdiff-run%d-det%d",RunNumber, i+1), "", timeBin, timeRange[0], timeRange[1]);
		detectors[i].tDiffg = new TH1F(Form("Tdiffg-run%d-det%d",RunNumber, i+1),"", timeBin, timeRange[0], timeRange[1]);
		detectors[i].tDiffn = new TH1F(Form("Tdiffn-run%d-det%d",RunNumber, i+1),"", timeBin, timeRange[0], timeRange[1]);

		detectors[i].tDiffg->SetLineColor(2);

		detectors[i].Eneutron = new TH1F(Form("Eneutron-run%d-det%d",RunNumber, i+1), "",148 , -20, 20);
		//      detectors[i].QVAL = new TH1F(Form("QVAL-run%d-det%d",RunNumber, i+1), "",148 , -20, 20);
		//      detectors[i].EX = new TH1F(Form("EX-run%d-det%d",RunNumber, i+1), "",148 , -20, 20);

		detectors[i].PSD = new TH2F(Form("PSD-run%d-det%d",RunNumber, i),"", 4096, 0, max_E,512,-.1,1);
		detectors[i].PSDg = new TH2F(Form("PSDg-run%d-det%d",RunNumber, i),"", 4096, 0, max_E,512,-.1,1);
		detectors[i].PSDn = new TH2F(Form("PSDn-run%d-det%d",RunNumber, i),"", 4096, 0, max_E,512,-.1,1);
	}


	ULong64_t beammonitor=tree->GetEntries("ch==2 && sn==89");
	std::cout << "Beam monitor count for run # " << RunNumber << " = " << beammonitor << std::endl;
	while(reader.Next())
	{

		int count = 0;
		unsigned long long tRF = 0;
		std::vector<unsigned long long> tN(NDET,0);
		std::vector<float> eL(NDET,0),eS(NDET,0),eLcal(NDET,0),dist(NDET,0),angle(NDET,0);

		for( int j = 0; j < *multi; j++)
		{
			int bd = SN2Bd.at(sn[j]);
			int haha = mapping[bd][ch[j]];

			if( haha < 0 ) continue;

			int ID = Index2ID.at(haha);
			//std::cout << ID << std::endl;
			// printf("%d , ID %d\n", haha, ID);

			if( ID == 200)
			{
				count ++;
				tRF = ( e_t[j] * 1000. ) + e_f[j]; //in ps
								   //tRF = ( e_t[i] ) + e_f[i]; //in ps
			}

			if( ID >= 0 && ID < NDET ){

				//              std::cout << ID << std::endl;

				float A = detCalMap[ID+1].first;
				float B = detCalMap[ID+1].second;
				float D = Distance[ID+1];
				float theta = Angle[ID+1];
				eL[ID] =e[j];
				eS[ID] =e2[j];
				eLcal[ID] = A * e[j] + B;
				dist[ID] = D;
				angle[ID] = theta * TMath::Pi()/180.;

				tN[ID] = ( e_t[j] * 1000. ) + e_f[j]; //in ps
			}
		} // end multiplicity for loop


		for( int i = 0; i < NDET; i++)
		{
			if(i%2 == 0 && tN[i] > 0){
				odd_counter++;
				//      std::cout<< "ODD ch tN= "<< i << " , " << tN[i] << std::endl;
			}else if(i%2 ==1 && tN[i]){
				even_counter++;
				//      std::cout<< "EVEN ch tN= "<< i << " , " << tN[i] << std::endl;
			}




			if(/* tRF != 0 && */tN[i] != 0 ) // this condition takes out around 95% of the data in big 5, i think because tN int type 
			{
				double tdf;

				if( tRF > tN[i])
				{
					tdf = (tRF - tN[i])*1./1000.;
					//              tdf = (tRF - tN[i])*1.;
				}
				else
				{
					tdf = (tN[i] - tRF)*(-1.)/1000.;
					//              tdf = (tN[i] - tRF)*(-1.);
				}
				double psd = (eL[i] - eS[i])*1.0/eL[i];
				//      PSD[i]->Fill(eLcal[i],psd);

				double photon_time = dist[i]/2.99792458e8 * 1e9;
				// flip on the yaxis 

				tdf = - tdf;

				//shift to 0 

				tdf += detectorShifts[i];

				//add photon travel time

				tdf = tdf + photon_time;

				// start at 0 

				if(tdf<0){
					tdf= tdf + 165;
				}

				//fold ontop of eachother (0 to 82.5 ns only)

				if(tdf > 82.5){
					tdf = tdf - 82.5;
				}

				tDiff[i]->Fill( tdf );
				//std::cout << i << std::endl;
				double tdf2 = tdf *1e-9;
				double E_tof = 6.241509e12 * ((0.5) * mnsi* (dist[i]*dist[i]/(tdf2*tdf2)));

				//              double Qv = (Tb - Ta + (1/(m*gamma))*(Ta*ma + Tb*mb - 2*std::sqrt(Ta*ma*Tb*mb)*cos(angle[i])));


				if(ncuts[i]->IsInside(eLcal[i],psd))
				{
					detectors[i].Eneutron->Fill(E_tof);
					//      Eneutron[i]->Fill(eLcal[i]);
					detectors[i].PSDn->Fill(eLcal[i],psd);

					detectors[i].tDiffn->Fill( tdf );
					detectors[i].eL_tDiff[i]->Fill( tdf, eLcal[i]);

				} // end if cuts->isInside
				else if(gcuts[i]->IsInside(eLcal[i],psd))
				{
					//Eneutron[i]->Fill(eL[i]);
					detectors[i].PSDg->Fill(eLcal[i],psd);
					detectors[i].tDiffg->Fill( tdf );

				} // end if cuts->isInside



			} // end if check for coincidence


		} // end for loop over NDET


	} // end while(reader.Next())

	std::cout << "Even count = " << even_counter << " Odd Counter = " << odd_counter << std::endl;
	return {detectors, isPrompt};

}


#endif // PROCESSRUNS_H
