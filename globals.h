#ifndef GLOBALS_H
#define GLOBALS_H



double Q;
double mnsi = 1.6749e-27;
double mn = 939.57;
double m21ne = 20.9938*mn;
double m20ne = 19.9924*mn;
double m17o = 16.9991*mn;
double m11b = 11.0009*mn;
const double J_to_keV = 6.241509074e15 / 1000.0; // J -> keV

double max_E = 8196.;
const float timeResol = 2; //ns 
const int timeRange[2] = {0, 100};
const int timeBin = (timeRange[1] - timeRange[0])/timeResol;

#endif
