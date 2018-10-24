CREATE TABLE weather (
	record_id SERIAL PRIMARY KEY,
	record_start TIMESTAMP NOT NULL,
	record_end TIMESTAMP NOT NULL,
	record_temp DECIMAL(10, 4),
	record_wind_speed DECIMAL(10, 4),
	record_wind_spins INTEGER NOT NULL,
	record_rain_depth DECIMAL(10, 4),
	record_rain_tips INTEGER NOT NULL
);