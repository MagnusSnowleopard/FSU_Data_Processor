#ifndef DETECTORSHIFTS_H
#define DETECTORSHIFTS_H

#include <vector>
#include <map>
#include <utility>  // std::pair
#include <algorithm>

#ifndef NDET
#define NDET 15  // must match your analysis
#endif

// Units: ns (consistent with your tdf_ns code path)
inline const std::vector<double> defaultShifts = {
    370 + 2,   // det 1
    370 + 5,   // det 2
    370 + 0,   // det 3
    370 + 4,   // det 4
    370 + 4,   // det 5
    370 + 1,   // det 6
    370 - 2,   // det 7
    370 + 3,   // det 8
    370 + 4,   // det 9
    370 + 4,   // det 10
    370 + 2,   // det 11
    370 + 1,   // det 12
    370 - 2,   // det 13
    370 + 1,   // det 14
    370 + 0    // det 15
};

// ------------------------------
// EXACT PER-RUN OVERRIDES (examples)
// Key = run number; Value = full (or partial) vector of shifts.
// If vector size < NDET, it will be padded with 0s; if > NDET, it will be trimmed.
inline const std::map<int, std::vector<double>> perRunShifts = {
    // Example: run 612 has a slightly different timing
    {612, {372, 375, 370, 374, 374, 371, 368, 373, 374, 374, 372, 371, 368, 371, 370}},

    // Example: run 745 only touches first 5 detectors; others default to 0 (then we’ll layer defaults)
    {745, {371, 374, 369, 373, 373}}
};

// ------------------------------
// PER-RUN RANGE OVERRIDES (examples) — inclusive [lo, hi]
// First match wins (order matters).
inline const std::vector<std::pair<std::pair<int,int>, std::vector<double>>> rangeShifts = {
    // All runs 600–620 share a common shift set:
    {{600, 620}, {372, 375, 370, 374, 374, 371, 368, 373, 374, 374, 372, 371, 368, 371, 370}},

    // Runs 900–999: apply a flat 370 ns across all detectors:
    {{900, 999}, std::vector<double>(NDET, 370.0)}
};

// ------------------------------
// Helper: ensure vector size == NDET (pad with 0s or trim)
inline std::vector<double> normalizeShifts(std::vector<double> v) {
    if ((int)v.size() < NDET) v.resize(NDET, 0.0);
    if ((int)v.size() > NDET) v.resize(NDET);
    return v;
}

// Merge (fallback): fill missing entries in 'dst' from 'fallback'
inline void fillMissingFrom(std::vector<double>& dst, const std::vector<double>& fallback) {
    if ((int)dst.size() < NDET) dst.resize(NDET, 0.0);
    for (int i = 0; i < NDET; ++i) {
        // Consider "missing" as exactly 0.0; if you want tri-state, add a sentinel.
        if (dst[i] == 0.0) dst[i] = fallback[i];
    }
}

// Main accessor: get the shifts for a given run
inline std::vector<double> getDetectorShiftsForRun(int runNumber) {
    // Start with defaults
    std::vector<double> shifts = defaultShifts;  // already NDET

    // Check exact per-run override
    if (auto it = perRunShifts.find(runNumber); it != perRunShifts.end()) {
        auto overrideV = normalizeShifts(it->second);
        // If you want per-run to fully REPLACE defaults, return overrideV.
        // If you want per-run to partially override, layer it on top:
        for (int i = 0; i < NDET; ++i) {
            if (it->second.size() > i) shifts[i] = overrideV[i];  // replace provided entries
        }
        return shifts;
    }

    // Check ranges (first match wins)
    for (const auto& entry : rangeShifts) {
        const int lo = entry.first.first;
        const int hi = entry.first.second;
        if (runNumber >= lo && runNumber <= hi) {
            auto overrideV = normalizeShifts(entry.second);
            // Replace provided entries; keep defaults for the rest
            for (int i = 0; i < NDET; ++i) {
                if (entry.second.size() > i) shifts[i] = overrideV[i];
            }
            return shifts;
        }
    }

    // Otherwise just the defaults
    return shifts;
}

#endif // DETECTORSHIFTS_H
