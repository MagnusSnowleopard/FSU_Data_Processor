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


//Local header include//
#include "processRuns.h"



static DetectorHistograms makeEmptyLike(const char* tag, int det,
		double max_E, int nbinsT,
		double t0, double t1) {
	DetectorHistograms H;
	H.tDiff    = new TH1F(Form("%s-Tdiff-det%d",  tag, det), "", nbinsT, t0, t1);
	H.tDiffg   = new TH1F(Form("%s-Tdiffg-det%d", tag, det), "", nbinsT, t0, t1);
	H.tDiffn   = new TH1F(Form("%s-Tdiffn-det%d", tag, det), "", nbinsT, t0, t1);
	H.tDiffg->SetLineColor(2);
	H.ENeutron = new TH1F(Form("%s-ENeutron-det%d", tag, det), "", 148, -20., 20.);
	H.PSD      = new TH2F(Form("%s-PSD-det%d",     tag, det), "", 4096, 0, max_E, 512, -.1, 1);
	H.PSDg     = new TH2F(Form("%s-PSDg-det%d",    tag, det), "", 4096, 0, max_E, 512, -.1, 1);
	H.PSDn     = new TH2F(Form("%s-PSDn-det%d",    tag, det), "", 4096, 0, max_E, 512, -.1, 1);
	return H;
}

static void addInto(DetectorHistograms& A, const DetectorHistograms& B, double s = 1.0) {
	if (B.tDiff)    A.tDiff   ->Add(B.tDiff,    s);
	if (B.tDiffg)   A.tDiffg  ->Add(B.tDiffg,   s);
	if (B.tDiffn)   A.tDiffn  ->Add(B.tDiffn,   s);
	if (B.ENeutron) A.ENeutron->Add(B.ENeutron, s);
	if (B.PSD)      A.PSD     ->Add(B.PSD,      s);
	if (B.PSDg)     A.PSDg    ->Add(B.PSDg,     s);
	if (B.PSDn)     A.PSDn    ->Add(B.PSDn,     s);
}


