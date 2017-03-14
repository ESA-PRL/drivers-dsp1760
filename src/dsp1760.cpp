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
    Driver(512)
{

}

DSP1760driver::~DSP1760driver()
{

}

int DSP1760driver::getFileDescriptor()
{
    // Need to define this for some reason, otherwise the component does not find it
    return Driver::getFileDescriptor();
}

bool DSP1760driver::update(float &delta)
{
    // Define local device buffer as big as the generic driver buffer (512 in this case)
    uint8_t buffer[Driver::MAX_PACKET_SIZE];
    static uint8_t sequence_local = 0xff;
    static uint8_t status;
    
    try
    {
        // Read the packet in the local buffer. Timeout and first byte timeouts are equal: 100ms
        // If set to 0ms it will call the timeout error all the time
        // Also if the buffer size is smaller than the driver buffer size then the following error is thrown:
        // FileDescriptorActivity: timeout in select()
        readPacket(buffer, Driver::MAX_PACKET_SIZE, 100, 100);
        
        // Count sequence to check for missing packets, byte 29 increments by
        // one after every new reading and loops back to 0 at 127
        uint8_t sequence = buffer[DSP1760_SEQUENCE];
        if(sequence_local != 0xff && ((sequence != 0 && sequence_local + 1 != sequence) || (sequence == 0 && sequence_local != 127)))
        {
            cout << "DSP1760driver: Value out of sequence" << endl;
            sequence_local = 0xff;
            return false;
        }
        // Update the sequence number
        sequence_local = sequence;
        
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

