# FSU_Data_Processor
FSU experiment analysis repository for data filtering. Convenient analysis code.  

Repository layout

├─ include/
│  ├─ globals.h              # global constants (NDET, etc.)
│  ├─ mapinit.h              # inline maps & matrices for hardware mapping
│  ├─ detcal.h               # calibration/geometry declarations (A,B), Distance, Angle
│  ├─ detectorshifts.h       # default + per-run + per-range shifts & accessor
│  └─ processRuns.h          # header-only analysis kernel + histogram structs
├─ src/
│  └─ script.C               # (optional) ROOT macro interface (script(...))
├─ data/                     # runNNN.root inputs (git-ignored by default)
├─ output/                   # merged results (git-ignored by default)
├─ .gitignore
└─ README.md

Compilation command: c++ -std=c++17 -O2 -Iinclude   src/script.cpp src/detcal_data.cpp src/main.cpp -w -o runAnalysis $(root-config --cflags --libs)

The project is a header-only ROOT analysis pipeline for your detector data. It ingests “runN.root” files, maps raw hardware channels to logical detector IDs, applies per-detector timing shifts that can vary by run number or run range, classifies hits into gamma vs neutron using PSD cuts, computes time-of-flight (TOF)–based neutron energy, fills a standard set of histograms per detector, and finally merges many runs by subtracting a scaled background from prompt data. It also makes quick-look plots and writes a merged ROOT file with the results.

What each header is responsible for
globals.h
Holds global constants used everywhere, most importantly the number of detectors (NDET). Keeping this single source of truth ensures all arrays and loops agree on the detector count.

mapinit.h
Defines the translation from low-level hardware to analysis channels. It contains:

A map from a logical “channel index” to a logical detector ID (0..NDET-1), plus a special ID (200) used for the RF/beam monitor channel.

A 2×16 matrix that maps “board row, channel number” to that logical channel index (or −1 for “ignore this channel”).

A map from digitizer serial numbers to the board row used in that 2×16 matrix.

Together, these let the analysis take each hit’s (serial number, channel) and decide whether it’s the RF reference or one of your detectors.

detcal.h
Declares your calibration and geometry data:

Per-detector linear energy calibration coefficients (A, B) to turn the raw long integral into calibrated energy.

Per-detector flight path distance (meters).

Per-detector angle (degrees), available for any angle-dependent physics you might add later.

These are declared here and defined once elsewhere so the whole project shares the same numbers.

detectorshifts.h
Encodes timing shifts (in nanoseconds) to be added per detector before folding time differences. It supports:

A default shift vector that applies to all runs.

Exact per-run overrides.

Per-run range overrides.
A single helper returns the correct shift vector for any run after applying those precedence rules (exact run beats range, which beats default).

processRuns.h
Implements the per-run analysis and defines the histogram bundles:

A struct that groups per-detector histograms (time-difference inclusive, gamma-only, neutron-only; neutron energy; PSD inclusive, gamma-only, neutron-only).

A struct that represents the result from one run: the vector of detector histograms plus a flag indicating whether this run is “prompt” or “background.”

Two arrays of PSD cuts (one for neutrons, one for gammas), built once per detector on first use.

The processRuns function that reads one ROOT file, performs mapping, timing, PSD classification, TOF energy computation, and fills all histograms for that run.

What happens inside processRuns (per run)
It opens run<RunNumber>.root, retrieves a TTree named “tree,” and binds readers for event ID, multiplicity, arrays of serial numbers, channels, long/short integrals, and timestamps split into coarse nanoseconds and fine picoseconds.

It ensures your PSD polygon cuts exist for each detector. Each cut is a region in calibrated energy vs PSD space; one region classifies neutrons, the other gammas.

It allocates a consistent set of histograms for each detector: three 1D time-difference spectra (inclusive, gamma-tagged, neutron-tagged), one 1D neutron energy spectrum from TOF, and three 2D PSD maps (inclusive, gamma-only, neutron-only).

It loops over all events. For each event, it:

Initializes per-detector scratch arrays for times and energies.

Iterates over each hit in the event, maps (serial number, channel) to a logical index using your tables, then to a detector ID.

If the ID is the special RF/monitor channel, it records the RF timestamp for this event.

If the ID is a real detector, it stores:

raw long/short integrals, and their calibrated energy via the linear (A,B) model,

the detector’s distance and angle,

the hit’s timestamp.

After collecting the event’s hits, it processes each detector with a valid timestamp:

Computes a signed time difference between that detector and the RF in picoseconds, converts to nanoseconds, and flips sign to match your convention.

Adds the detector-specific shift for the current run (nanoseconds).

Adds the photon travel time (distance divided by the speed of light) in nanoseconds.

