#ifndef DETCAL_H
#define DETCAL_H

#include <map>
#include <utility>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

// Global maps (declared extern here, defined in a .cpp)
extern std::map<int, std::pair<double, double>> detCalMap;
extern std::map<int, double> Distance;
extern std::map<int, double> Angle;

// Struct that loads calibration from CSV when constructed
struct DetCalInitializer {
    DetCalInitializer(const char* filename = "detcal.csv") {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "WARNING: Could not open calibration file '" 
                      << filename << "'\n";
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
            Distance[detnum]  = D;
            Angle[detnum]     = theta;
        }

        file.close();
        std::cout << "Loaded calibration for " 
                  << detCalMap.size() << " detectors from " 
                  << filename << std::endl;
    }
};
#endif
