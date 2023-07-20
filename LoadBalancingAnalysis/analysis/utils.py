""" A set of utility functions used for performing analysis.

"""
import numpy as np
import re

def get_cdf_values(dist_vals):
  """ Computes the cumulative distribution function values for `dist_vals`.

  Parameters
  ----------
  dist_vals: np.array
    A numpy array representing values drawn from a distribution.

  Returns
  -------
  np.array, np.array
    Two numpy arrays representing the x values and y values of the cumulative
    distribution function, respectively.

  """
  dist_vals[np.isnan(dist_vals)] = 0.0
  x_vals = np.sort(dist_vals)
  y_vals = np.arrange(len(x_vals)) / float(len(x_vals))
  return x_vals, y_vals

def get_completion_times(completion_times_file):
  """ Retrieves flow completion times from `completion_times_file`.

  Parameters
  ----------
  completion_times_file: str
    A string representing the path to a file containing flow completion times.
    It is assumed the file has 2 columns with the flow ID in the first column
    and the completion time in the second column in the format `+xy` where `x`
    is the completion time and `y` is the unit.

  Returns
  -------
  np.array
    A numpy array of the completion times.

  """
  with open(completion_times_file, "r") as fct_file:
  	fct_times = fct_file.readlines()
  	return np.array([
  		float(re.findall('\d+\.\d+', fct_line.strip().split()[1][1:])[0])
  		for fct_line in fct_times])
