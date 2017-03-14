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
    uint8_t buffer[MAX_PACKET_SIZE];
    static uint8_t sequence = 0xff;
    static uint8_t status;
    
    try
    {
        // Read the packet in the local buffer. Timeout and first byte timeouts are equal: 100ms
        // If set to 0ms it will call the timeout error all the time
        // Also if the buffer size is smaller than the driver buffer size then the following error is thrown:
        // FileDescriptorActivity: timeout in select()
        readPacket(buffer, MAX_PACKET_SIZE, 100, 100);
        
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
        
        status = buffer[DSP1760_STATUS];
        if(((status >> 2) & 1) == 0)
        {
            // Gyro Z status is invalid
            cout << "DSP1760driver: Invalid rotational value" << endl;
            return false;
        }
        
        // Z rotational data is contained in 4 bytes from byte 12 to 15, big endian format
        uint8_t rotation_z[4] = {buffer[DSP1760_ROT_Z+3], buffer[DSP1760_ROT_Z+2], buffer[DSP1760_ROT_Z+1], buffer[DSP1760_ROT_Z]};
        memcpy(&delta, &rotation_z, sizeof(rotation_z));
        
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

