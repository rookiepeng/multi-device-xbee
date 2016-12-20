# Multi_Radar_XBee
Use MSP430 to sample analog signals (from motion radars in my application), and transfer data to a computer through XBee (ZigBee) modules.

## Environments
### Hardware
- MSP430 (M430F261X)
- XBee module

### Software
- IAR Embedded Workbench for MSP430
- National Instruments (NI) LabVIEW 2015 or newer

## Configuration
### Connections
![Connection](https://github.com/rookiepeng/Multi_Radar_XBee/blob/master/Figures/connection.png?raw=true)
## Running
### Query Mode
- Support 3 devices at the same time
- Sampling rate: 20 Hz x 12 bits x 2 channels each device
- Update every 10 s

### Continuous Mode
- 1 device
- Sampling rate: 80 Hz x 12 bits x 2 channels
- Update every 0.0125 s

## Screenshots
### Query Mode
![Query mode](https://github.com/rookiepeng/Multi_Radar_XBee/blob/master/Figures/query.PNG?raw=true)
### Continuous Mode
![Continuous mode](https://github.com/rookiepeng/Multi_Radar_XBee/blob/master/Figures/continuous.PNG?raw=true)
