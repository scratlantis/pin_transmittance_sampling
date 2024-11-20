import pandas as pd
import json
import matplotlib.pyplot as plt
import numpy as np
import math

from matplotlib.ticker import MaxNLocator
from numpy import genfromtxt




with open('../results/data/default/default_info.json') as json_data:
    d = json.load(json_data)
    json_data.close()
#csvLeft = pd.read_csv('../results/data/default/default_left.csv', dtype=np.float32)
#csvRight = pd.read_csv('../results/data/default/default_right.csv', dtype=np.float32)

print(d["left_avg_sample_time"])

leftPoints = genfromtxt('../results/data/default/default_left.csv', delimiter=';')
rightPoints = genfromtxt('../results/data/default/default_right.csv', delimiter=';')
print(leftPoints)


leftAvgTime = d["left_avg_sample_time"]
rightAvgTime = d["right_avg_sample_time"]
leftSampleCount = d['left_sample_count']
rightSampleCount = d['right_sample_count']



time = d["left_avg_sample_time"] * d['left_sample_count']
xValLeft = np.arange(leftAvgTime * 9, leftAvgTime * leftSampleCount, leftAvgTime)
xValRight = np.arange(rightAvgTime * 9, rightAvgTime * rightSampleCount, rightAvgTime)
#print("numpy2")
#print(xValLeft)

fig, ax = plt.subplots()
ax.plot(xValLeft, leftPoints)
ax.plot(xValRight, rightPoints)
#ax.plot(leftPoints)
#ax.plot(rightPoints)
ax.xaxis.set_major_locator(MaxNLocator(nbins=5))
ax.yaxis.set_major_locator(MaxNLocator(nbins=5))

#ax.set_yscale('linear')
#plt.gca().invert_yaxis()

#plt.xticks([])
#plt.yticks([np.arange(0, 0.0001, step=0.000001)])
#plt.plot(xValLeft, leftPoints)
#plt.plot(xValRight, rightPoints)
plt.show()

#print(csvRight.to_string())