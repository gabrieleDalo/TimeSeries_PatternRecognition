#ifndef TIMESERIES_H
#define TIMESERIES_H
#include <string>

// Struttura per le nostre timeseries (AoS)
struct TimeSeriesData{
    std::string timestamp;  // Timestamp
    double value;           // Valore
};

#endif //TIMESERIES_H
