from datetime import timedelta
from datetime import datetime
from mockseries.trend import LinearTrend
from mockseries.seasonality import SinusoidalSeasonality
from mockseries.noise import RedNoise
from mockseries.seasonality import YearlySeasonality
from mockseries.utils.timedeltas import JANUARY, FEBRUARY, MARCH, APRIL, MAY, JUNE, JULY, AUGUST, SEPTEMBER, OCTOBER, NOVEMBER, DECEMBER
from mockseries.utils import datetime_range
from mockseries.utils import plot_timeseries, write_csv

constraints = {
        JANUARY: 4.3,
        FEBRUARY: 5.1,
        MARCH: 8.6,
        APRIL: 12.3,
        MAY: 16.4,
        JUNE: 21.1,
        JULY: 23.8,
        AUGUST: 23.7,
        SEPTEMBER: 18.9,
        OCTOBER: 14.5,
        NOVEMBER: 9.5,
        DECEMBER: 5.4,
    }

# A timeseries can be expressed as a combination of 3 components: trend, seasonality and noise.

# The trend is the long term, average change of the timeseries, such as the increase in CO2 emissions.
# Seasonality is a cyclic pattern in the timeseries, such as the impact of the day-night cycle.
# Noise represents irregular and random changes of the timeseries. 

WARMING_COEFFICIENT = 1  + 1.41
PERIOD_YEARS = 2024
trend = LinearTrend(coefficient=WARMING_COEFFICIENT, time_unit=timedelta(days=365.25*PERIOD_YEARS))
normalized_yearly_seasonality = YearlySeasonality(constraints, normalize=True)
noise = RedNoise(0,1)
timeseries = 8.4 * normalized_yearly_seasonality + trend + noise     # 8.4 scala l'ampiezza della stagionalità determinando quanto forte è la variazione stagionale rispetto ad altre componenti della serie temporale

# datetime_range helps you get the time points of a given granularity in a timeframe.
# For instance:
# timeframe of 1 hour, with a granularity of 1 minute: 60 points;
# timeframe of 1 hour, with a granularity of 1 second: 3600 points.

ts_index = datetime_range(
    granularity=timedelta(days=1),
    start_time=datetime(1001, 1, 1),
    end_time=datetime(3024, 1, 1),
)

ts_values = timeseries.generate(ts_index)

#print(ts_index, ts_values)
plot_timeseries(ts_index, ts_values, save_path="mockseries_plot.png")
write_csv(ts_index, ts_values, "mockseries_data.csv")

