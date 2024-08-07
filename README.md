Embedded Systems for Pet Feeder

# Introduction
The goal of this project was to create an automated pet feeder capable of maintaining a water dish at a set level, freshening the water when the pet is nearby, and dispensing predetermined amounts of food at scheduled times. The feeder utilizes capacitive sensing to measure the water level, a PIR motion sensor to detect when the pet is present, a peristaltic pump to deliver water, a DC gear motor to run an auger to dispense food, and a microcontroller to control the components and timing.

# Hardware

<img src="petfeeder.png" alt="petfeeder" width="400"/>

# Working

## Water Level Sensing
A capacitive sensing circuit was used to determine the water level in the pet's dish. Two copper electrode strips are placed vertically inside the dish with one end immersed in the water. The strips form a capacitor where the water acts as the dielectric. A capacitor's capacitance is directly proportional to the overlapping surface area of its plates which changes as the water level changes. 
To measure capacitance, the base of an NPN transistor is momentarily pulled low to discharge the capacitor. Then the capacitor is allowed to recharge through a 1Mohm resistor connected to Vcc. A timer measures the time for the voltage to rise above the comparator threshold which is proportional to the capacitance. Lower water 