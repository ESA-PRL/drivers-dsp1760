/*
 * File:   dsp1760.cpp
 * Author: Karl Kangur, ESA/ESTEC
 * Date:   2017-03-13
 */

#include <stdio.h>
#include <string.h>
#include <iostream>
#include "dsp1760.hpp"

using namespace std;
using namespace dsp1760;

// Generic driver internal buffer size
DSP1760driver::DSP1760driver():
    Driver(MAX_PACKET_SIZE)
{
    // Default data rate is 1000Hz
    datarate = DR1000;
    suppress_invalid_messages = false;
    sequence = 0xff;
}

DSP1760driver::~DSP1760driver()
{
    if(isValid())
    {
        close();
    }
}

int DSP1760driver::getFileDescriptor()
{
    // Need to define this for some reason, otherwise the component does not find it
    return Driver::getFileDescriptor();
}

bool DSP1760driver::update(float &delta)
{
     float temperature;
     return update(delta, temperature);
}

bool DSP1760driver::update(float &delta, float &temperature)
{
     uint8_t sequence;
     return update(delta, temperature, sequence);
}

bool DSP1760driver::update(float &delta, float &temperature, uint8_t &sequence_out)
{
    uint8_t buffer[MAX_PACKET_SIZE];
    
    // Do not process anything when the configuration mode is enabled
    if(config_mode == ON)
    {
        sequence = 0xff;
        sequence_out = sequence;
        return false;
    }
    
    try
    {
        // Read the packet in the local buffer. Timeout and first byte timeouts are equal: 2000ms
        // If set to 0ms it will call the timeout error all the time
        // Also if the buffer size is smaller than the driver buffer size then the following error is thrown:
        // FileDescriptorActivity: timeout in select()
        readPacket(buffer, MAX_PACKET_SIZE, 2000, 2000);
        
        // Count sequence to check for missing packets, byte 29 increments by
        // one after every new reading and loops back to 0 at 127
        packet_index = buffer[DSP1760_SEQUENCE];
        if(sequence != 0xff && ((packet_index != 0 && sequence + 1 != packet_index) || (packet_index == 0 && sequence != 127)))
        {
            cout << "DSP1760driver: Value out of sequence" << endl;
            sequence = 0xff;
            return false;
        }
        // Update the sequence number
        sequence = packet_index;
        sequence_out = sequence;
        
        bool valid = ((buffer[DSP1760_STATUS] >> 2) & 1);
        if(!valid && !suppress_invalid_messages)
        {
            // Gyro Z status is invalid
            cout << "DSP1760driver: Invalid rotational value" << endl;
            return false;
        }
        else if(valid && suppress_invalid_messages)
        {
            // This is needed because after a data rate change there are systematically 2 invalid messages
            suppress_invalid_messages = false;
        }
        
        // Z rotational data is contained in 4 bytes from byte 12 to 15, big endian format
        uint8_t rotation_z[4] = {buffer[DSP1760_ROT_Z+3], buffer[DSP1760_ROT_Z+2], buffer[DSP1760_ROT_Z+1], buffer[DSP1760_ROT_Z]};
        memcpy(&delta, &rotation_z, sizeof(rotation_z));

        uint8_t temperature_read[2] = {buffer[DSP1760_TEMP+1], buffer[DSP1760_TEMP]};
        int16_t temperature_int;
        memcpy(&temperature_int, &temperature_read, sizeof(temperature_read));
        temperature = ((float)temperature_int)*temperature_factor;

        return true;
    }
    catch(int timeout_error)
    {
        cout << "DSP1760driver: Timeout error" << endl;
        return false;
    }
}

int DSP1760driver::getIndex()
{
    return packet_index;
}

bool DSP1760driver::configurationMode(CONFMODE mode)
{
    config_mode = mode;
    
	char buffer[10];
	sprintf(buffer, "=config,%d\n", mode);
    return writePacket((uint8_t*)buffer, sizeof(buffer) / sizeof(buffer[0]), 100);
}

bool DSP1760driver::setDataRate(DATARATE rate)
{
    setDataRate((int)rate);
}

bool DSP1760driver::setDataRate(int rate)
{
    switch(rate)
    {
    case 1: datarate = DR1; break;
    case 5: datarate = DR5; break;
    case 10: datarate = DR10; break;
    case 25: datarate = DR25; break;
    case 50: datarate = DR50; break;
    case 100: datarate = DR100; break;
    case 250: datarate = DR250; break;
    case 500: datarate = DR500; break;
    case 750: datarate = DR750; break;
    case 1000: datarate = DR1000; break;
    default:
        cout << "DSP1760driver: Data rate not valid" << endl;
        return false;
    }
    
    configurationMode(ON);
    
    char buffer[16];
    sprintf(buffer, "=dr,%d\n", datarate);
    bool ret = writePacket((uint8_t*)buffer, sizeof(buffer) / sizeof(buffer[0]), 100);
    
    configurationMode(OFF);
    
    // Changing the data rate invalidates the 2 next messages, suppress the warning
    suppress_invalid_messages = true;
    
    return ret;
}

bool DSP1760driver::setTemperatureDecimal(bool isDecimal)
{
    configurationMode(ON);
    
    char buffer[32];
    if(isDecimal)
    {
        temperature_factor = 0.01;
        sprintf(buffer, "=tempunits,%s\n", "C_100");
    }
    else
    {
        temperature_factor = 1.0;
        sprintf(buffer, "=tempunits,%s\n", "C");
    }
    bool ret = writePacket((uint8_t*)buffer, sizeof(buffer) / sizeof(buffer[0]), 100);
    
    configurationMode(OFF);
    
    // Changing the data rate invalidates the 2 next messages, suppress the warning
    suppress_invalid_messages = true;
 
    return ret;
}

// Change the angular (gyro) data format
bool DSP1760driver::setAngularDataFormat(FORMAT format)
{
    configurationMode(ON);
    
    char buffer[32];
    if(format == DELTA)
    {
        // radians or degrees
        sprintf(buffer, "=rotfmt,delta\n");
    }
    else if(format == RATE)
    {
        // radians or degrees per second
        sprintf(buffer, "=rotfmt,rate\n");
    }
    else if(format == RESET)
    {
        // Resets to default (delta)
        sprintf(buffer, "=rotfmt,reset\n");
    }
    bool ret = writePacket((uint8_t*)buffer, sizeof(buffer) / sizeof(buffer[0]), 100);
    
    configurationMode(OFF);
    
    // Changing the data rate invalidates the 2 next messages, suppress the warning
    suppress_invalid_messages = true;
 
    return ret;
}

// Virtual method, must be redefined to process custom packet
int DSP1760driver::extractPacket(uint8_t const* buffer, size_t buffer_size) const
{
    // One packet must be at least 36 chars long, return if buffer is less than 36 bytes
    if(buffer_size < 36)
    {
        return 0;
    }
    
    // Every packet of 36 bytes starts with 0xFE81FF55 for the DSP1760
    if(buffer[0] == 0xFE && buffer[1] == 0x81 && buffer[2] == 0xFF && buffer[3] == 0x55)
    {
        // Return 36 bytes for the next readPacket call
        return 36;
    }
    
    // Remove one byte at the beginning of the device buffer
    return -1;
}

