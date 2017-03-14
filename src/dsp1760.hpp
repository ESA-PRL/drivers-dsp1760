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
    class DSP1760driver: public iodrivers_base::Driver
    {
    public:
        DSP1760driver();
        ~DSP1760driver();
        bool update(float &delta);
        int getFileDescriptor();
        int getIndex();
        
    private:
        static const int MAX_PACKET_SIZE = 512;
        virtual int extractPacket(uint8_t const* buffer, size_t buffer_size) const;
        int packet_index;
    };
}

#endif
