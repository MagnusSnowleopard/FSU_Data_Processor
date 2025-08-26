#include "TFile.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TH2F.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TCutG.h"
#include "TChain.h"
#include "TSystem.h"
#include "TROOT.h"
#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>


//Local header include//
#include "processRuns.h"

// --- directories (relative to repo root) -------------------------------------
static constexpr const char* kDataDir   = "data";
static constexpr const char* kOutputDir = "output";
// --- safer addInto for DetectorHistograms -----------------------------------
static bool sameAxis(const TAxis* a, const TAxis* b) {
	return a && b &&
		a->GetNbins() == b->GetNbins() &&
		a->GetXmin()  == b->GetXmin()  &&
		a->GetXmax()  == b->GetXmax();
}
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
	// detach from any file
	H.tDiff   ->SetDirectory(nullptr);
	H.tDiffg  ->SetDirectory(nullptr);
	H.tDiffn  ->SetDirectory(nullptr);
	H.ENeutron->SetDirectory(nullptr);
	H.PSD     ->SetDirectory(nullptr);
	H.PSDg    ->SetDirectory(nullptr);
	H.PSDn    ->SetDirectory(nullptr);

	return H;
}

static void add1D(TH1F*& dst, TH1F* src, double scale, const char* tag) {
	if (!src) return;

	if (!dst) {
		dst = static_cast<TH1F*>(src->Clone());
		dst->SetDirectory(nullptr);
		if (scale != 1.0) dst->Scale(scale);
		return;
	}

	if (!sameAxis(dst->GetXaxis(), src->GetXaxis())) {
		std::cerr << "[addInto] binning mismatch (" << tag << "): "
			<< dst->GetName() << " vs " << src->GetName() << "\n";
		return;
	}

	if (!dst->GetSumw2N()) dst->Sumw2();
	if (!src->GetSumw2N()) src->Sumw2();
	dst->Add(src, scale);
}

static void add2D(TH2F*& dst, TH2F* src, double scale, const char* tag) {

	gErrorIgnoreLevel = kError;
	if (!src) return;

	if (!dst) {
		dst = static_cast<TH2F*>(src->Clone());
		dst->SetDirectory(nullptr);
		if (scale != 1.0) dst->Scale(scale);
		return;
	}

	if (!sameAxis(dst->GetXaxis(), src->GetXaxis()) ||
			!sameAxis(dst->GetYaxis(), src->GetYaxis())) {
		std::cerr << "[addInto] binning mismatch (" << tag << "): "
			<< dst->GetName() << " vs " << src->GetName() << "\n";
		return;
	}

	if (!dst->GetSumw2N()) dst->Sumw2();
	if (!src->GetSumw2N()) src->Sumw2();
	dst->Add(src, scale);
}

