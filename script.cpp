#include "TFile.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TH2F.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TCutG.h"
#include "TChain.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>

#define NDET 15

std::map<int, std::pair<double, double>> detCalMap;
std::map<int, double> Distance;
std::map<int, double> Angle;
// Helper function (not user-called)
namespace {
	struct DetCalInitializer {
		DetCalInitializer(const char* filename = "detcal.csv") {
			std::ifstream file(filename);
			if (!file.is_open()) {
				std::cerr << "WARNING: Could not open calibration file '" << filename << "'\n";
				return;
			}

			std::string line;
			std::getline(file, line); // skip header

			while (std::getline(file, line)) {
				std::stringstream ss(line);
				std::string field;
				int detnum;
				double A, B, D, theta;

				std::getline(ss, field, ','); detnum = std::stoi(field);
				std::getline(ss, field, ','); A = std::stod(field);
				std::getline(ss, field, ','); B = std::stod(field);
				std::getline(ss, field, ','); D = std::stod(field);
				std::getline(ss, field, ','); theta = std::stod(field);

				detCalMap[detnum] = std::make_pair(A, B);
				Distance[detnum] = D; 
				Angle[detnum] = theta;
			}

			file.close();
			std::cout << "Loaded calibration for " << detCalMap.size() << " detectors from " << filename << std::endl;
		}
	};

	// Static object that runs at load time
	DetCalInitializer _autoLoadDetcal;
}
/*
   TFile *_file = TFile::Open("PSD_cuts_15mev.root");

   TCutG *ncut_00 = (TCutG*)gROOT->FindObject("ncut1");
   TCutG *ncut_01 = (TCutG*)gROOT->FindObject("ncut2");
   TCutG *ncut_02 = (TCutG*)gROOT->FindObject("ncut3");
   TCutG *ncut_03 = (TCutG*)gROOT->FindObject("ncut4");
   TCutG *ncut_04 = (TCutG*)gROOT->FindObject("ncut5");
   TCutG *ncut_05 = (TCutG*)gROOT->FindObject("ncut6");
   TCutG *ncut_06 = (TCutG*)gROOT->FindObject("ncut7");
   TCutG *ncut_07 = (TCutG*)gROOT->FindObject("ncut8");
   TCutG *ncut_08 = (TCutG*)gROOT->FindObject("ncut9");
   TCutG *ncut_09 = (TCutG*)gROOT->FindObject("ncut10");
   TCutG *ncut_10 = (TCutG*)gROOT->FindObject("ncut10");
   TCutG *ncut_11 = (TCutG*)gROOT->FindObject("ncut12");
   TCutG *ncut_12 = (TCutG*)gROOT->FindObject("ncut13");
   TCutG *ncut_13 = (TCutG*)gROOT->FindObject("ncut14");
   TCutG *ncut_14 = (TCutG*)gROOT->FindObject("ncut15");

   TCutG *gcut_00 = (TCutG*)gROOT->FindObject("gcut1");
   TCutG *gcut_01 = (TCutG*)gROOT->FindObject("gcut2");
   TCutG *gcut_02 = (TCutG*)gROOT->FindObject("gcut3");
   TCutG *gcut_03 = (TCutG*)gROOT->FindObject("gcut4");
   TCutG *gcut_04 = (TCutG*)gROOT->FindObject("gcut5");
   TCutG *gcut_05 = (TCutG*)gROOT->FindObject("gcut6");
   TCutG *gcut_06 = (TCutG*)gROOT->FindObject("gcut7");
   TCutG *gcut_07 = (TCutG*)gROOT->FindObject("gcut8");
   TCutG *gcut_08 = (TCutG*)gROOT->FindObject("gcut9");
   TCutG *gcut_09 = (TCutG*)gROOT->FindObject("gcut10");
   TCutG *gcut_10 = (TCutG*)gROOT->FindObject("gcut10");
   TCutG *gcut_11 = (TCutG*)gROOT->FindObject("gcut12");
   TCutG *gcut_12 = (TCutG*)gROOT->FindObject("gcut13");
   TCutG *gcut_13 = (TCutG*)gROOT->FindObject("gcut14");
   TCutG *gcut_14 = (TCutG*)gROOT->FindObject("gcut15");
   */
TCutG *gcuts[NDET] = {nullptr};/* = {
				  gcut_00, gcut_01, gcut_02, gcut_03, gcut_04, gcut_05, gcut_06,
				  gcut_07, gcut_08, gcut_09, gcut_10, gcut_11, gcut_12, gcut_13,
				  gcut_14
				  };*/
