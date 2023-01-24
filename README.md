This is the source code for a fun project, `DJ Toaster Pro`, or in short `DJTP`.

The device is based on an `Arduino Uno`. It has a single trigger (alarm) input and increases programmable DMX channel values if triggered. Two potentiometers at the front of the device control how fast the values increase and decrease. A 3-position sliding switch at the side of the device puts the output into `BYPASS/OFF`, `LINEAR`, or `EXPONENTIAL` mode, modifying the slope of the value increase.

A test push button allows simulation of the external alarm signal.

Code quality: since this is just a fun project, I hacked together the code quite quickly and I provide it `AS-IS` for anyone willing or needing to modify the operation of the device.

### How to use the code

- Copy the source code into your `Documents/Arduino` folder. The structure should be as follows:
  - `Documents/Arduino/libraries/DMXSerial`
  - `Documents/Arduino/DJTPv1`
- Use Arduino IDE to open the `.ino` file
- Connect the DJTP device via USB to your computer
- Choose `Arduino Uno` as target
- Compile and upload the project
