#include "findeBestMatch.h"
#include <iostream>
#include <omp.h>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <vector>
#include <random>
#include "timeSeries.h"
#include "utils.h"

using namespace std;

// Funzione per trovare la finestra con la minima SAD (sequenziale)
void findBestMatchSequential(const vector<TimeSeriesData>& timeseries, const vector<double>& target_sequence, vector<double>& bestMatchingValues, const bool printOn){
    string bestMatching_initialDate;
    string bestMatching_finalDate;
    double min_sad = numeric_limits<double>::infinity();
    int window_size = static_cast<int>(target_sequence.size());
    int nCheck = 0;
    bestMatchingValues.clear();

    // Scorri la timeseries con una finestra della dimensione della target_sequence, di un indice alla volta
    for(vector<double>::size_type i = 0; i <= timeseries.size() - window_size; i++){
        double sad = calculateSAD(timeseries, target_sequence, i);
        nCheck++;

        if(sad < min_sad){
            min_sad = sad;
            bestMatchingValues.clear();
            for(size_t j = 0; j < window_size; j++){
                bestMatchingValues.push_back(timeseries[i + j].value);
            }
            bestMatching_initialDate = timeseries[i].timestamp;
            bestMatching_finalDate = timeseries[i+window_size-1].timestamp;
        }
    }
    bestMatchingValues.push_back(nCheck);

    if(printOn){
        cout << "N° of checking: " << nCheck << endl;
        cout << "Best matching found at date: " << bestMatching_initialDate << " - " << bestMatching_finalDate << endl;
        cout << "Min SAD value: " << min_sad << endl;
    }
}

// Funzione per trovare la finestra con la minima SAD (parallela)
void findBestMatchParallel(const vector<TimeSeriesData>& timeseries, const vector<double>& target_sequence, vector<double>& bestMatchingValues, const int nThreads, const bool printOn){
    string bestMatching_initialDate, bestMatching_finalDate;
    double global_min_sad = std::numeric_limits<double>::infinity();
    const size_t window_size = target_sequence.size();        // Dimensione della sequenza da cercare (= finestra da scorrere)
    const size_t data_size = timeseries.size();               // Dimensione della timeseries
    int nCheck = 0;                                       // Contatore del numero di controlli effettuati
    bestMatchingValues.clear();

    // Parallelizzazione del ciclo principale
    #pragma omp parallel num_threads(nThreads) default(none) \
    shared(timeseries, target_sequence, window_size, data_size, global_min_sad, bestMatchingValues, bestMatching_initialDate, bestMatching_finalDate, nCheck)
    {
        string local_bestMatching_initialDate, local_bestMatching_finalDate;
        double local_min_sad = std::numeric_limits<double>::infinity();     // Ogni thread calcola il proprio best matching locale
        vector<double> local_bestMatchingValues;
        local_bestMatchingValues.reserve(window_size);
        int local_nCheck = 0;

        #pragma omp for
        for(vector<double>::size_type i = 0; i <= (data_size - window_size); i++){
            double sad = calculateSAD(timeseries, target_sequence, i);
            local_nCheck++;

            // Se il valore di SAD corrente è il più basso trovato finora, aggiorna i valori locali
            if(sad < local_min_sad){
                local_min_sad = sad;
                local_bestMatchingValues.clear();
                for(size_t j = 0; j < window_size; j++){
                    local_bestMatchingValues.push_back(timeseries[i + j].value);
                }
                local_bestMatching_initialDate = timeseries[i].timestamp;
                local_bestMatching_finalDate = timeseries[i + window_size - 1].timestamp;
            }
        }

        // Aggiorna i risultati globali con i risultati locali
        #pragma omp critical
        {
            nCheck += local_nCheck;
            if(local_min_sad < global_min_sad){
                global_min_sad = local_min_sad;
                bestMatchingValues = local_bestMatchingValues;
                bestMatching_initialDate = local_bestMatching_initialDate;
                bestMatching_finalDate = local_bestMatching_finalDate;
            }
        }
    }
    bestMatchingValues.push_back(nCheck);

    if(printOn){
        cout << "N° of checking: " << nCheck << endl;
        cout << "Best matching found at date: " << bestMatching_initialDate << " - " << bestMatching_finalDate << endl;
        cout << "Min SAD value: " << global_min_sad << endl;
    }
}

