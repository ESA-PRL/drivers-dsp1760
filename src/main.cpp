/*
 * File:   main.cpp
 * Author: Karl Kangur, ESA/ESTEC
 * Date:   2017-03-14
 */

#include <iostream>
#include "dsp1760.hpp"

int main(int argc, char** argv)
{   
	if(argc < 2)
	{
		fprintf(stderr,"Use \n./dsp1760_test <device>\n");
		return -1;
	}

    dsp1760::DSP1760driver device;
    
    if(!device.openSerial(argv[1], 921600))
    {
        std::cout << "Could not open device on port " << argv[1] << std::endl;    
    }
    
    float delta;
    while(true)
    {
		if(device.update(delta))
		{
			printf("Delta: %f, packet: %d\n", delta, device.getIndex());
		}
	}

    return 0;
}
