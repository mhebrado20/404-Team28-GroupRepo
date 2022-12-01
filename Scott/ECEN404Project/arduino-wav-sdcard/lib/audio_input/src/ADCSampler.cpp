#include "ADCSampler.h"

ADCSampler::ADCSampler(adc_unit_t adcUnit, adc1_channel_t adcChannel, const i2s_config_t &i2s_config) : I2SSampler(I2S_NUM_0, i2s_config)
{
    m_adcUnit = adcUnit;
    m_adcChannel = adcChannel;
}

void ADCSampler::configureI2S()
{
    //init ADC pad
    i2s_set_adc_mode(m_adcUnit, m_adcChannel);
    // enable the adc
    i2s_adc_enable(m_i2sPort);
}

void ADCSampler::unConfigureI2S()
{
    // make sure to do this or the ADC is locked
    i2s_adc_disable(m_i2sPort);
}

int ADCSampler::read(int16_t *samples, int count)
{
    // read from i2s
    size_t bytes_read = 0;
    i2s_read(m_i2sPort, samples, sizeof(int16_t) * count, &bytes_read, portMAX_DELAY);
    int samples_read = bytes_read / sizeof(int16_t);
    int16_t *intermediate_samples = samples;

    //butterworth 5th order difference coefficients, calculated using CurioRes files
    float b[] = {0.01134013, 0.05670066, 0.11340131, 0.11340131, 0.05670066, 0.01134013};
    float a[] = {1.58974837, -1.52338634, 0.76185031, -0.21653758, 0.02544105};

    /*
    samples[0] = 0;
    samples[1] = 0;
    samples[2] = 0;
    samples[3] = 0;
    samples[4] = 0;
    samples[5] = 0;
    */
    
    for (int i = 0; i < samples_read; i++)
    {
        samples[i] = (2048 - (uint16_t(samples[i]) & 0xfff) - 200) * 20;
        // intermediate_samples[i] = (2048 - (uint16_t(samples[i]) & 0xfff) - 200) * 20;
        // samples[i] = 0; //initializing every part of the array to zero to start
    }

    /*
    for (int i = 0; i < samples_read - 6; i++)
    {
        // following CurioRes example
        samples[i] = a[0]*samples[i+1] + a[1]*samples[i+2] + a[2]*samples[i+3] + a[3]*samples[i+4] + a[4]*samples[i+5]
             + b[0]*intermediate_samples[i] + b[1]*intermediate_samples[i+1] + b[2]*intermediate_samples[i+2] 
             + b[3]*intermediate_samples[i+3] + b[4]*intermediate_samples[i+4] + b[5]*intermediate_samples[i+5];

        for(int i = 1; i >= 0; i--)
        {
            intermediate_samples[i+1] = intermediate_samples[i];
            samples[i+1] = samples[i];
        }
    }
    */
    
    
    return samples_read;
}