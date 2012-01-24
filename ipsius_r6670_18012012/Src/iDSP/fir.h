/*
 * SpanDSP - a series of DSP components for telephony
 *
 * fir.h - General telephony FIR routines
 *
 */

#ifndef _FIR_H_
#define _FIR_H_

#include "EchoIntTypes.h"

struct fir16_state_t
{
    int taps;
    int curr_pos;
    const int16_t *coeffs;
    int16_t *history;
};

// ---------------------------------------------------------

const int16_t *fir16_create(fir16_state_t *fir, const int16_t *coeffs, int taps);
void fir16_flush(fir16_state_t *fir);
void fir16_free(fir16_state_t *fir);
int16_t fir16(fir16_state_t *fir, int16_t sample);


#endif


