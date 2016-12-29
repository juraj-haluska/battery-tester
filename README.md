# Device for charging batteries and measuring their parameters. #

Primary usage of this device is measurement parameters (internal resistance and capacity) of rechargeable 
batteries based on lithium and nickel. Measurement of those parameters is accomplished by discharging 
batteries and monitoring currents and voltages. This device also supports charging batteries.
On device there are four independent channels and each channel has two slots - one for 18650 lithium batteries and one for classic AA batteries.
There are two ways how to operate device - via terminal (USB/UART) or using LCD display and mechanical switches. By using terminal you can set up
calibration constants, charging and discharging parameters (currents, voltages, cut off temperatures, etc.) and you can get sample data
for drawing graphs. Device supports charging/discharging currents up to 5A.

This repo contains source code in [source](https://github.com/spacive/battery-tester/tree/master/source) directory. There are also project files from Atollic TrueSTUDIO. In [schematics](https://github.com/spacive/battery-tester/tree/master/schematics) directory you
can find images of schematics and eagle project files where is also a PCB layout. In [measurements](https://github.com/spacive/battery-tester/tree/master/measurements) directory there are logs from device and scripts for parsing logs and drawing graphs.

I developed this device as Bachelor's thesis for University of Zilina. It was my first touch with ARM Cortex-M0 based microcontroller so code may not be perfect.

## Photo ##
![](https://github.com/spacive/Battery-Charger/blob/master/photo.jpg "Title")