// Simile a prima ma con schedulazione esplicita dei chunk da assegnare ai threads (prima era 1 iterazione a thread di default)
void findBestMatchParallelExplicitChunk(const vector<TimeSeriesData>& timeseries, const vector<double>& target_sequence, vector<double>& bestMatchingValues, const int nThreads, const bool printOn){
    string bestMatching_initialDate, bestMatching_finalDate;
    double global_min_sad = std::numeric_limits<double>::infinity();
    const size_t window_size = target_sequence.size();
    const size_t data_size = timeseries.size();
    int nCheck = 0;
    bestMatchingValues.clear();

    // Calcolo esplicitamente il chunk size per ciascun thread
    const size_t chunkSize = ceil(static_cast<double>(data_size - window_size + 1) / nThreads);

    #pragma omp parallel num_threads(nThreads) default(none) \
    shared(chunkSize, timeseries, target_sequence, window_size, data_size, global_min_sad, bestMatchingValues, bestMatching_initialDate, bestMatching_finalDate, nCheck)
    {
        string local_bestMatching_initialDate, local_bestMatching_finalDate;
        double local_min_sad = std::numeric_limits<double>::infinity();
        vector<double> local_bestMatchingValues;
        local_bestMatchingValues.reserve(window_size);
        int local_nCheck = 0;

        #pragma omp for schedule(static, chunkSize)
        for(vector<double>::size_type i = 0; i <= (data_size - window_size); i++){
            double sad = calculateSAD(timeseries, target_sequence, i);
            local_nCheck++;

            if(sad < local_min_sad){
                local_min_sad = sad;
                local_bestMatchingValues.clear();
                for(size_t j = 0; j < window_size; j++){
                    local_bestMatchingValues.push_back(timeseries[i + j].value);
                }
                local_bestMatching_initialDate = timeseries[i].timestamp;
                local_bestMatching_finalDate = timeseries[i + window_size - 1].timestamp;
            }
        }

        #pragma omp critical
        {
            nCheck += local_nCheck;
            if(local_min_sad < global_min_sad){
                global_min_sad = local_min_sad;
                bestMatchingValues = local_bestMatchingValues;
                bestMatching_initialDate = local_bestMatching_initialDate;
                bestMatching_finalDate = local_bestMatching_finalDate;
            }
        }
    }

    if(printOn){
        cout << "N° of checking: " << nCheck << endl;
        cout << "Best matching found at date: " << bestMatching_initialDate << " - " << bestMatching_finalDate << endl;
        cout << "Min SAD value: " << global_min_sad << endl;
    }
    bestMatchingValues.push_back(nCheck);
}

// Simile a prima ma con schedulazione esplicita dei chunk da assegnare ai threads ed uso atomic quando possibile
void findBestMatchParallelExplicitChunkAtomic(const vector<TimeSeriesData>& timeseries, const vector<double>& target_sequence, vector<double>& bestMatchingValues, const int nThreads, const bool printOn){
    string bestMatching_initialDate, bestMatching_finalDate;
    double global_min_sad = std::numeric_limits<double>::infinity();
    const size_t window_size = target_sequence.size();
    const size_t data_size = timeseries.size();
    int nCheck = 0;
    bestMatchingValues.clear();

    const size_t chunkSize = ceil(static_cast<double>(data_size - window_size + 1) / nThreads);

    #pragma omp parallel num_threads(nThreads) default(none) \
    shared(chunkSize, timeseries, target_sequence, window_size, data_size, global_min_sad, bestMatchingValues, bestMatching_initialDate, bestMatching_finalDate, nCheck)
    {
        string local_bestMatching_initialDate, local_bestMatching_finalDate;
        double local_min_sad = std::numeric_limits<double>::infinity();
        vector<double> local_bestMatchingValues;
        local_bestMatchingValues.reserve(window_size);
        int local_nCheck = 0;

        #pragma omp for schedule(static, chunkSize)
        for(vector<double>::size_type i = 0; i <= (data_size - window_size); i++){
            double sad = calculateSAD(timeseries, target_sequence, i);
            local_nCheck++;

            if(sad < local_min_sad){
                local_min_sad = sad;
                local_bestMatchingValues.clear();
                for(size_t j = 0; j < window_size; j++){
                    local_bestMatchingValues.push_back(timeseries[i + j].value);
                }
                local_bestMatching_initialDate = timeseries[i].timestamp;
                local_bestMatching_finalDate = timeseries[i + window_size - 1].timestamp;
            }
        }

        #pragma omp atomic update
            nCheck = nCheck + local_nCheck;

        // Così è inoltre possibile evitare di accedere in sezione critica quando non è realmente necessario
        if(local_min_sad < global_min_sad){
            #pragma omp critical
            {
                if(local_min_sad < global_min_sad){     // Il doppio controllo serve per garantire la consistenza, il minimo globale potrebbe essere aggiornato da qualcun altro prima che io entri in sezione critica
                    global_min_sad = local_min_sad;
                    bestMatchingValues = local_bestMatchingValues;
                    bestMatching_initialDate = local_bestMatching_initialDate;
                    bestMatching_finalDate = local_bestMatching_finalDate;
                }
            }
        }
    }
    bestMatchingValues.push_back(nCheck);

    if(printOn){
        cout << "N° of checking: " << nCheck << endl;
        cout << "Best matching found at date: " << bestMatching_initialDate << " - " << bestMatching_finalDate << endl;
        cout << "Min SAD value: " << global_min_sad << endl;
    }
}