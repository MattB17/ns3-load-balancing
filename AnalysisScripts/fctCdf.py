""" A script to examine flow completion times.

Usage:
  `python fctCdf.py <fct_file_name>`

Where <fct_file_name> is the name of the file containing flow completion
times.

"""
import sys
from LoadBalancingAnalysis import plotting, utils

if __name__ == "__main__":
  x, y = utils.get_cdf_values(utils.get_completion_times(sys.argv[1]))
  plotting.plot_cdf(x, y, "FCT")