Wraps the time into your analysis window using the same folding rules you use (first into a 165-ns bucket to be non-negative, then into a half-period like 82.5 ns). This yields a final value that lives inside the histogram range.

Computes PSD as (long − short) / long if the long integral is positive.

Fills the inclusive time-difference spectrum.

Places the hit in the inclusive PSD map (calibrated energy vs PSD).

Classifies the hit by checking whether it falls inside your neutron cut polygon or gamma cut polygon, then fills the corresponding PSD and time-difference histograms.

Computes neutron energy from TOF for neutron-classified hits using classical kinematics with SI units and converts joules to keV; fills the TOF-derived neutron energy spectrum.

It returns a result object containing all per-detector histograms for that run and a flag telling you whether this run was prompt or background.

Throughout, it guards against missing files/trees, out-of-range channels, unknown serial numbers, or absent RF timestamps, and quietly skips anything it can’t interpret.

What the top-level script function does
It parses four string inputs: start and end of the prompt run range, and start and end of the background run range.

For each prompt run number, it asks the shift system for the correct per-detector shifts for that run, calls processRuns with the “prompt” flag, and collects the per-run results. It repeats the same for all background runs with the “background” flag.

It allocates three sets of per-detector totals:

A prompt total (the sum over all prompt runs),

A background total (the sum over all background runs),

A results total that will hold the background-subtracted answer.

The binning and axes are kept consistent with what processRuns used so sums are well-defined.

It adds each run’s histograms into either the prompt or background totals, detector by detector and histogram by histogram.

It performs background subtraction by scaling the background totals and subtracting from the prompt totals. The typical scale is the ratio of prompt to background live times; beam-monitor-based normalization can be used if you extend the bookkeeping to carry per-run monitor counts. The result is, per detector, “prompt minus scaled background” for all histograms.

It makes three quick-look canvases:

An overlay of the gamma-tagged time-difference and neutron-tagged time-difference for each detector (gamma often drawn in red, neutron in blue) to visualize temporal separation.

The TOF-derived neutron energy spectrum for each detector.

A neutron PSD heatmap (calibrated energy vs PSD) for each detector to verify the PSD cut performance.
If you prefer a 1D calibrated energy spectrum instead of the 2D heatmap for this third panel, you can project the X-axis of the neutron PSD map.

It writes a single merged ROOT file containing, per detector, the background-subtracted inclusive PSD, neutron PSD, gamma PSD, inclusive time difference, neutron time difference, gamma time difference, and the neutron energy spectrum, with a consistent naming scheme that includes the detector index.

Data flow summary (end-to-end)
Raw hits arrive as arrays of serial numbers, channels, integrals, and timestamps.

Hardware is mapped to logical detector IDs; one special channel is the RF/monitor.

For each detector hit, the analysis computes a folded time difference relative to RF, applies a run-aware per-detector shift, adds photon flight time, and fills time-difference spectra.

PSD is computed and used to classify hits into neutron or gamma; those categories fill their own PSD and time-difference histograms.

For neutron-classified hits, TOF energy is computed and filled.

Results from many runs are accumulated as “prompt totals” and “background totals.”

Background totals are scaled and subtracted from prompt totals to form the final “results totals.”

The results are plotted for inspection and saved to disk.

Scaling choices (how background is normalized)
Two common normalizations are supported conceptually:

Livetime scaling: scale factor equals prompt livetime divided by background livetime. This assumes your run metadata accurately reflect live time.

Beam-monitor scaling: scale factor equals the ratio of summed monitor counts in prompt runs to those in background runs. This requires you to count or return the monitor rate per run in the pipeline.

Either way, once you compute the scale, subtraction is performed consistently across all histograms.

Error handling and safeguards
If an input file can’t be opened or a TTree is missing, the run is skipped with a warning and no histograms are added.

Unrecognized serial numbers or out-of-range channels are ignored.

PSD calculations avoid division by zero when the long integral is zero.

Time folding and shifting enforce the same analysis window across detectors and runs.

Why it’s header-only
Most of the logic is implemented in headers and marked inline. That avoids linker errors when the same functions or globals are included from multiple translation units, and it keeps the project lightweight. Calibration data can be either defined once in a .cpp (using extern declarations) or also made inline in a header if you prefer a purely header-only deployment.

What you get out at the end
For each detector, after background subtraction you have:

An inclusive folded time-difference spectrum.

Gamma-only and neutron-only time-difference spectra.

A TOF-derived neutron energy spectrum.

Inclusive, gamma-only, and neutron-only PSD maps.

These support quick validation of timing alignment, PSD separation quality, and neutron energy behavior, and they’re saved in one merged ROOT file named to reflect the prompt and background run ranges.

￼
￼
￼
￼

