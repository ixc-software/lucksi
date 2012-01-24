#include "stdafx.h"

#include "EchoIntTypes.h"
#include "fir.h"

// ---------------------------------------------------------------

const int16_t *fir16_create(fir16_state_t *fir, const int16_t *coeffs, int taps)
{
    fir->taps = taps;
    fir->curr_pos = taps - 1;
    fir->coeffs = coeffs;

    if ((fir->history = (int16_t *) malloc(taps*sizeof(int16_t))))
        memset(fir->history, 0, taps*sizeof(int16_t));

    return fir->history;
}

// ---------------------------------------------------------------

void fir16_flush(fir16_state_t *fir)
{
    memset(fir->history, 0, fir->taps*sizeof(int16_t));
}

// ---------------------------------------------------------------

void fir16_free(fir16_state_t *fir)
{
    free(fir->history);
}

// ---------------------------------------------------------------

int16_t fir16(fir16_state_t *fir, int16_t sample)
{
    int32_t y;
    int i;
    int offset1;
    int offset2;

    fir->history[fir->curr_pos] = sample;

    offset2 = fir->curr_pos;
    offset1 = fir->taps - offset2;
    y = 0;
    for (i = fir->taps - 1;  i >= offset1;  i--)
        y += fir->coeffs[i] * fir->history[i - offset1];
    for (  ;  i >= 0;  i--)
        y += fir->coeffs[i] * fir->history[i + offset2];

    if (fir->curr_pos <= 0)
        fir->curr_pos = fir->taps;
    fir->curr_pos--;

    return (int16_t) (y >> 15);
}

