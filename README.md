Embedded Systems for Pet Feeder

# Introduction
The goal of this project was to create an automated pet feeder capable of maintaining a water dish at a set level, freshening the water when the pet is nearby, and dispensing predetermined amounts of food at scheduled times. The feeder utilizes capacitive sensing to measure the water level, a PIR motion sensor to detect when the pet is present, a peristaltic pump to deliver water, a DC gear motor to run an auger to dispense food, and a microcontroller to control the components and timing.

# Hardware

<img src="petfeeder.png" alt="petfeeder" width="400"/>

# Working

## Water Level Sensing
A capacitive sensing circuit was used to determine the water level in the pet's dish. Two copper electrode strips are placed vertically inside the dish with one end immersed in the water. The strips form a capacitor where the water acts as the dielectric. A capacitor's capacitance is directly proportional to the overlapping surface area of its plates which changes as the water level changes. 
To measure capacitance, the base of an NPN transistor is momentarily pulled low to discharge the capacitor. Then the capacitor is allowed to recharge through a 1Mohm resistor connected to Vcc. A timer measures the time for the voltage to rise above the comparator threshold which is proportional to the capacitance. Lower water 

## Water Delivery
A peristaltic pump pulls water from a bottle through silicon tubing and delivers it through a nozzle into the pet's dish. The pump speed is controlled by a MOSFET connected to a PWM output from the microcontroller. This allows variable water flow rates. Diodes are connected across the pump terminals for flyback protection.

## Food Dispensing
Food is stored in a bottle above an auger. A DC gear motor turns the auger to push food out of the bottom of the bottle into the feeder chute. The motor connects to a power MOSFET similar to the pump for speed control. 

## Motion Sensing and Control
A passive infrared sensor detects motion from the pet to determine when it is present near the feeder. This triggers a freshening of the water in the dish by running the pump for a short time.
The TM4C123 microcontroller handles reading the sensors, controlling the outputs, maintaining time, storing settings, and scheduling feedings.
The feeder device was constructed per the lab guides using a whitewood base, 3D printed components, two 16oz plastic bottles for water and food storage, and electronic components including the capacitive sensor, pump, motors, and TM4C123 LaunchPad. The software was developed incrementally per the lab requirements. A real-time OS was utilized to handle the variety of time-sensitive operations. The water sensing was calibrated by measuring water levels versus sensor readings. Feeding times and durations were adjustable through serial commands for testing. 