void script(TString start, TString end, TString bgstart, TString bgend) 
	//void script(TString start, TString end) 
{

	int run_start = start.Atoi();
	int run_end = end.Atoi();

	int bg_start = bgstart.Atoi();
	int bg_end = bgend.Atoi();

	std::cout << "========================================" << std::endl;
	std::cout << "Run " << run_start << " to " << run_end << std::endl;
	std::cout << "BG  " << bg_start << " to  " << bg_end << std::endl;
	std::cout << "========================================" << std::endl;


	std::vector<RunHistograms> allRuns;

	// process prompt runs
	for (int run = run_start; run <= run_end; ++run) {
		std::vector<double> shifts = getDetectorShiftsForRun(run);  // ← picks exact > range > default
		allRuns.push_back(processRuns(run, /*isPrompt=*/true, shifts));
	}

	// process background runs
	for (int run = bg_start; run <= bg_end; ++run) {
		std::vector<double> shifts = getDetectorShiftsForRun(run);
		allRuns.push_back(processRuns(run, /*isPrompt=*/false, shifts));
	}
	//Exaple override shifts using detectorshifts.h
	// exact per-run
	// perRunShifts.insert({612, {372, 375, 370, 374, 374, 371, 368, 373, 374, 374, 372, 371, 368, 371, 370}});

	// range 650–659
	// rangeShifts.push_back({{650,659}, {370,370,370,370,370,370,370,370,370,370,370,370,370,370,370}});


	//Initialize total histograms per detector
	std::vector<DetectorHistograms> promptTotal(NDET), bgTotal(NDET), resultsTotal(NDET);
	// 2) allocate totals and results

	const int    nbinsT = timeBin;             // 0.5 ns bins over 0..100 ns
	const double t0 = timeRange[0], t1 = timeRange[1];


	std::vector<DetectorHistograms> promptTotal(NDET), bgTotal(NDET), resultsTotal(NDET);
	for (int i = 0; i < NDET; ++i) {
		promptTotal[i]  = makeEmptyLike("Prompt",  i+1, max_E, nbinsT, t0, t1);
		bgTotal[i]      = makeEmptyLike("BG",      i+1, max_E, nbinsT, t0, t1);
		resultsTotal[i] = makeEmptyLike("Result",  i+1, max_E, nbinsT, t0, t1);
	}

	// 3) accumulate prompt vs bg
	for (const auto& R : allRuns) {
		for (int i = 0; i < NDET; ++i) {
			if (R.isPrompt) addInto(promptTotal[i], R.detectors[i], 1.0);
			else            addInto(bgTotal[i],     R.detectors[i], 1.0);
		}
	}


	// Scale and subtract


	for (int i = 0; i < NDET; ++i) {
		addInto(resultsTotal[i], bgTotal[i], -scale);
	}

	// 5) plotting on three canvases:
	//    (A) tDiff overlay: tDiffg (red) + tDiffn (blue) on each pad
	//    (B) ENeutron (1D)
	//    (C) PSDn (eL vs PSD) heatmap  (use PSD if you want inclusive)
	gStyle->SetOptStat(1110);

	TCanvas* cT = new TCanvas("cT", "tDiff overlay (g=red, n=blue)", 1600, 900);
	TCanvas* cE = new TCanvas("cE", "ENeutron",                       1600, 900);
	TCanvas* cP = new TCanvas("cP", "PSDn (eL vs PSD)",               1600, 900);

	cT->Divide(5, 3);  // 15 detectors
	cE->Divide(5, 3);
	cP->Divide(5, 3);

	for (int i = 0; i < NDET; ++i) {
		// (A) overlay
		cT->cd(i+1);
		TH1* g = resultsTotal[i].tDiffg;
		TH1* n = resultsTotal[i].tDiffn;
		if (g || n) {
			if (g) { g->SetLineColor(kRed);   g->Draw("HIST"); }
			if (n) { n->SetLineColor(kBlue);  n->Draw(g ? "HIST SAME" : "HIST"); }
		}

		// (B) ENeutron
		cE->cd(i+1);
		if (resultsTotal[i].ENeutron) resultsTotal[i].ENeutron->Draw("HIST");

		// (C) PSDn heatmap (this is your eL vs PSD)
		cP->cd(i+1);
		if (resultsTotal[i].PSDn) resultsTotal[i].PSDn->Draw("COLZ");
	}

	// 6) write to file
	const TString outname = Form("merged_run%d-%d_bg%d-%d.root", run_start, run_end, bg_start, bg_end);
	TFile* out = TFile::Open(outname, "RECREATE");
	if (!out || out->IsZombie()) {
		std::cerr << "ERROR: cannot create output file " << outname << "\n";
		return;
	}

	for (int i = 0; i < NDET; ++i) {
		auto& R = resultsTotal[i];
		if (R.PSD)      { R.PSD     ->SetName(Form("PSD_cal_%d",     i+1)); R.PSD     ->Write(); }
		if (R.PSDn)     { R.PSDn    ->SetName(Form("PSDn_cal_%d",    i+1)); R.PSDn    ->Write(); }
		if (R.PSDg)     { R.PSDg    ->SetName(Form("PSDg_cal_%d",    i+1)); R.PSDg    ->Write(); }
		if (R.tDiff)    { R.tDiff   ->SetName(Form("tDiff_%d",       i+1)); R.tDiff   ->Write(); }
		if (R.tDiffn)   { R.tDiffn  ->SetName(Form("tDiffn_%d",      i+1)); R.tDiffn  ->Write(); }
		if (R.tDiffg)   { R.tDiffg  ->SetName(Form("tDiffg_%d",      i+1)); R.tDiffg  ->Write(); }
		if (R.ENeutron) { R.ENeutron->SetName(Form("ENeutron_cal_%d",i+1)); R.ENeutron->Write(); }
	}
	out->Close();

	std::cout << "Saved: " << outname << std::endl;
}

