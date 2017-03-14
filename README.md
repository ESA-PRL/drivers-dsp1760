# DSP1760 library

Library to parse the [DSP1760 optical gyroscope](http://www.kvh.com/Military-and-Government/Gyros-and-Inertial-Systems-and-Compasses/Gyros-and-IMUs-and-INS/Fiber-Optic-Gyros/DSP-1760.aspx) packets within the ROCK framework.

Note that the library will output an error when the gyroscope is saturated.

**Author: [Karl Kangur](mailto:karl.kangur@esa.int "Contact the author"),  
Contact: [Martin Azkarate](mailto:Martin.Azkarate@esa.int "Contact the maintainer"),  
Affiliation: Automation and Robotics Laboratories, ESTEC, ESA**

## Usage

### Building

Clone the library to `drivers/dsp1760` and add it to the ROCK manifest, call `amake`.

To use it within the ROCK framework use the [DSP1760 component](https://github.com/hdpr-rover/drivers-orogen-dsp1760).

### Unit test

A test script is under `src/main.cpp`. It is built and stored under `build/dsp1760_test`. To run it call the executable with the device as argument:

    ./build/dsp1760_test /dev/ttyUSBx

