#include <iostream>
#include <omp.h>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <vector>
#include <random>
#include "utils.h"
#include "findeBestMatch.h"
#include "timeSeries.h"

using namespace std;

int main() {
    double startExeTime, endExeTime;
    startExeTime = omp_get_wtime();
    const string filename = "mockseries_data.csv";
    int nRepeat = 100;
    vector<TimeSeriesData> timeseriesData, timeseriesDataParallel;
    bool getInput;

    // Chiedo all'utente se vuole eseguire la sperimentazione in modo automatico
    cout << "Custom experiments [1 = yes/0 = no]? ";
    getInput = askYesOrNo();

    if(getInput){
        int timeStampLenght, maxTimeStampLenght = 10000;
        int nThreads, maxNRepeat = 200;
        bool repeat;

        cout << "Insert number of threads to use for parallelization (" << 1 << " - " << omp_get_max_threads() << "): ";
        nThreads = getIntFromUser(1,omp_get_max_threads());

        cout << "Repeat experiment to compute mean performances [1/0]? ";
        repeat = askYesOrNo();     // Chiedo se l'esperimento deve essere ripetuto più volte

        if(repeat){
            cout << "Insert n° of repetitions (" << 1 << " - " << maxNRepeat << "): ";
            nRepeat = getIntFromUser(1, maxNRepeat);
        }

        cout << "Insert dimension of random time series to search (" << 1 << " - " << maxTimeStampLenght << "): ";
        timeStampLenght = getIntFromUser(1,maxTimeStampLenght);

        cout << endl;
        cout << "Reading from CSV..." << endl;
        if(!fileExists(filename)){              // Controlla se il file esiste
            cerr << "Error: file " << filename << " doesn't exists." << endl;
            return 1;
        }
        readCSV(timeseriesData, filename);             // Leggi i dati dal file CSV in modo sequenziale e parallelo
        readCSVParallel(timeseriesDataParallel, filename, nThreads);
        if(!checkTimeseriesEquality(timeseriesData, timeseriesDataParallel)) {
            cout << "Different data read with sequential and parallel" << endl;
            return 1;
        }

        vector<double> patternSequence;
        cout << "Acquiring time series to search..." << endl;
        acquireSampleTimeSeries(patternSequence, timeStampLenght, timeseriesData);       // Prende una sottosequenza della timeseries e la perturba
        cout << endl;

        double startTime, endTime;
        double totalTime_sequential, totalTime_parallel, totalTime_parallel_explicitChunk, totalTime_parallel_explicitChunkAtomic;
        double speedUp = 0;
        int nCheck = 0;
        vector<double> bestMatchingValuesSequential, bestMatchingValuesParallel, bestMatchingValuesParallelExplicitChunk, bestMatchingValuesParallelExplicitChunkAtomic;
        bestMatchingValuesSequential.reserve(timeStampLenght+1);
        bestMatchingValuesParallel.reserve(timeStampLenght+1);
        bestMatchingValuesParallelExplicitChunk.reserve(timeStampLenght+1);
        bestMatchingValuesParallelExplicitChunkAtomic.reserve(timeStampLenght+1);

        if(repeat){
            cout << "------- Iteration 1 -------" << endl;
        }

        // Nel caso di esperimenti ripetuti, solo alla prima iterazione controlla la consistenza dei risultati, nelle altre sarà uguale
        // quindi, separo la prima iterazione del ciclo dalle altre per evitare che i controlli vengano ripetuti

        startTime = omp_get_wtime();
        findBestMatchSequential(timeseriesData, patternSequence, bestMatchingValuesSequential, true);
        endTime = omp_get_wtime();

        if(bestMatchingValuesSequential.empty()){       // Capita ad esempio se la dimensione della timeseries è minore della timeseries da cercare
            cout << "No match found in the timeseries." << endl;
            return 1;
        }
        totalTime_sequential = computeElapsedTime(startTime, endTime);
        nCheck = static_cast<int>(bestMatchingValuesSequential[bestMatchingValuesSequential.size()-1]);
        printVector(bestMatchingValuesSequential, bestMatchingValuesSequential.size()-1);
        cout << "Compute time (sequential): " << totalTime_sequential << " s" << endl;
        cout << endl;

        startTime = omp_get_wtime();
        findBestMatchParallel(timeseriesData, patternSequence, bestMatchingValuesParallel, nThreads, true);
        endTime = omp_get_wtime();

        if(bestMatchingValuesParallel.empty()){
            cout << "No match found in the timeseries." << endl;
            return 1;
        }
        if(nCheck != bestMatchingValuesParallel[bestMatchingValuesParallel.size()-1]) {
            cout << "Different number of checks" << endl;
            return 1;
        }
        totalTime_parallel = computeElapsedTime(startTime, endTime);
        printVector(bestMatchingValuesParallel, bestMatchingValuesParallel.size()-1);
        cout << "Compute time (parallel): " << totalTime_parallel << " s" << endl;
        cout << endl;

        startTime = omp_get_wtime();
        findBestMatchParallelExplicitChunk(timeseriesData, patternSequence, bestMatchingValuesParallelExplicitChunk, nThreads, true);
        endTime = omp_get_wtime();

        if(bestMatchingValuesParallelExplicitChunk.empty()){
            cout << "No match found in the timeseries." << endl;
            return 1;
        }
        if(nCheck != bestMatchingValuesParallelExplicitChunk[bestMatchingValuesParallelExplicitChunk.size()-1]) {
            cout << "Different number of checks" << endl;
            return 1;
        }
        totalTime_parallel_explicitChunk = computeElapsedTime(startTime, endTime);
        printVector(bestMatchingValuesParallelExplicitChunk, bestMatchingValuesParallelExplicitChunk.size()-1);
        cout << "Compute time (parallel explicit chunk): " << totalTime_parallel_explicitChunk << " s" << endl;
        cout << endl;

        startTime = omp_get_wtime();
        findBestMatchParallelExplicitChunkAtomic(timeseriesData, patternSequence, bestMatchingValuesParallelExplicitChunkAtomic, nThreads, true);
        endTime = omp_get_wtime();

        if(bestMatchingValuesParallelExplicitChunkAtomic.empty()){
            cout << "No match found in the timeseries." << endl;
            return 1;
        }
        if(nCheck != bestMatchingValuesParallelExplicitChunkAtomic[bestMatchingValuesParallelExplicitChunkAtomic.size()-1]) {
            cout << "Different number of checks" << endl;
            return 1;
        }
        totalTime_parallel_explicitChunkAtomic = computeElapsedTime(startTime, endTime);
        printVector(bestMatchingValuesParallelExplicitChunkAtomic, bestMatchingValuesParallelExplicitChunkAtomic.size()-1);
        cout << "Compute time (parallel explicit chunk with atomic): " << totalTime_parallel_explicitChunkAtomic << " s" << endl;
        cout << endl;

        // Calcola lo speedup sul miglior tempo registrato in parallelo
        // per farlo devo verificare qual'è la versione che ha impiegato il minor tempo
        if(totalTime_parallel <= totalTime_parallel_explicitChunk){
            if(totalTime_parallel <= totalTime_parallel_explicitChunkAtomic) {
                cout << "Better parallelize without explicit chunk declaration" << endl;
                if(equal(bestMatchingValuesSequential.begin(), bestMatchingValuesSequential.end(), bestMatchingValuesParallel.begin())){
                    cout << "Same results with sequential and parallel" << endl;
                }else{
                    cout << "Different results for sequential and parallel" << endl;
                    return 1;
                }
                speedUp = totalTime_sequential/totalTime_parallel;
            }else{
                cout << "Better parallelize with explicit chunk declaration and atomic" << endl;
                if(equal(bestMatchingValuesSequential.begin(), bestMatchingValuesSequential.end(), bestMatchingValuesParallelExplicitChunkAtomic.begin())){
                    cout << "Same results with sequential and parallel" << endl;
                }else{
                    cout << "Different results for sequential and parallel" << endl;
                    return 1;
                }
                speedUp = totalTime_sequential/totalTime_parallel_explicitChunkAtomic;
            }
        }else{
            if(totalTime_parallel_explicitChunk <= totalTime_parallel_explicitChunkAtomic) {
                cout << "Better parallelize with explicit chunk declaration" << endl;
                if(equal(bestMatchingValuesSequential.begin(), bestMatchingValuesSequential.end(), bestMatchingValuesParallelExplicitChunk.begin())){
                    cout << "Same results with sequential and parallel" << endl;
                }else{
                    cout << "Different results for sequential and parallel" << endl;
                    return 1;
                }
                speedUp = totalTime_sequential/totalTime_parallel_explicitChunk;
            }else{
                cout << "Better parallelize with explicit chunk declaration and atomic" << endl;
                if(equal(bestMatchingValuesSequential.begin(), bestMatchingValuesSequential.end(), bestMatchingValuesParallelExplicitChunkAtomic.begin())){
                    cout << "Same results with sequential and parallel" << endl;
                }else{
                    cout << "Different results for sequential and parallel" << endl;
                    return 1;
                }
                speedUp = totalTime_sequential/totalTime_parallel_explicitChunkAtomic;
            }
        }
        cout << "Speedup: " << speedUp;
        checkTypeSpeedUp(speedUp, nThreads);
        cout << "Efficiency: " << speedUp/nThreads << endl;
        cout << endl;

        if(repeat){
            for(int i = 1; i < nRepeat; i++){
                cout << "------- Iteration " << (i+1) << " -------" << endl;
                startTime = omp_get_wtime();
                findBestMatchSequential(timeseriesData, patternSequence, bestMatchingValuesSequential, false);
                endTime = omp_get_wtime();
                totalTime_sequential += computeElapsedTime(startTime, endTime);

                startTime = omp_get_wtime();
                findBestMatchParallel(timeseriesData, patternSequence, bestMatchingValuesParallel, nThreads, false);
                endTime = omp_get_wtime();
                totalTime_parallel += computeElapsedTime(startTime, endTime);

                startTime = omp_get_wtime();
                findBestMatchParallelExplicitChunk(timeseriesData, patternSequence, bestMatchingValuesParallelExplicitChunk, nThreads, false);
                endTime = omp_get_wtime();
                totalTime_parallel_explicitChunk += computeElapsedTime(startTime, endTime);

                startTime = omp_get_wtime();
                findBestMatchParallelExplicitChunkAtomic(timeseriesData, patternSequence, bestMatchingValuesParallelExplicitChunkAtomic, nThreads, false);
                endTime = omp_get_wtime();
                totalTime_parallel_explicitChunkAtomic += computeElapsedTime(startTime, endTime);
            }

            cout << endl;
            if(timeStampLenght <= 20){      // Se la sequenza da cercare contiene troppi elementi evito di stamparla
                cout << "Pattern to find: ";
                for(vector<double>::size_type i = 0; i < timeStampLenght; i++){
                    cout << patternSequence[i] << "; ";
                }
                cout << endl;
            }

            // Faccio la media finale dei risultati ottenuti

            cout << "Mean time (sequential): " << totalTime_sequential/nRepeat << " s" << endl;
            cout << "Mean time (parallel): " << totalTime_parallel/nRepeat << " s" << endl;
            cout << "Mean time (parallel explicit chunk): " << totalTime_parallel_explicitChunk/nRepeat << " s" << endl;
            cout << "Mean time (parallel explicit chunk with atomic): " << totalTime_parallel_explicitChunkAtomic/nRepeat << " s" << endl;

            if(totalTime_parallel <= totalTime_parallel_explicitChunk){
                if(totalTime_parallel <= totalTime_parallel_explicitChunkAtomic) {
                    cout << "Better parallelize without explicit chunk declaration" << endl;
                    speedUp = totalTime_sequential/totalTime_parallel;
                }else{
                    cout << "Better parallelize with explicit chunk declaration and atomic" << endl;
                    speedUp = totalTime_sequential/totalTime_parallel_explicitChunkAtomic;
                }
            }else{
                if(totalTime_parallel_explicitChunk <= totalTime_parallel_explicitChunkAtomic) {
                    cout << "Better parallelize with explicit chunk declaration" << endl;
                    speedUp = totalTime_sequential/totalTime_parallel_explicitChunk;
                }else{
                    cout << "Better parallelize with explicit chunk declaration and atomic" << endl;
                    speedUp = totalTime_sequential/totalTime_parallel_explicitChunkAtomic;
                }
            }
            cout << "Speedup: " << speedUp;
            checkTypeSpeedUp(speedUp, nThreads);
            cout << "Efficiency: " << speedUp/nThreads << endl;
        }
    }else{
        // Parametri su cui eseguire gli esperimenti
        const vector<int> nThreadsOptions = {1, 2, 4, 6, 8, 10, 12};
        const vector<int> timeSeriesSizes = {1, 10, 50, 100, 500, 1000};
        vector<double> patternSequence;

        cout << endl;
        cout << "Reading time series from CSV..." << endl;
        if(!fileExists(filename)){              // Controlla se il file esiste
            cerr << "Error: file " << filename << " doesn't exists." << endl;
            return 1;
        }
        readCSV(timeseriesData, filename);
        for(auto nThreads : nThreadsOptions){
            readCSVParallel(timeseriesDataParallel, filename, nThreads);
            if(!checkTimeseriesEquality(timeseriesData, timeseriesDataParallel)) {
                std::cout << "Different data read with sequential and parallel" << std::endl;
                return 1;
            }
            if(nThreads != nThreadsOptions.back()) {
                timeseriesDataParallel.clear();
                timeseriesDataParallel.shrink_to_fit();
            }
        }

        string outputRsultsFile = "speedup_results_AoS.csv";
        ofstream resultsFile(outputRsultsFile);        // Creo un file CSV su cui salvare i risultati per poterne poi fare il plot
        resultsFile << "NThreads,SequenceSize,Speedup,Efficiency,BestVersion,MeanTimeSequential\n";

        for(auto sequenceSize : timeSeriesSizes){
            double bestOverallSpeedup = 0;
            double bestOverallEfficiency = 0;
            int bestNThreads = 1, nCheck = 0;
            string bestVersion = "None";
            vector<double> bestMatchingValuesSequential, bestMatchingValuesParallel, bestMatchingValuesParallelExplicitChunk, bestMatchingValuesParallelExplicitChunkAtomic;
            bestMatchingValuesSequential.reserve(sequenceSize+1);
            bestMatchingValuesParallel.reserve(sequenceSize+1);
            bestMatchingValuesParallelExplicitChunk.reserve(sequenceSize+1);
            bestMatchingValuesParallelExplicitChunkAtomic.reserve(sequenceSize+1);

            cout << "Acquiring time series to search..." << endl;
            acquireSampleTimeSeries(patternSequence, sequenceSize, timeseriesData);          // Prende una sottosequenza della timeseries e la perturba
            cout << endl;

            cout << "---------- Sequence size: " << sequenceSize << " ----------" << endl;

            for(auto nThreads : nThreadsOptions){
                double totalTimeSequential = 0, totalTimeParallel = 0, totalTimeParallelExplicitChunk = 0, totalTimeParallelExplicitChunkAtomic = 0;

                // Nel caso di esperimenti ripetuti, solo alla prima iterazione controlla la consistenza dei risultati, nelle altre sarà uguale
                // Calcola il tempo per la versione sequenziale
                // Questo tempo non viene calcolato una sola volta all'inizio ma viene calcolato per ogni numero di threads in modo da ottenere risultati coerenti
                // le prestazioni potrebbero peggiorare nel tempo all'aumentare del numero di ripetizioni dell'esperimento, es. a causa dell'aumento di temperatura del pc
                double startTime = omp_get_wtime();
                findBestMatchSequential(timeseriesData, patternSequence, bestMatchingValuesSequential, false);
                double endTime = omp_get_wtime();
                totalTimeSequential += (endTime - startTime == 0 ? 1e-8 : endTime - startTime);

                if(bestMatchingValuesSequential.empty()){
                    cout << "No match found in the timeseries." << endl;
                    return 1;
                }
                nCheck = static_cast<int>(bestMatchingValuesSequential[bestMatchingValuesSequential.size()-1]);

                // Calcola il tempo per le versioni parallele
                startTime = omp_get_wtime();
                findBestMatchParallel(timeseriesData, patternSequence, bestMatchingValuesParallel, nThreads, false);
                endTime = omp_get_wtime();
                totalTimeParallel += (endTime - startTime == 0 ? 1e-8 : endTime - startTime);

                if(bestMatchingValuesParallel.empty()){
                    cout << "No match found in the timeseries." << endl;
                    return 1;
                }
                // Controllo se il numero di confronti effettuati è lo stesso in tutte le versioni
                if(nCheck != bestMatchingValuesParallel[bestMatchingValuesParallel.size()-1]) {
                    cout << "Different number of checks" << endl;
                    return 1;
                }

                startTime = omp_get_wtime();
                findBestMatchParallelExplicitChunk(timeseriesData, patternSequence, bestMatchingValuesParallelExplicitChunk, nThreads, false);
                endTime = omp_get_wtime();
                totalTimeParallelExplicitChunk += (endTime - startTime == 0 ? 1e-8 : endTime - startTime);

                if(bestMatchingValuesParallelExplicitChunk.empty()){
                    cout << "No match found in the timeseries." << endl;
                    return 1;
                }
                if(nCheck != bestMatchingValuesParallelExplicitChunk[bestMatchingValuesParallelExplicitChunk.size()-1]) {
                    cout << "Different number of checks" << endl;
                    return 1;
                }

                startTime = omp_get_wtime();
                findBestMatchParallelExplicitChunkAtomic(timeseriesData, patternSequence, bestMatchingValuesParallelExplicitChunkAtomic, nThreads, false);
                endTime = omp_get_wtime();
                totalTimeParallelExplicitChunkAtomic += (endTime - startTime == 0 ? 1e-8 : endTime - startTime);

                if(bestMatchingValuesParallelExplicitChunkAtomic.empty()){
                    cout << "No match found in the timeseries." << endl;
                    return 1;
                }
                if(nCheck != bestMatchingValuesParallelExplicitChunkAtomic[bestMatchingValuesParallelExplicitChunkAtomic.size()-1]) {
                    cout << "Different number of checks" << endl;
                    return 1;
                }

                // Controlla se i risultati sono uguali in tutti i casi
                // NB. potrebbe capitare anche che ci siano più pattern che hanno la stessa SAD
                if(!equal(bestMatchingValuesSequential.begin(), bestMatchingValuesSequential.end(), bestMatchingValuesParallel.begin()) || !equal(bestMatchingValuesSequential.begin(), bestMatchingValuesSequential.end(), bestMatchingValuesParallelExplicitChunk.begin()) || !equal(bestMatchingValuesSequential.begin(), bestMatchingValuesSequential.end(), bestMatchingValuesParallelExplicitChunkAtomic.begin())){
                    cout << "Different results for sequential and parallel" << endl;
                    return 1;
                }

                for(int i = 1; i < nRepeat; i++){
                    startTime = omp_get_wtime();
                    findBestMatchSequential(timeseriesData, patternSequence, bestMatchingValuesSequential, false);
                    endTime = omp_get_wtime();
                    totalTimeSequential += (endTime - startTime == 0 ? 1e-8 : endTime - startTime);

                    startTime = omp_get_wtime();
                    findBestMatchParallel(timeseriesData, patternSequence, bestMatchingValuesParallel, nThreads, false);
                    endTime = omp_get_wtime();
                    totalTimeParallel += (endTime - startTime == 0 ? 1e-8 : endTime - startTime);

                    startTime = omp_get_wtime();
                    findBestMatchParallelExplicitChunk(timeseriesData, patternSequence, bestMatchingValuesParallelExplicitChunk, nThreads, false);
                    endTime = omp_get_wtime();
                    totalTimeParallelExplicitChunk += (endTime - startTime == 0 ? 1e-8 : endTime - startTime);

                    startTime = omp_get_wtime();
                    findBestMatchParallelExplicitChunkAtomic(timeseriesData, patternSequence, bestMatchingValuesParallelExplicitChunkAtomic, nThreads, false);
                    endTime = omp_get_wtime();
                    totalTimeParallelExplicitChunkAtomic += (endTime - startTime == 0 ? 1e-8 : endTime - startTime);
                }

                // Calcola i tempi medi e lo speedup
                double meanTimeSequential = totalTimeSequential / nRepeat;
                double meanTimeParallel = totalTimeParallel / nRepeat;
                double meanTimeParallelExplicitChunk = totalTimeParallelExplicitChunk / nRepeat;
                double meanTimeParallelExplicitChunkAtomic = totalTimeParallelExplicitChunkAtomic / nRepeat;

                double speedupParallel = meanTimeSequential / meanTimeParallel;
                double speedupExplicitChunk = meanTimeSequential / meanTimeParallelExplicitChunk;
                double speedupExplicitChunkAtomic = meanTimeSequential / meanTimeParallelExplicitChunkAtomic;

                // Trova il miglior speedup per questo numero di thread
                double bestSpeedup = speedupParallel;
                string currentBestVersion = "Base parallel";

                if(speedupExplicitChunk > bestSpeedup){
                    bestSpeedup = speedupExplicitChunk;
                    currentBestVersion = "Parallel with explicit chunk declaration";
                }
                if(speedupExplicitChunkAtomic > bestSpeedup){
                    bestSpeedup = speedupExplicitChunkAtomic;
                    currentBestVersion = "Parallel with explicit chunk and atomic";
                }

                double efficiency = bestSpeedup / nThreads;
                double currentScore = bestSpeedup * efficiency; // Punteggio basato su speedup ed efficienza
                double bestScore = bestOverallSpeedup * bestOverallEfficiency; // Punteggio del miglior risultato globale

                if(currentScore > bestScore){
                    bestOverallSpeedup = bestSpeedup;
                    bestOverallEfficiency = efficiency;
                    bestNThreads = nThreads;
                    bestVersion = currentBestVersion;
                }

                // Salva i risultati nel file
                resultsFile << nThreads << "," << sequenceSize << "," << bestSpeedup << "," << efficiency << "," << currentBestVersion << "," << meanTimeSequential << "\n";

                cout << "N° threads: " << nThreads
                     << ", Best Speedup: " << bestSpeedup
                     << ", Efficiency: " << efficiency
                     << ", Best Version: " << currentBestVersion;
                cout << ", Mean time sequential: " << meanTimeSequential << " s" << endl;
            }

            // Stampa il miglior numero di thread e la versione per questa dimensione
            cout << endl;
            cout << "=> best number of threads for sequence size = " << sequenceSize << " is " << bestNThreads << ", when using version: " << bestVersion << endl;
            cout << "Best speedup = " << bestOverallSpeedup;
            checkTypeSpeedUp(bestOverallSpeedup, bestNThreads);
            cout << "Efficiency = " << bestOverallEfficiency << endl;
            cout << endl;
        }

        resultsFile.close();
        cout << "Results saved in " << outputRsultsFile << endl;
    }

    endExeTime = omp_get_wtime();

    // Mostra il tempo formattato
    cout << "Total execution time: "
            << static_cast<int>(endExeTime - startExeTime) / 60 << " minutes and "
            << static_cast<int>(endExeTime - startExeTime) % 60 << " seconds" << std::endl;

    return 0;
}

