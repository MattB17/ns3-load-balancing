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