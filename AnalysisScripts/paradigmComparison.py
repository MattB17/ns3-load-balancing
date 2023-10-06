""" A script to compare the load balancing paradigms being compared.

In traditional load balancing, the aim is for point solutions that perfectly
balance load among the links. But then in periods where the load is fluctuating
this previous perfect balance leads to a high degree of transient imbalance.
Instead, we propose a new paradigm that aims for a small degree of imbalance
in order to better adjust to fluctuating load.

"""

import sys
import numpy as np
import matplotlib.pyplot as plt

if __name__ == "__main__":
    ideal_x = np.array([0, 0.25, 0.5, 0.75, 1.0])

    baseline_with_errors = (sys.argv[1].lower() == "true")
    baseline_x = np.array([])
    baseline_y = np.array([])
    if baseline_with_errors:
        baseline_x = np.array([0, 0.02, 0.25, 0.3, 0.5, 0.58, 0.75, 0.9, 1.0])
        baseline_y = np.array([0, 0.1, 0.25, 0.4, 0.5, 0.72, 0.75, 1.05, 1.0])
    else:
        baseline_x = np.array([0.25, 0.5, 0.75, 1.0])
        baseline_y = np.array([0.25, 0.5, 0.75, 1.0])

    plt.plot(ideal_x, ideal_x, color='red')

    if baseline_with_errors:
        plt.plot(baseline_x, baseline_y, color='black')
    else:
        plt.scatter(baseline_x, baseline_y, marker="*")

    plot_new_paradigm = (sys.argv[2].lower() == "true")
    if plot_new_paradigm:
        # we use the rough equation y = mx + b where m = 0.9 and b = 0.1.
        # This line passes through (0, 0.1) and (1.0, 1.0). In particular, it
        # gets closer to the ideal line as x increases.
        new_x = np.array([0.0, 0.25, 0.5, 0.75, 1.0])
        new_y = np.array([0.1, 0.325, 0.55, 0.775, 1.0])
        plt.plot(new_x, new_y, color='blue')

    ticks = [0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1]
    plt.xticks(ticks)
    plt.yticks(ticks)
    plt.xlabel("Average Link Load")
    plt.ylabel("Max Link Load")
    plt.show()
