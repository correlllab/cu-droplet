// SB_adc.h

#ifndef __SB_ADC_H__
#define __SB_ADC_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define SAMPLES_PER_FFT 256

int8_t ADC_samples[SAMPLES_PER_FFT];
uint8_t uADC_samples[SAMPLES_PER_FFT];

void init_VIN_SEN_adc(void);
void init_MIC_AMP_adc(void);

int get_VIN_SEN_result(void);
float get_VIN_SEN_value(void);

#ifdef __cplusplus
}
#endif

#endif
