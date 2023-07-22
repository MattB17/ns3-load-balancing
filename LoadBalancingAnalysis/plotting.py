""" A set of utility functions used to create plots.

"""
import numpy as np
import matplotlib.pyplot as plt

def plot_cdf(cdf_vals, quantiles, plot_title):
  """ Plots a CDF distribution based on `cdf_vals` and `quantiles`.

  Parameters
  ----------
  cdf_vals: np.array
    The cdf values to be plotted.
  quantiles: np.array
    The quantiles corresponding to the CDF values.
  plot_title: str
    A string representing a title for the plot.

  Returns
  -------
  None

  """
  plt.plot(cdf_vals, quantiles)
  plt.title(plot_title)
  plt.show()