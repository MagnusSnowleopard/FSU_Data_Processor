#include "script.h"
#include "TString.h"

// Provided by src/script.cpp
void script(TString start, TString end, TString bgstart, TString bgend);

int main(int argc, char** argv) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <run_start> <run_end> <bg_start> <bg_end>\n", argv[0]);
        return 1;
    }
    script(TString(argv[1]), TString(argv[2]), TString(argv[3]), TString(argv[4]));
    return 0;
}
