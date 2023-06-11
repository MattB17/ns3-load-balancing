#ifndef CDF_H
#define CDF_H

#include <stdio.h>
#include <stdlib.h>

#define TG_CDF_TABLE_ENTRY 32

struct CdfEntry {
	double value;
	double quantile;
};

// CDF distribution.
struct CdfTable {
	struct CdfEntry* entries;  // entries in CDF (value, quantile).
	int num_entries;           // number of entries in the CDF table.
	int max_entries;           // maximum number of entries in CDF table.
	double min_quant;          // minimum value of CDF (default 0).
	double max_quant;          // maximum value of CDF (default 1).
};

// Initialize a CDF distribution.
void InitCdf(struct CdfTable* table);

// Free resources for the CDF distribution
void FreeCdf(struct CdfTable* table);

// Get CDF distribution from a given file.
void LoadCdf(struct CdfTable* table, const char *file_name);

// Print CDF distribution information.
void PrintCdf(struct CdfTable* table);

// Get average value of CDF distribution.
double AvgCdf(struct CdfTable* table);

// Generate a random value based on a CDF distribution.
double GenRandomCdfValue(struct CdfTable* table);

#endif  // CDF_H