#include "detcal.h"

// Define the extern maps (storage)
std::map<int, std::pair<double, double>> detCalMap;
std::map<int, double> Distance;
std::map<int, double> Angle;

// Load CSV at program startup (runs before main)
static DetCalInitializer gDetCalInit("data/detcal.csv");
