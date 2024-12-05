#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <vector>
#include "timeSeries.h"
#include "utils.h"

using namespace std;

// Chiede all'utente di inserire un valore intero in un certo range
int getIntFromUser(int min, int max){
    int input;
    while(true){
        cin >> input;

        if(cin.fail() || input < min || input > max){
            cin.clear();
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            cout << "Invalid input. Reinsert: ";
        }else{
            return input;
        }
    }
}

// Calcola il tempo trascorso tra 2 tempi, se la differenza è troppo piccola ne mette una di default
double computeElapsedTime(const double startTime, const double endTime){
    return endTime - startTime == 0 ? 1e-8 : endTime - startTime;
}

// Controlla se il file specificato esiste nella directory
bool fileExists(const string& filename){
    return filesystem::exists(filename);
}

// Funzione che controlla di che tipo è lo speedup
void checkTypeSpeedUp(const double speedUp, const int nThreads){
    const double epsilon_linear_speedup = 0.5;

    if(speedUp == nThreads){
        cout << " => ideal speedup" << endl;
    }else if(speedUp > nThreads){
        cout << " => superlinear speedup" << endl;
    }else if(abs(speedUp - nThreads) < epsilon_linear_speedup){
        cout << " => linear speedup" << endl;
    }else if(speedUp < 1){
        cout << " => no speedup" << endl;
    }else{
        cout << " => sublinear speedup" << endl;
    }
}

// Calcola il SAD tra una time series ed una sottosequenza di una timeseries più grande
double calculateSAD(const vector<TimeSeriesData>& timeseries, const vector<double>& target_sequence, size_t start){
    double sad = 0.0;
    // In C++ i vettori sono array dinamici contigui in memoria, quindi si può accedere direttamente (senza scorrerlo) a qualsiasi indice dell'array in tempo costante
    for(vector<double>::size_type j = 0; j < target_sequence.size(); j++){
        sad += abs(timeseries[start + j].value - target_sequence[j]);
    }
    return sad;
}