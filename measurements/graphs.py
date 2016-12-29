# -*- coding: utf-8 -*-
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import numpy as np

from numpy import genfromtxt
data = genfromtxt('measurements/discharg3.csv', delimiter=';')

dlzka = len(data)

cas = [0] * (dlzka-1)
vol = [0] * (dlzka-1)
cur = [0] * (dlzka-1)
te1 = [0] * (dlzka-1)
te2 = [0] * (dlzka-1)
cap = [0] * (dlzka-1)

i = 1
while (i<dlzka):
	cas[i-1] = data[i][0]
	vol[i-1] = data[i][1]
	cur[i-1] = data[i][2]
	te1[i-1] = data[i][3]
	te2[i-1] = data[i][4]
	cap[i-1] = cas[i-1]/3600
	i += 1
	
plt.plot(cap,vol,'b')
plt.xticks(np.arange(0,max(cap), 0.25))
plt.ylabel('Voltage (mV)')
plt.xlabel('Capacity (Ah)')
plt.grid()
plt.show()

plt.figure(2)
blue_patch = mpatches.Patch(color='blue', label='Battery')
red_patch = mpatches.Patch(color='red', label='Heatsing')
plt.legend(handles=[red_patch,blue_patch],loc=4)
plt.plot(cap,te1,'b')
plt.plot(cap,te2,'r')
plt.ylabel('Temperature ('+unichr(176)+'C)')
plt.xlabel('Capacity (Ah)')
plt.grid()
plt.show()
