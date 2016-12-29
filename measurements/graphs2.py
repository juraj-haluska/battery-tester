# -*- coding: utf-8 -*-
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import numpy as np

from numpy import genfromtxt
data = genfromtxt('measurements/chargeli4.csv', delimiter=';')

dlzka = len(data)

cas = [0] * (dlzka-1)
vol = [0] * (dlzka-1)
cur = [0] * (dlzka-1)
te1 = [0] * (dlzka-1)
te2 = [0] * (dlzka-1)
mins = [0] * (dlzka-1)

i = 1
while (i<dlzka):
	cas[i-1] = data[i][0]
	vol[i-1] = data[i][1]
	cur[i-1] = data[i][2]
	te1[i-1] = data[i][3]
	te2[i-1] = data[i][4]
	mins[i-1] = cas[i-1]/60
	i += 1
	
	
blue_patch = mpatches.Patch(color='blue', label='Voltage')
red_patch = mpatches.Patch(color='red', label='Current')
fig, ax1 = plt.subplots()	
ax1.plot(mins,vol,'b')
plt.ylabel('Voltage (mV)')
plt.xlabel('Time (m)')
plt.legend(handles=[red_patch,blue_patch],loc=3)
ax2 = ax1.twinx()
ax2.plot(mins,cur,'r')
plt.ylabel('Current (mA)')
plt.xticks(np.arange(0,max(mins), 25))
ax1.grid()
#plt.show()
plt.savefig("cv.png")

plt.figure(2)
blue_patch = mpatches.Patch(color='blue', label='Battery')
red_patch = mpatches.Patch(color='red', label='Heatsing')
plt.legend(handles=[red_patch,blue_patch],loc=1)
plt.plot(mins,te1,'b')
plt.plot(mins,te2,'r')
plt.ylabel('Temperature ('+unichr(176)+'C)')
plt.xlabel('Time (m)')
plt.grid()
#plt.show()
plt.savefig("tmp.png")