// Public API used by your code
static void addInto(DetectorHistograms& A, const DetectorHistograms& B, double s = 1.0) {
	add1D(A.tDiff,    B.tDiff,    s, "tDiff");
	add1D(A.tDiffg,   B.tDiffg,   s, "tDiffg");
	add1D(A.tDiffn,   B.tDiffn,   s, "tDiffn");
	add1D(A.ENeutron, B.ENeutron, s, "ENeutron");

	add2D(A.PSD,      B.PSD,      s, "PSD");
	add2D(A.PSDg,     B.PSDg,     s, "PSDg");
	add2D(A.PSDn,     B.PSDn,     s, "PSDn");
}
void script(TString start, TString end, TString bgstart, TString bgend) 
	//void script(TString start, TString end) 
{
	gErrorIgnoreLevel = kError;
	int run_start = start.Atoi();
	int run_end = end.Atoi();

	int bg_start = bgstart.Atoi();
	int bg_end = bgend.Atoi();

	std::cout << "========================================" << std::endl;
	std::cout << "Run " << run_start << " to " << run_end << std::endl;
	std::cout << "BG  " << bg_start << " to  " << bg_end << std::endl;
	std::cout << "========================================" << std::endl;
	// Ensure output/ exists
	gSystem->mkdir(kOutputDir, true);
	// Save current working directory; we’ll hop into data/ so processRuns()

	std::vector<RunHistograms> allRuns;

	//Exaple override shifts using detectorshifts.h
	// exact per-run
	// perRunShifts.insert({612, {372, 375, 370, 374, 374, 371, 368, 373, 374, 374, 372, 371, 368, 371, 370}});

	// range 650–659
	// rangeShifts.push_back({{650,659}, {370,370,370,370,370,370,370,370,370,370,370,370,370,370,370}});
	//Initialize total histograms per detector
	std::vector<DetectorHistograms> resultsTotal(NDET); // accumulate prompt here
	std::vector<DetectorHistograms> promptTotal(NDET); // accumulate prompt here
	std::vector<DetectorHistograms> bgTotal(NDET);
	double t0 = timeRange[0];double t1 = timeRange[1];
	double prompt_counter; double BG_counter; double prompt_count; double bg_count;

	// 1) allocate totals
	for (int i = 0; i < NDET; ++i) {
		promptTotal[i]  = makeEmptyLike("Prompt",  i+1, max_E, kNBinsT, t0, t1);
		bgTotal[i]      = makeEmptyLike("BG",      i+1, max_E, kNBinsT, t0, t1);
		resultsTotal[i] = makeEmptyLike("Result",  i+1, max_E, kNBinsT, t0, t1);
	}
	// 2) stream runs: fill prompt totals
	for (int run = run_start; run <= run_end; ++run) {
		std::vector<double> shifts = getDetectorShiftsForRun(run); // or zeros if you don’t use this
		(void)processRunFill(run, /*isPrompt=*/true, shifts, promptTotal);
		prompt_counter = GETBEAMCOUNTER(run, /*isPrompt=*/true);
		std::cout << "Prompt Run # " << run << " || Beam counter = " << prompt_counter << std::endl;
		prompt_count += prompt_counter;
	}

	// 3) stream runs: fill background totals
	for (int run = bg_start; run <= bg_end; ++run) {
		std::vector<double> shifts = getDetectorShiftsForRun(run);
		(void)processRunFill(run, /*isPrompt=*/false, shifts, bgTotal);	
		BG_counter = GETBEAMCOUNTER(run, /*isPrompt=*/false);
		std::cout << "BG Run # " << run << " || Beam counter = " << BG_counter << std::endl;
		bg_count += BG_counter;
	}

	// 4) subtract background into a “results” set (clone prompt, then subtract)
	const double Pt = 8551.0, Bt = 6193.0;

	// to convert this into real beam current --> beamcounter / beamtime * 3 /10;

	const double scale = prompt_count / ((Pt/Bt)*bg_count);

	for (int i = 0; i < NDET; ++i) {
		if (resultsTotal[i].tDiff   && promptTotal[i].tDiff)    resultsTotal[i].tDiff   ->Add(promptTotal[i].tDiff,  1.0);
		if (resultsTotal[i].tDiffg  && promptTotal[i].tDiffg)   resultsTotal[i].tDiffg  ->Add(promptTotal[i].tDiffg, 1.0);
		if (resultsTotal[i].tDiffn  && promptTotal[i].tDiffn)   resultsTotal[i].tDiffn  ->Add(promptTotal[i].tDiffn, 1.0);
		if (resultsTotal[i].ENeutron&& promptTotal[i].ENeutron) resultsTotal[i].ENeutron->Add(promptTotal[i].ENeutron,1.0);
		if (resultsTotal[i].PSD     && promptTotal[i].PSD)      resultsTotal[i].PSD     ->Add(promptTotal[i].PSD,     1.0);
		if (resultsTotal[i].PSDg    && promptTotal[i].PSDg)     resultsTotal[i].PSDg    ->Add(promptTotal[i].PSDg,    1.0);
		if (resultsTotal[i].PSDn    && promptTotal[i].PSDn)     resultsTotal[i].PSDn    ->Add(promptTotal[i].PSDn,    1.0);

		if (resultsTotal[i].tDiff   && bgTotal[i].tDiff)    resultsTotal[i].tDiff   ->Add(bgTotal[i].tDiff,    -scale);
		if (resultsTotal[i].tDiffg  && bgTotal[i].tDiffg)   resultsTotal[i].tDiffg  ->Add(bgTotal[i].tDiffg,   -scale);
		if (resultsTotal[i].tDiffn  && bgTotal[i].tDiffn)   resultsTotal[i].tDiffn  ->Add(bgTotal[i].tDiffn,   -scale);
		if (resultsTotal[i].ENeutron&& bgTotal[i].ENeutron) resultsTotal[i].ENeutron->Add(bgTotal[i].ENeutron, -scale);
		if (resultsTotal[i].PSD     && bgTotal[i].PSD)      resultsTotal[i].PSD     ->Add(bgTotal[i].PSD,      -scale);
		if (resultsTotal[i].PSDg    && bgTotal[i].PSDg)     resultsTotal[i].PSDg    ->Add(bgTotal[i].PSDg,     -scale);
		if (resultsTotal[i].PSDn    && bgTotal[i].PSDn)     resultsTotal[i].PSDn    ->Add(bgTotal[i].PSDn,     -scale);
	}

	std::cout << "Tesseract Diagonalized" << std::endl;
	// 5) quick plots (optional)
	gStyle->SetOptStat(1110);
	// ... (draw overlays like before if you want) ...
/*
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
		//resultsTotal[i].Eneutron->Draw("colz");
		resultsTotal[i].PSDn->Draw("colz");
		resultsTotal[i].PSDg->Draw("same");

		c1->cd(i+1);
		resultsTotal[i].tDiffg->Draw("");
		resultsTotal[i].tDiffg->SetLineColor(2);
		resultsTotal[i].tDiffn->Draw("same");

		c2->cd(i+1);
		resultsTotal[i].ENeutron->Draw("colz");
	}

	std::cout << " Plotting complete " << std::endl;
*/ // doesn't work with this process mode right now 

	// 6) write results
	const TString outname = Form("%s/merged_run%d-%d_bg%d-%d.root",
			kOutputDir, run_start, run_end, bg_start, bg_end);
	TFile out(outname, "RECREATE");
	for (int i=0;i<NDET;++i) {
		if (resultsTotal[i].PSD)      { resultsTotal[i].PSD     ->SetName(Form("PSD_cal_%d",      i+1)); resultsTotal[i].PSD     ->Write(); }
		if (resultsTotal[i].PSDn)     { resultsTotal[i].PSDn    ->SetName(Form("PSDn_cal_%d",     i+1)); resultsTotal[i].PSDn    ->Write(); }
		if (resultsTotal[i].PSDg)     { resultsTotal[i].PSDg    ->SetName(Form("PSDg_cal_%d",     i+1)); resultsTotal[i].PSDg    ->Write(); }
		if (resultsTotal[i].tDiff)    { resultsTotal[i].tDiff   ->SetName(Form("tDiff_%d",        i+1)); resultsTotal[i].tDiff   ->Write(); }
		if (resultsTotal[i].tDiffn)   { resultsTotal[i].tDiffn  ->SetName(Form("tDiffn_%d",       i+1)); resultsTotal[i].tDiffn  ->Write(); }
		if (resultsTotal[i].tDiffg)   { resultsTotal[i].tDiffg  ->SetName(Form("tDiffg_%d",       i+1)); resultsTotal[i].tDiffg  ->Write(); }
		if (resultsTotal[i].ENeutron) { resultsTotal[i].ENeutron->SetName(Form("ENeutron_cal_%d", i+1)); resultsTotal[i].ENeutron->Write(); }
	}
	std::cout << "Saved: " << outname << "\n";
}



