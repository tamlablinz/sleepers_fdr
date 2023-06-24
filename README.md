# sleepers_fdr
Sleepers controllers for Festival der Regionen opening concert

The Sleepers are ESP32 S3 boards transmitting to a ESP32 S2 acting as a receiver. They all automatically connect using the ESP NOW protocol. 

Every sleeper has one MPU (I2S), three piezos for analyzing knocks with a short FFT and four touch pads (capacitive). 

The receiver board gets raw sensor data from every sleeper, maps the ranges to the actual movement range of the physical object, and finally transmits all as MIDI data. Every sleeper is transmitted in a different MIDI channel (1-8). Knocks are send as MIDI notes while MPU and capacitive are CC. 