TCutG *ncuts[NDET] = {nullptr};/* = {
				  ncut_00, ncut_01, ncut_02, ncut_03, ncut_04, ncut_05, ncut_06, 
				  ncut_07, ncut_08, ncut_09, ncut_10, ncut_11, ncut_12, ncut_13,
				  ncut_14
				  };*/

const std::map<unsigned short, int> SN2Bd = {
	/*
	   {336, 0},
	   {409, 1},
	   {89, 2}
	   };
	   */

{89, 0},
{334, 1}
};



// 3,5,6,9,11,12,14, big neutron
// 105, 106, 107, 108, 110, 111, 112, 114, 115  small neutron detector
// 200 is RF
// detector number in channel number index
/*
   const int mapping[3][16] = {
// 0,   1,   2,  3,  4,   5,   6,   7,  8,   9,  10,  11,  12, 13,  14, 15
{108,  -1,  -1, -1, -1, 105, 106, 107, -1,  -1, 110, 111, 112, -1, 114, 115 }, 
{ -1,  -1,  -1,  4, -1,   5,   7,  -1, -1,   2,  -1,  11,  10, -1,  13,   8 },
{ -1,  -1,  -1, -1, -1,  -1,  -1,  -1, -1,  -1,  -1,  -1, 200, -1,  -1,  -1 }
};
*/



const int mapping[2][16] = {
	// 0,   1,   2,  3,  4,   5,   6,   7,  8,   9,  10,  11,  12, 13,  14, 15
	{ 200,  -1,  -1,  -1, -1,   -1,   -1,  -1, -1,   -1,  -1,  -1,  -1, -1,  -1,   -1 },
	{-1,  101,  102, 103, 104, 105, 106, 107, 108,  109, 110, 111, 112, 113, 114, 115 }
};




/*
// small detectors prefixed with 100
const std::map<unsigned short, int> Index2ID = {
{  2,   3},
{  4,   0},
{  5,   1},
{  7,   2},
{  8,  15},
{ 10,   4},
{ 11,   5},
{ 13,   6},
{105,   7},
{106,   8},
{107,   9},
{108,  10},
{110,  11},
{111,  12},
{112,  13},
{114,  14},
{115,  16},
{200, 200}
};
*/


const std::map<unsigned short, int> Index2ID = {
	{101,   0},
	{102,   1},
	{103,   2},
	{104,   3},
	{105,   4},
	{106,   5},
	{107,   6},
	{108,   7},
	{109,   8},
	{110,   9},
	{111,  10},
	{112,  11},
	{113,  12}, 
	{114,  13},
	{115,  14},
	{200, 200}
};




/*
   const std::map<unsigned short, int> ID2Index = {
   { 3, 2},
   { 0, 4},
   { 1, 5},
   { 2, 7},
   { 15, 8},
   { 4, 10},
   { 5, 11},
   { 6, 13},
   { 7, 105},
   { 8, 106},
   { 9, 107},
   { 10,108},
   { 11,110},
   { 12,111},
   { 13,112},
   { 14,114},
   { 16,115},
   {200,200}
   };
   */



const std::map<unsigned short, int> ID2Index = {
	{ 12, 113},
	{ 8, 109},
	{ 0, 101},
	{ 1, 102},
	{ 2, 103},
	{ 3, 104},
	{ 4, 105},
	{ 5, 106},
	{ 6, 107},
	{ 7,108},
	{ 9,110},
	{ 10,111},
	{ 11,112},
	{ 13,114},
	{ 14,115},
	{200,200}

};


/*
TH2F *dudu[NDET]; // short vs long 

TH1F * tDiff[NDET];
TH1F * tDiffg[NDET];
TH1F * tDiffn[NDET];

TH2F * PSD[NDET];
TH2F * PSDg[NDET];
TH2F * PSDn[NDET];

TH2F * eL_tDiff[NDET];

TH1F* Eneutron[NDET];

ushort eL[NDET];
ushort eLcal[NDET];
ushort eS[NDET];
ushort dist[NDET];
ushort angle[NDET];
*/

double Q;
double mnsi = 1.6749e-27;
double mn = 939.57;
double m21ne = 20.9938*mn;
double m20ne = 19.9924*mn;
double m17o = 16.9991*mn;
double m11b = 11.0009*mn;


const float timeResol = 2; //ns 
const int timeRange[2] = {0, 100};
const int timeBin = (timeRange[1] - timeRange[0])/timeResol;

