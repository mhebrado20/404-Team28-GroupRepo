#include <Arduino.h>
#include "ADCSampler.h"
#include <iostream>

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


/**********************************************
 * 
 * This function assigns the microphone data to the array using i2s_read which reads data from the I2S buffer,
 * the for loop is where the values are assigned, 2048 - (uint8_t(samples[i]) & 0xfff) gives the values of the microphone
 * analog output in mV, I am unsure why the function was multiplied by 15 my best guess is it has to do with uint16_t 
 * datatype being the initial datatype fo the samples array
 * 
 * At the very end the terminating bit is added to the final bit of the array
 * 
**********************************************/
int ADCSampler::read(int16_t *samples, int count)
{
    // read from i2s
    size_t bytes_read = 0;
    i2s_read(I2S_NUM_0, samples, sizeof(int16_t) * count, &bytes_read, portMAX_DELAY);
    // std::cout << "bytes_read: " << bytes_read << std::endl;
    int samples_read = bytes_read / sizeof(int16_t);
    // std::cout << "samples read:" << samples_read << std::endl;
    for (int i = 0; i < samples_read; i++)
    {
        // std::cout << "size of samples:" << sizeof(samples) << std::endl;
        // std::cout << "element: " << (256 - (uint8_t(samples[i]) & 0xfff)) << std::endl;
        // vTaskDelay(100);
        // (uint16_t(samples[i]) & 0xfff)
        samples[i] = (2048 - (uint16_t(samples[i]) & 0xfff)) * 15;
        // Serial.println(samples[i], HEX);
        // samples[i] = (2048 - (uint16_t(samples[i]) & 0xfff) * 15);
        // std::cout << "element " << i << ": " << samples[i] << std::endl;
        // vTaskDelay(10);
    }
    // samples[samples_read * 2] = (uint16_t)0x0000; //could be here
    // std::cout << "final sample:" << samples[samples_read] << std::endl;
    // std::cout << "correct value:" << (uint8_t)0x00 << std::endl;
    // samples[sizeof(samples)-1] = 0x00;
    return samples_read;
}
