/*
 * File:   dsp1760.hpp
 * Author: Karl Kangur, ESA/ESTEC
 * Date:   2017-03-13
 */

#ifndef _DSP1760DRIVER_HPP_
#define _DSP1760DRIVER_HPP_

#include <iodrivers_base/Driver.hpp>

// Start bytes of message parts
#define DSP1760_HEADER 0
#define DSP1760_MESSAGE 4
#define DSP1760_CRC 32
#define DSP1760_ROT_X 4
#define DSP1760_ROT_Y 8
#define DSP1760_ROT_Z 12
#define DSP1760_STATUS 28
#define DSP1760_SEQUENCE 29
#define DSP1760_TEMP 30

namespace dsp1760
{
    enum CONFMODE
    {
        OFF = 0,
        ON = 1
    };
    
    enum DATARATE
    {
        DR1 = 1,
        DR5 = 5,
        DR10 = 10,
        DR25 = 25,
        DR50 = 50,
        DR100 = 100,
        DR250 = 250,
        DR500 = 500,
        DR750 = 750,
        DR1000 = 1000
    };
    
    class DSP1760driver: public iodrivers_base::Driver
    {
    public:
        DSP1760driver();
        ~DSP1760driver();
        bool update(float &delta);
        bool update(float &delta, float &temperature);
        int getFileDescriptor();
        int getIndex();
        bool setDataRate(int rate);
        bool setDataRate(DATARATE rate);
        bool setTemperatureDecimal(bool isDecimal);
        
    private:
        static const int MAX_PACKET_SIZE = 512;
        virtual int extractPacket(uint8_t const* buffer, size_t buffer_size) const;
        bool suppress_invalid_messages;
        int packet_index;
        bool configurationMode(CONFMODE mode);
        DATARATE datarate;
        CONFMODE config_mode;
	float temperature_factor;
    };
}

#endif
