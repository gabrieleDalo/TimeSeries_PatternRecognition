import pandas as pd
import matplotlib.pyplot as plt
import os

# Funzione per chiedere all'utente il nome del file
def get_file_from_user():
    while True:
        filename = input("Insert CSV file name: ").strip()
        # Controlla se il file esiste
        if os.path.exists(filename):
            if filename.lower().endswith('.csv'):
                return filename
            else: 
                print(f"File '{filename}' isn't a CSV file.")
        else:
            print(f"Cannot find '{filename}', please try again.")

# Funzione per determinare il tipo di struttura (AoS o SoA) dal nome del file
def get_structure_from_filename(filename):
    if 'AoS' in filename:
        return 'AoS'
    elif 'SoA' in filename:
        return 'SoA'
    else:
        return ''

# Funzione per creare e salvare i grafici
def plot_and_save(data, sequence_size, output_folder):
    # Filtra i dati per la sequence size specifica
    filtered_data = data[data['SequenceSize'] == sequence_size]

    # Crea il grafico
    fig, ax1 = plt.subplots(figsize=(10, 6))

    # Grafico dello speedup
    ax1.set_title(f"Sequence size {sequence_size}")
    ax1.set_xlabel("N° of threads")
    ax1.set_ylabel("Speedup", color="tab:blue")
    ax1.plot(filtered_data['NThreads'], filtered_data['Speedup'], label="Speedup", color="tab:blue", marker='o')
    ax1.tick_params(axis='y', labelcolor="tab:blue")

    # Aggiungi un secondo asse per l'efficienza
    ax2 = ax1.twinx()
    ax2.set_ylabel("Efficiency", color="tab:orange")
    ax2.plot(filtered_data['NThreads'], filtered_data['Efficiency'], label="Efficienza", color="tab:orange", marker='x')
    ax2.tick_params(axis='y', labelcolor="tab:orange")

    fig.tight_layout()
    plt.grid()

    # Salva il grafico nella cartella specificata
    plot_filename = os.path.join(output_folder, f"plot_sequence_size_{sequence_size}.png")
    plt.savefig(plot_filename)
    plt.close(fig)
    
# Chiedi all'utente di inserire il nome del file
filename = get_file_from_user()

# Determina la struttura (AoS o SoA) dal nome del file
structure_type = get_structure_from_filename(filename)

# Imposta la cartella di output in base al tipo di struttura
output_folder = f"results_plots_{structure_type}"

# Crea la cartella per i plot se non esiste
os.makedirs(output_folder, exist_ok=True)

# Leggi il CSV
data = pd.read_csv(filename)
data.columns = data.columns.str.strip()  # Rimuove eventuali spazi extra nei nomi delle colonne

# Ottieni tutte le dimensioni della sequence size disponibili
sequence_sizes = data['SequenceSize'].unique()

# Crea e salva un grafico per ogni sequence size
for size in sequence_sizes:
    plot_and_save(data, size, output_folder)

print(f"Plots saved in folder: {output_folder}")