struct DetectorHistograms{
	TH1F* tDiff;
	TH1F* tDiffg;
	TH1F* tDiffn;
	TH1F* ENeutron;
//	TF1F* QVAL;
//	TH1F* EX;
	TH1F* PSD;
	TH1F* PSDg;
	TH1F* PSDn;


};


struct RunHistograms {

	std::vector<DetectorHistograms> detectors; //size NDET
	bool isPrompt;

};


std::<double> detectorShifts = {

	370 + 2, //det 1
	370 + 5, //det 2
	370 + 0, //det 3
	370 + 4, //det 4
	370 + 4, //det 5
	370 + 1, //det 6
	370 - 2, //det 7
	370 + 3, //det 8
	370 + 4, //det 9
	370 + 4, //det 10
	370 + 2, //det 11
	370 + 1, //det 12
	370 - 2, //det 13
	370 + 1, //det 14
	370 + 0, //det 15
};



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
	double max_E = 8196.;
	double timeBin = 2.;
	double timeRange[] = {0,100};

	for(int i = 0; i < NDET; ++i){
	detectors[i].tDiff = new TH1F(Form("Tdiff-run%d-det%d",RunNumber, i+1), "", timeBin, timeRange[0], timeRange[1]);
	detectors[i].tDiffg = new TH1F(Form("Tdiffg-run%d-det%d",RunNumber, i+1),"", timeBin, timeRange[0], timeRange[1]);
	detectors[i].tDiffn = new TH1F(Form("Tdiffn-run%d-det%d",RunNumber, i+1),"", timeBin, timeRange[0], timeRange[1]);

	detectors[i].tDiffg->SetLineColor(2);

	detectors[i].Eneutron = new TH1F(Form("Eneutron-run%d-det%d",RunNumber, i+1), "",148 , -20, 20);
//	detectors[i].QVAL = new TH1F(Form("QVAL-run%d-det%d",RunNumber, i+1), "",148 , -20, 20);
//	detectors[i].EX = new TH1F(Form("EX-run%d-det%d",RunNumber, i+1), "",148 , -20, 20);

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
	/*	for( int i = 0; i < NDET; i++) 
		{
			tN[i] = 0.;
			eL[i] = 0.;
			eLcal[i] = 0.;
			eS[i] = 0.;
			dist[i] = 0.;
			angle[i] = 0.;
		}*/

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

				//		std::cout << ID << std::endl;

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
				//	std::cout<< "ODD ch tN= "<< i << " , " << tN[i] << std::endl;
			}else if(i%2 ==1 && tN[i]){
				even_counter++;
				//	std::cout<< "EVEN ch tN= "<< i << " , " << tN[i] << std::endl;
			}




			if(/* tRF != 0 && */tN[i] != 0 ) // this condition takes out around 95% of the data in big 5, i think because tN int type 
			{ 
				double tdf;

				if( tRF > tN[i]) 
				{
					tdf = (tRF - tN[i])*1./1000.;
					//	        tdf = (tRF - tN[i])*1.;
				}
				else
				{
					tdf = (tN[i] - tRF)*(-1.)/1000.;
					//		tdf = (tN[i] - tRF)*(-1.);
				}
				double psd = (eL[i] - eS[i])*1.0/eL[i];
			//	PSD[i]->Fill(eLcal[i],psd);

				double photon_time = dist[i]/2.99792458e8 * 1e9;
				// flip on the yaxis 

				tdf = - tdf;

				//shift to 0 

/*
				if(i == 0){ tdf = tdf + 370 + 2;}
				if(i == 1){ tdf = tdf + 370 + 5;}
				if(i == 2){ tdf = tdf + 370 + 0;}
				if(i == 3){ tdf = tdf + 370 + 4;}
				if(i == 4){ tdf = tdf + 370 + 4;}
				if(i == 5){ tdf = tdf + 370 + 1;}
				if(i == 6){ tdf = tdf + 370 - 2;}
				if(i == 7){ tdf = tdf + 370 + 3;}
				if(i == 8){ tdf = tdf + 370 + 4;}
				if(i == 9){ tdf = tdf + 370 + 4;}
				if(i == 10){ tdf = tdf + 370 + 2;}
				if(i == 11){ tdf = tdf + 370 + 1;}	
				if(i == 12){ tdf = tdf + 370 - 2;}
				if(i == 13){ tdf = tdf + 370 + 1;}
				if(i == 14){ tdf = tdf + 370 + 0;}
*/
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
				double mnsi = 1.675e-27;
				double E_tof = 6.241509e12 * ((0.5) * mnsi* (dist[i]*dist[i]/(tdf2*tdf2))); 

				//		double Qv = (Tb - Ta + (1/(m*gamma))*(Ta*ma + Tb*mb - 2*std::sqrt(Ta*ma*Tb*mb)*cos(angle[i])));


				if(ncuts[i]->IsInside(eLcal[i],psd))
				{
					detectors[i].Eneutron->Fill(E_tof);
					//	Eneutron[i]->Fill(eLcal[i]);
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



void script(TString start, TString end, TString bgstart, TString bgend) 
	//void script(TString start, TString end) 
{

	int run_start = start.Atoi();
	int run_end = end.Atoi();

	int bg_start = bgstart.Atoi();
	int bg_end = bgend.Atoi();

	std::cout << "========================================" << std::endl;
	std::cout << "Run " << run_start << " to Run " << run_end << std::endl;
	std::cout << "BG run " << bg_start << " to BG Run " << bg_end << std::endl;
	std::cout << "========================================" << std::endl;


	std::vector<RunHistograms> allRuns;

	// Process prompt runs 
	for(int run = start; run < end; ++run){
	//customize time shifts here. 
		std::vector<double shifts = defaultShifts; 
		allRuns.push_back(processRuns(run,true,shifts));
	}	
	
	for(int bgrun = bg_start; bgrun < bg_end; ++bgrun){
	//customize time shifts here. 
		std::vector<double shifts = defaultShifts; 
		allRuns.push_back(processRuns(bgrun,false,shifts));
	}	

	//Initialize total histograms per detector
	std::vector<DetectorHistograms> promptTotal(NDET), bgTotal(NDET), resultsTotal(NDET);

	double max_E = 8196.;
	double timeBin = 2.;
	double timeRange[] = {0,100};
	for(int i = 0; i < NDET; ++i){

	promptTotal[i].tDiff = new TH1F(Form("Prompt-Tdiff--det%d",i+1), "", timeBin, timeRange[0], timeRange[1]);
	promptTotal[i].tDiffg = new TH1F(Form("Prompt-Tdiffg-det%d", i+1),"", timeBin, timeRange[0], timeRange[1]);
	promptTotal[i].tDiffn = new TH1F(Form("Prompt-Tdiffn-det%d", i+1),"", timeBin, timeRange[0], timeRange[1]);
	promptTotal[i].tDiffg->SetLineColor(2);
	promptTotal[i].Eneutron = new TH1F(Form("Prompt-Eneutron-det%d", i+1), "",148 , -20, 20);
//	promptTotal[i].QVAL = new TH1F(Form("Prompt-QVAL-det%d", i+1), "",148 , -20, 20);
//	promptTotal[i].EX = new TH1F(Form("Prompt-EX-det%d", i+1), "",148 , -20, 20);
	promptTotal[i].PSD = new TH2F(Form("Prompt-PSD-det%d", i+1),"", 4096, 0, max_E,512,-.1,1);
	promptTotal[i].PSDg = new TH2F(Form("Prompt-PSDg-det%d", i+1),"", 4096, 0, max_E,512,-.1,1);
	promptTotal[i].PSDn = new TH2F(Form("Prompt-PSDn-det%d", i+1),"", 4096, 0, max_E,512,-.1,1);

	bgTotal[i].tDiff = new TH1F(Form("BG-Tdiff-det%d", i+1), "", timeBin, timeRange[0], timeRange[1]);
	bgTotal[i].tDiffg = new TH1F(Form("BG-Tdiffg-det%d", i+1),"", timeBin, timeRange[0], timeRange[1]);
	bgTotal[i].tDiffn = new TH1F(Form("BG-Tdiffn-det%d", i+1),"", timeBin, timeRange[0], timeRange[1]);
	bgTotal[i].tDiffg->SetLineColor(2);
	bgTotal[i].Eneutron = new TH1F(Form("BG-Eneutron-det%d", i+1), "",148 , -20, 20);
//	bgTotal[i].QVAL = new TH1F(Form("BG-QVAL-det%d", i+1), "",148 , -20, 20);
//	bgTotal[i].EX = new TH1F(Form("BG-EX-det%d", i+1), "",148 , -20, 20);
	bgTotal[i].PSD = new TH2F(Form("BG-PSD-det%d", i+1),"", 4096, 0, max_E,512,-.1,1);
	bgTotal[i].PSDg = new TH2F(Form("BG-PSDg-det%d", i+1),"", 4096, 0, max_E,512,-.1,1);
	bgTotal[i].PSDn = new TH2F(Form("BG-PSDn-det%d", i+1),"", 4096, 0, max_E,512,-.1,1);

	

	resultsTotal[i].tDiff = (TH1F*)promptTotal[i].tDiff->Clone(Form("Tdiff-det%d", i+1));
	resultsTotal[i].tDiffg = (TH1F*)promptTotal[i].tDiffg->Clone((Form("Tdiffg-det%d", i+1));
	resultsTotal[i].tDiffn = (TH1F*)promptTotal[i].tDiffn->Clone((Form("Tdiffn-det%d", i+1));
	resultsTotal[i].tDiffg->SetLineColor(2); ///keep going from here 
	resultsTotal[i].Eneutron = new TH1F(Form("Prompt-Eneutron-det%d", i+1), "",148 , -20, 20);
	resultsTotal[i].QVAL = new TH1F(Form("Prompt-QVAL-det%d", i+1), "",148 , -20, 20);
	resultsTotal[i].EX = new TH1F(Form("Prompt-EX-det%d", i+1), "",148 , -20, 20);
	resultsTotal[i].PSD = new TH2F(Form("Prompt-PSD-det%d", i+1),"", 4096, 0, max_E,512,-.1,1);
	resultsTotal[i].PSDg = new TH2F(Form("Prompt-PSDg-det%d", i+1),"", 4096, 0, max_E,512,-.1,1);
	resultsTotal[i].PSDn = new TH2F(Form("Prompt-PSDn-det%d", i+1),"", 4096, 0, max_E,512,-.1,1);

	}


	// Scale and subtract

	double Pt = 8551.; //60-65
	double Bt = 6193.; //66-69
	double scaleFactor = Pt/Bt;

	for(int i = 0; i < NDET; i++){

	bgTotal[i].TDiff->Scale 

	


	}




	//setwarning(nullptr)
	ULong64_t beammonitor;
	double runtime = 8551.;
	double Beamcurrent = beammonitor/runtime * 30./100.;

	std::cout << "Average Particle Current = "<< Beamcurrent << std::endl;

	//^###########################################################
	//^ * Process
	//^###########################################################

	std::cout << "Filling histograms..." << std::endl;



	std::cout << "Plotting..." << std::endl;

	gStyle->SetOptStat(11111111);

	TCanvas *canvas = new TCanvas();
	TCanvas *c1 = new TCanvas();
	TCanvas *c2 = new TCanvas();
	canvas->Divide(5,4);
	c1->Divide(5,4);
	c2->Divide(5,4);


	for(int i=0;i<NDET;i++)
	{
		canvas->cd(i+1); 
		//PSD[i]->Draw("colz");
		//ncuts[i]->Draw("same");
		Eneutron[i]->Draw("colz");
		//PSDn[i]->Draw("colz");
		//PSDg[i]->Draw("same");

		c1->cd(i+1);
		tDiffg[i]->Draw("");
		tDiffg[i]->SetLineColor(2);
		tDiffn[i]->Draw("same");


		c2->cd(i+1);
		eL_tDiff[i]->Draw("colz");
	}

	std::cout << "Plotting complete." << std::endl;

	TFile* outFile = new TFile(merged_filename_out, "RECREATE");
	for (int i = 0; i < NDET; i++) {
		PSD[i]->SetName(Form("PSD_cal_%d", i));
		PSD[i]->Write();

		tDiff[i]->SetName(Form("tDiff_%d", i));
		tDiff[i]->Write();

		Eneutron[i]->SetName(Form("Eneutron_cal_%d", i));
		Eneutron[i]->Write();

		PSDn[i]->SetName(Form("PSDn_cal_%d", i));
		PSDn[i]->Write();

		PSDg[i]->SetName(Form("PSDg_cal_%d", i));
		PSDg[i]->Write();  // previously you had PSDg writing to "PSDn_raw_%d" – this was likely a bug

		tDiffn[i]->SetName(Form("tDiffn_%d", i));
		tDiffn[i]->Write();

		tDiffg[i]->SetName(Form("tDiffg_%d", i));
		tDiffg[i]->Write();

		eL_tDiff[i]->SetName(Form("eL_tDiffn_%d", i));
		eL_tDiff[i]->Write();
	}
	std::cout << "Histograms Saved" << std::endl;
	outFile->Close();
} // end script()
