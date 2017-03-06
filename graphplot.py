#!/usr/bin/python

import sys
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

data = pd.read_csv(sys.argv[1],header=None,sep=" ")

p = np.poly1d(np.polyfit(data[0],data[1],2))

plt.scatter(data[0],data[1])
plt.plot(data[0],p(data[0]),color='r')
plt.title(sys.argv[1])
plt.xlabel("Side length of window (px)")
plt.ylabel("Execution time (s)")
plt.show()
