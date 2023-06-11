#include "cdf.h"

// Initialize a CDF distribution.
void InitCdf(struct CdfTable* table) {
	if (!table)
		return;
	
	table->entries = (struct CdfEntry*)malloc(TG_CDF_TABLE_ENTRY * sizeof(struct CdfEntry));
	table->num_entries = 0;
	table->max_entries = TG_CDF_TABLE_ENTRY;
	table->min_quant = 0;
	table->max_quant = 1;

	if (!(table->entries))
		perror("Error: malloc entries in InitCdf()");
}

// Free resources of a CDF distribution.
void FreeCdf(struct CdfTable* table) {
	if (table)
		free(table->entries);
}

void LoadCdf(struct CdfTable* table, const char* file_name) {
	FILE *fd = NULL;
	char line[256] = {0};
	struct CdfEntry* e = NULL;

	if (!table)
		return;

	fd = fopen(file_name, "r");
	if (!fd)
		perror("Error: opening the CDF file in LoadCdf()");

	while (fgets(line, sizeof(line), fd)) {
		// Resize entries
		if (table->num_entries >= table->max_entries) {
			table->max_entries *= 2;
			e = (struct CdfEntry*)malloc(table->max_entries * sizeof(struct CdfEntry));
			if (!e)
				perror("Error: malloc entries in LoadCdf()");
			for (int table_idx = 0; table_idx < table->num_entries; table_idx++)
				e[table_idx] = table->entries[table_idx];
			free(table->entries);
			table->entries = e;
		}
		// Extract the value and quantile for the CDF entry.
		sscanf(line, "%lf %lf",
			&(table->entries[table->num_entries].value),
			&(table->entries[table->num_entries].quantile));

		if (table->min_quant > table->entries[table->num_entries].quantile)
			table->min_quant = table->entries[table->num_entries].quantile;
		if (table->max_quant < table->entries[table->num_entries].quantile)
			table->max_quant = table->entries[table->num_entries].quantile;

		table->num_entries++;
	}
	fclose(fd);
}

// Print CDF distribution information.
void PrintCdf(struct CdfTable* table) {
	if (!table)
		return;
	for (int table_idx = 0; table_idx < table->num_entries; table_idx++)
		printf("%.2f %.2f\n", table->entries[table_idx].value,
			table->entries[table_idx].quantile);
}

// Get the average value of the CDF distribution.
double AvgCdf(struct CdfTable* table) {
	int table_idx = 0;
	double avg = 0;
	double value, prob;

	if (!table)
		return 0;

	for (table_idx = 0; table_idx < table->num_entries; table_idx++) {
		if (table_idx == 0) {
			value = table->entries[table_idx].value / 2;
			prob = table->entries[table_idx].quantile;
		} else {
			value = (table->entries[table_idx].value +
				table->entries[table_idx - 1].value) / 2;
			prob = (table->entries[table_idx].quantile -
				table->entries[table_idx - 1].quantile);
		}
		avg += (value * prob);
	}
	return avg;
}

double Interpolate(double x, double x1, double y1, double x2, double y2) {
	if (x1 == x2) {
		return (y1 + y2) / 2;
	} else {
		return y1 + (((x - x1) / (x2 - x1)) * (y2 - y1));
	}
}

// Generate a random floating point number from min to max.
double RandRange(double min, double max) {
	return min + ((rand() / RAND_MAX) * (max - min));
}

// Generate a random value based on the CDF distribution.
double GenRandomCdfValue(struct CdfTable* table) {
    if (!table)
    	return 0;

	int table_idx = 0;
	double x = RandRange(table->min_quant, table->max_quant);

	for (table_idx = 0; table_idx < table->num_entries; table_idx++) {
		if (x <= table->entries[table_idx].quantile) {
			if (table_idx == 0)
				return Interpolate(x, 0, 0,
					               table->entries[table_idx].quantile,
					               table->entries[table_idx].value);
			else
				return Interpolate(x,
					               table->entries[table_idx - 1].quantile,
					               table->entries[table_idx - 1].value,
					               table->entries[table_idx].quantile,
					               table->entries[table_idx].value);
		}
	}
	return table->entries[table->num_entries - 1].value;
}