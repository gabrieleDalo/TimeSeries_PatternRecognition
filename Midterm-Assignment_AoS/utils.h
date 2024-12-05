#ifndef UTILS_H
#define UTILS_H

#include <omp.h>
#include <vector>
#include <string>
#include <random>
#include "timeSeries.h"

// Chiedo all'utente se devo ripetere l'esecuzione più volte
inline bool askYesOrNo(){
    int input;
    while(true){
        std::cin >> input;

        if(std::cin.fail() || (input != 1 && input != 0)){
            std::cin.clear();  // Resetta eventuali errori di input
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // Ignora l'input errato
            std::cout << "Invalid input. Insert 1 for yes or 0 per no: ";       // Stampa un messaggio di errore
        }else{
            return static_cast<bool>(input);
        }
    }
}

int getIntFromUser(int min, int max);
double computeElapsedTime(const double startTime, const double endTime);

// Genera una sequenza casuale di valori double di una certa lunghezza, con valori compresi tra un min ed un max
inline void generateRandomSequence(std::vector<double>& sequence, int length, double min, double max){
    sequence.reserve(length);

    double startTime;
    startTime = omp_get_wtime();

    // Inizializza il generatore di numeri casuali
    std::random_device rd;  // Seed
    std::mt19937 gen(rd()); // Mersenne Twister PRNG con il seed ottenuto
    std::uniform_real_distribution<> dis(min, max); // Distribuzione uniforme su [min, max]

    for(std::vector<double>::size_type i = 0; i < length; i++){
        sequence.push_back(dis(gen)); // Genera un numero casuale nell'intervallo e lo inserisce nella sequenza
    }

    std::cout << "Total time (sequential): " << computeElapsedTime(startTime, omp_get_wtime()) << " s" << std::endl;
}

// Acquisisce una sottoinsieme di una timeseries e la perturba
inline void acquireSampleTimeSeries(std::vector<double>& sequence, int length, const std::vector<TimeSeriesData>& timeseries){
    sequence.reserve(length);
    sequence.resize(length);

    double perturbation = 0.5;
    double startTime;
    startTime = omp_get_wtime();

    // Acquisisce e perturba gli ultimi length valori della timeseries originale
    std::transform(timeseries.end() - length, timeseries.end(), sequence.begin(),
                       [perturbation](const TimeSeriesData& data) {
                           return data.value + perturbation;});

    std::cout << "Total time (sequential): " << computeElapsedTime(startTime, omp_get_wtime()) << " s" << std::endl;
}

bool fileExists(const std::string& filename);

// Legge una timeseries da un file CSV in modo sequenziale
inline void readCSV(std::vector<TimeSeriesData>& timeseries, const std::string& filename){
    std::ifstream file(filename);
    double readingTime = 0.0;

    readingTime = omp_get_wtime();

    if(!file.is_open()){
        std::cerr << "Error: can't open file " << filename << std::endl;
    }else{
        std::string line;

        // Leggi i dati riga per riga
        while(getline(file, line)){
            std::istringstream ss(line);
            std::string timestamp;
            std::string valueStr;

            // Leggi il timestamp (prima colonna)
            if(!getline(ss, timestamp, ';')) continue;

            // Leggi il valore (seconda colonna)
            if(!getline(ss, valueStr, ';')) continue;

            // Aggiungi il dato alla lista
            timeseries.push_back({timestamp, stod(valueStr)});    // stod converte il valore in double

            //std::cout << "Timestamp: " << timestamp << ", Value: " << stod(valueStr) << std::endl;
        }

        file.close();
    }

    readingTime = computeElapsedTime(readingTime, omp_get_wtime());

    std::cout << "N° of total timestamps: " << timeseries.size() << std::endl;
    std::cout << "Reading time (sequential): " << readingTime << " s" << std::endl;
    std::cout << std::endl;
}

