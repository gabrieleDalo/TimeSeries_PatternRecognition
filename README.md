# TimeSeries_PatternRecognition
A simple project to perform pattern recognition on a time series of data and evaluate performances on sequential and parallel implementation.<br/>
The file mockseries_data.csv contains the time series used in the program, generated with TimeSeriesGenerator.py.<br/>
By executing our project (either the one with AoS or SoA) we can generate a .csv file containing the results of our experiments such as speedup_results_AoS.csv and speedup_results_SoA.csv.<br/>
The file plot_results.py is a script that when you launch asks for the name of a .csv file and generate plots based on the data contained in it, by giving the name speedup_results_AoS.csv or speedup_results_SoA.csv we can generate plots from the results of our project, as the ones saved in the Example_results folder.
