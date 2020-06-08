#include "modem_iq.h"
#include <math.h>

ModemIQ::ModemIQ(long frequency, long bandwidth, std::string outputFile)
{
    frequency_m = frequency;
    bandwidth_m = bandwidth;
    outIQFile = std::ofstream(outputFile, std::ios::binary);
}

void ModemIQ::stop()
{
    outIQFile.close();
}

void ModemIQ::process(liquid_float_complex *buffer, unsigned int &length)
{
    for (i = 0; i < length; i++)
    {
        imag = (int16_t)buffer[i].imag;
        real = (int16_t)buffer[i].real;
        outIQFile.write((char *)&imag, 2);
        outIQFile.write((char *)&real, 2);
    }
}