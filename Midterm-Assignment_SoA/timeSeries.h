#ifndef TIMESERIES_H
#define TIMESERIES_H
#include <string>

// Struttura per le nostre timeseries (SoA)
struct TimeSeriesData{
    std::vector<std::string> timestamps;  // Timestamp
    std::vector<double> values;           // Valore
};

#endif //TIMESERIES_H