// Legge una timeseries da un file CSV in modo parallelo
inline void readCSVParallel(std::vector<TimeSeriesData>& timeseries, const std::string& filename, const int nThreads) {
    std::ifstream file(filename, std::ios::ate); // Apertura in modalità "ate" che posiziona il puntatore alla fine, consentendo di ottenere la dimensione del file con file.tellg()
    double readingTime = 0.0;

    readingTime = omp_get_wtime();

    if(!file.is_open()){
        std::cerr << "Error: can't open file " << filename << std::endl;
    }else{
        size_t fileSize = file.tellg(); // Ottieni la dimensione del file
        file.seekg(0, std::ios::beg);  // Il puntatore viene poi riportato all'inizio

        // Viene allocata una stringa grande quanto il file ed il contenuto viene letto direttamente in questa stringa
        // E' necessario fare così per la lettura parallela poichè un CSV può essere letto direttamente solo riga per riga
        std::string fileContent;
        fileContent.resize(fileSize);
        file.read(&fileContent.at(0), fileSize);
        file.close();

        // Questo blocco divide il contenuto del file messo come stringa in righe, usando \n come delimitatore.
        std::vector<std::string> lines;
        size_t lineStart = 0;
        for(size_t i = 0; i < fileContent.size(); ++i){
            if(fileContent.at(i) == '\n'){      // Controlla se la riga è finita
                lines.emplace_back(fileContent.substr(lineStart, i - lineStart));
                lineStart = i + 1;
            }
        }
        if(lineStart < fileContent.size()){             // Dopo l'ultimo \n, la riga rimanente viene aggiunta a lines (se presente)
            lines.emplace_back(fileContent.substr(lineStart));
        }

        size_t nLines = lines.size();
        std::vector<std::string> timestamps(nLines);
        std::vector<double> values(nLines);

        #pragma omp parallel for num_threads(nThreads) default(none) \
        shared(nLines, timestamps, values, lines, timeseries)
            for(size_t i = 0; i < nLines; i++){
                std::istringstream ss(lines.at(i));
                std::string timestamp, valueStr;
                if(getline(ss, timestamp, ';') && getline(ss, valueStr, ';')){          // Ogni riga viene suddivisa in timestamp e valueStr con std::getline usando ; come separatore
                    timestamps[i] = timestamp;
                    values[i] = std::stod(valueStr);
                }else{
                    timestamps[i] = ""; // Gestione errori di parsing
                    values[i] = std::numeric_limits<double>::quiet_NaN();               // Errori di parsing sono gestiti assegnando valori vuoti o NaN
                }
            }
        // I dati sono relativamente pochi, quindi non conviene parallelizzare anche questo secondo ciclo, sono le operazioni di I/O ad essere più lente => da parallelizzare
        for(size_t i = 0; i < nLines; i++){
            if(!timestamps[i].empty() && !std::isnan(values[i])){                       // Filtra i valori non validi
                timeseries.push_back({timestamps[i], values[i]});
                //std::cout << "Timestamp: " << timestamps[i] << ", Value: " << values[i] << std::endl;
            }
        }
    }

    readingTime = computeElapsedTime(readingTime, omp_get_wtime());

    std::cout << "N° of total timestamps: " << timeseries.size() << std::endl;
    std::cout << "Reading time (parallel) with " << nThreads << " threads : " << readingTime << " s" << std::endl;
    std::cout << std::endl;
}

// Controlla se 2 vettori di TimeSeries sono uguali
inline bool checkTimeseriesEquality(const std::vector<TimeSeriesData>& timeseries, const std::vector<TimeSeriesData>& timeseries2){
    if(timeseries.size() != timeseries2.size()){
        return false;
    }

    // Confronta gli elementi dei vettori uno per uno
    for(std::vector<double>::size_type i = 0; i < timeseries.size(); i++){
        if((timeseries[i].timestamp != timeseries2[i].timestamp) || (timeseries[i].value != timeseries2[i].value)){
            return false;
        }
    }

    return true;
}

inline void printVector(const std::vector<double>& vectorToPrint, size_t size){
    if(!vectorToPrint.empty()){
        if(vectorToPrint.size() <= 20){
            std::cout << "Matched values: ";
            for(std::vector<double>::size_type i = 0; i < size; i++){
                std::cout << vectorToPrint[i] << "; ";
            }
            std::cout << std::endl;
        }
    }else{
        std::cout << "No match found in the timeseries." << std::endl;
    }
}

void checkTypeSpeedUp(const double speedUp, const int nThreads);
double calculateSAD(const std::vector<TimeSeriesData>& timeseries, const std::vector<double>& target_sequence, size_t start);

#endif //UTILS_H
