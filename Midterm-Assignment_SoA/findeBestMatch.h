#ifndef FINDEBESTMATCH_H
#define FINDEBESTMATCH_H

#include <vector>
#include "timeSeries.h"

void findBestMatchSequential(const TimeSeriesData& timeseries, const std::vector<double>& target_sequence, std::vector<double>& bestMatchingValues, const bool printOn);
void findBestMatchParallel(const TimeSeriesData& timeseries, const std::vector<double>& target_sequence, std::vector<double>& bestMatchingValues, const int nThreads, const bool printOn);
void findBestMatchParallelExplicitChunk(const TimeSeriesData& timeseries, const std::vector<double>& target_sequence, std::vector<double>& bestMatchingValues, const int nThreads, const bool printOn);
void findBestMatchParallelExplicitChunkAtomic(const TimeSeriesData& timeseries, const std::vector<double>& target_sequence, std::vector<double>& bestMatchingValues, const int nThreads, const bool printOn);


#endif //FINDEBESTMATCH_H
