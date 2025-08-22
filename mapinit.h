#ifndef MAPINIT_H
#define MAPINIT_H

#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>


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

const int mapping[2][16] = {
        // 0,   1,   2,  3,  4,   5,   6,   7,  8,   9,  10,  11,  12, 13,  14, 15
        { 200,  -1,  -1,  -1, -1,   -1,   -1,  -1, -1,   -1,  -1,  -1,  -1, -1,  -1,   -1 },
        {-1,  101,  102, 103, 104, 105, 106, 107, 108,  109, 110, 111, 112, 113, 114, 115 }
};

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

#endif
