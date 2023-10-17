""" A script to generate the time in system vs utilization for a M/M/1 queue.

This is used to analyze queueing behaviour at different utilization levels and
what the effect of a small change in utilization level means in terms of the
average time that a packet spends in the queue.

"""
import sys
import numpy as np
import matplotlib.pyplot as plt

if __name__ == "__main__":
    x = np.linspace(0, 0.99, 100)
    y = (1 / (1 - x))
    plt.plot(x, y)

    with_annotations = (sys.argv[1].lower() == "true")

    if (with_annotations):
        x1 = np.array([0.5, 0.55])
        y1 = (1 / (1 - x1))
        plt.scatter(x1, y1, marker="o", s=50, color="black")
        plt.text(x1[0], y1[0] + 1.5, "a", fontsize=12)
        plt.text(x1[1], y1[1] + 1.5, "b", fontsize=12)
        plt.annotate(text="", xy=(x1[0], y1[0]-2.5), xytext=(x1[1],y1[1]-2.5),
                     arrowprops=dict(arrowstyle='<->', color='red'))

        x2 = np.array([0.93, 0.98])
        y2 = (1 / (1 - x2))
        plt.scatter(x2, y2, marker="o", s=50, color="black")
        plt.text(x2[0] - 0.05, y2[0] + 1.5, "c", fontsize=12)
        plt.text(x2[1] - 0.05, y2[1] + 1.5, "d", fontsize=12)
        plt.annotate(text='', xy=(x2[1]+0.03, y2[0]),
                     xytext=(x2[1]+0.03, y2[1]),
                     arrowprops=dict(arrowstyle='<->', color='red'))

    plt.xlabel("Utilization")
    plt.ylabel("Average Time in System")
    plt.show()
