# Multi-Device Data Collection with XBee (ZigBee)
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
<img src="https://github.com/rookiepeng/Multi-Device-XBee/blob/master/docs/connection.png?raw=true">

## Running

### Query Mode
LabVIEW code: LabVIEW/XBee_Query.vi
- Support 3 devices at the same time
- Sampling: 20 Hz x 12 bits x 2 channels each device
- Update every 10 s

### Continuous Mode
LabVIEW code: LabVIEW/XBee_Continuous.vi
- 1 device
- Sampling: 80 Hz x 12 bits x 2 channels
- Update every 0.0125 s

## Screenshots

### Query Mode
<img src="https://github.com/rookiepeng/Multi-Device-XBee/blob/master/docs/queryMode.png?raw=true">

### Continuous Mode
<img src="https://github.com/rookiepeng/Multi-Device-XBee/blob/master/docs/continuousMode.png?raw=true">
