/*
 * SpanDSP - a series of DSP components for telephony
 *
 * dtmf.c - DTMF generation and detection.
 *
 * Written by Steve Underwood <steveu@coppice.org>
 *
 * Copyright (C) 2001-2003, 2005, 2006 Steve Underwood
 *
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id: dtmf.c,v 1.42 2008/06/13 14:46:52 steveu Exp $
 */
 
/*! \file dtmf.h */

#include "stdafx.h"

#include "span_inttypes.h"
#include "span_telephony.h"
#include "span_queue.h"
#include "span_complex.h"
#include "span_tone_detect.h"
#include "span_super_tone_rx.h"
#include "span_dc_restore.h"
#include "span_dtmf.h"

#define DEFAULT_DTMF_TX_LEVEL       -10
#define DEFAULT_DTMF_TX_ON_TIME     50
#define DEFAULT_DTMF_TX_OFF_TIME    55


#define DTMF_THRESHOLD              10438           /* -42dBm0 */

#define DTMF_POWER_OFFSET           68.251f         /* 10*log(256.0*256.0*DTMF_SAMPLES_PER_BLOCK) */
#define DTMF_SAMPLES_PER_BLOCK      102

//#define DTMF_RELATIVE_PEAK_ROW      6.309f          /* 8dB */
const Platform::int64 CDtmfRelativePeekRow = 6.309f * 1024;

//#define DTMF_RELATIVE_PEAK_COL      6.309f          /* 8dB */
const Platform::int64 CDtmfRelativePeekCol = 6.309f * 1024;

//#define DTMF_TO_TOTAL_ENERGY        83.868f         /* -0.85dB */
const Platform::int64 CDtmfToTotalEnergy = 83.868f * 1024;

//#define DTMF_NORMAL_TWIST           6.309f          /* 8dB */
const int64_t CDtmfNormalTwist = 6.309f * 1024;

//#define DTMF_REVERSE_TWIST          2.512f          /* 4dB */
const int64_t CDtmfReverseTwist = 2.512f * 1024;




static const float dtmf_row[] =
{
     697.0f,  770.0f,  852.0f,  941.0f
};
static const float dtmf_col[] =
{
    1209.0f, 1336.0f, 1477.0f, 1633.0f
};

static const char dtmf_positions[] = "123A" "456B" "789C" "*0#D";

static goertzel_descriptor_t dtmf_detect_row[4];
static goertzel_descriptor_t dtmf_detect_col[4];


namespace 
{             
    int16_t FiltrDialtone(dtmf_rx_state_t *s, int16_t xamp)
    {
        float famp = xamp;
        float v1;

        // Sharp notches applied at 350Hz and 440Hz - the two common dialtone frequencies.
        // These are rather high Q, to achieve the required narrowness, without using lots of
        // sections. 
        v1 = 0.98356f*famp + 1.8954426f*s->z350[0] - 0.9691396f*s->z350[1];
        famp = v1 - 1.9251480f*s->z350[0] + s->z350[1];
        s->z350[1] = s->z350[0];
        s->z350[0] = v1;

        v1 = 0.98456f*famp + 1.8529543f*s->z440[0] - 0.9691396f*s->z440[1];
        famp = v1 - 1.8819938f*s->z440[0] + s->z440[1];
        s->z440[1] = s->z440[0];
        s->z440[0] = v1;
        return famp;
    }        

    int GoertzelProcess(dtmf_rx_state_t *s, const int16_t* pData, int size)
    {        
        int16_t xamp; 

        /* The block length is optimised to meet the DTMF specs. */
        int rest = DTMF_SAMPLES_PER_BLOCK - s->current_sample;
        if (size > rest) size = rest;            

        for (int i = 0; i < 4; ++i)
        {
            GoertzelUpdateOptimized(&s->row_out[i], pData, size);
            GoertzelUpdateOptimized(&s->col_out[i], pData, size);
        }      

        s->current_sample += size;
        ESS_ASSERT(s->current_sample <= DTMF_SAMPLES_PER_BLOCK);
        if (s->current_sample == DTMF_SAMPLES_PER_BLOCK) s->current_sample = 0;                
        return size;
    }

    void AnalyzeGoetzelResult(dtmf_rx_state_t *s)
    {
        int64_t row_energy[4];
        int64_t col_energy[4];

        int best_row;
        int best_col;

        uint8_t hit;

        /* We are at the end of a DTMF detection block */
        /* Find the peak row and the peak column */
        row_energy[0] = goertzel_result(&s->row_out[0]);
        best_row = 0;
        col_energy[0] = goertzel_result(&s->col_out[0]);
        best_col = 0;
        for (int i = 1;  i < 4;  i++)
        {
            row_energy[i] = goertzel_result(&s->row_out[i]);
            if (row_energy[i] > row_energy[best_row])
                best_row = i;
            col_energy[i] = goertzel_result(&s->col_out[i]);
            if (col_energy[i] > col_energy[best_col])
                best_col = i;
        }                                 

        hit = 0;
        /* Basic signal level test and the twist test */
        if (row_energy[best_row] >= s->threshold
            &&
            col_energy[best_col] >= s->threshold
            &&
            col_energy[best_col] * 1024 < row_energy[best_row]*s->reverse_twist
            &&
            col_energy[best_col]*s->normal_twist > row_energy[best_row] * 1024)
        {        	        	
            /* Relative peak test ... */
            int i;
            for (i = 0;  i < 4;  i++)
            {
                if ((i != best_col  &&  (col_energy[i]*CDtmfRelativePeekCol) > col_energy[best_col] * 1024)
                    ||
                    (i != best_row  &&  (row_energy[i]*CDtmfRelativePeekRow) > row_energy[best_row] * 1024) )
                {
                    break;
                }
            }
            /* ... and fraction of total energy test */
            if (i >= 4
                &&
                (row_energy[best_row] + col_energy[best_col]) > (CDtmfToTotalEnergy * s->energy >> 10) )
            {
                /* Got a hit */
                hit = dtmf_positions[(best_row << 2) + best_col];
            }
        }
        /* The logic in the next test should ensure the following for different successive hit patterns:
        -----ABB = start of digit B.
        ----B-BB = start of digit B
        ----A-BB = start of digit B
        BBBBBABB = still in digit B.
        BBBBBB-- = end of digit B
        BBBBBBC- = end of digit B
        BBBBACBB = B ends, then B starts again.
        BBBBBBCC = B ends, then C starts.
        BBBBBCDD = B ends, then D starts.
        This can work with:
        - Back to back differing digits. Back-to-back digits should
        not happen. The spec. says there should be a gap between digits.
        However, many real phones do not impose a gap, and rolling across
        the keypad can produce little or no gap.
        - It tolerates nasty phones that give a very wobbly start to a digit.
        - VoIP can give sample slips. The phase jumps that produces will cause
        the block it is in to give no detection. This logic will ride over a
        single missed block, and not falsely declare a second digit. If the
        hiccup happens in the wrong place on a minimum length digit, however
        we would still fail to detect that digit. Could anything be done to
        deal with that? Packet loss is clearly a no-go zone.
        Note this is only relevant to VoIP using A-law, u-law or similar.
        Low bit rate codecs scramble DTMF too much for it to be recognised,
        and often slip in units larger than a sample. */
        if (hit != s->in_digit)
        {
            if (s->last_hit != s->in_digit)
            {
                /* We have two successive indications that something has changed. */
                /* To declare digit on, the hits must agree. Otherwise we declare tone off. */
                hit = (hit  &&  hit == s->last_hit)  ?  hit   :  0;
                if (s->realtime_callback)
                {
                    /* Avoid reporting multiple no digit conditions on flaky hits */
                    ESS_UNIMPLEMENTED;

                    /*
                    if (s->in_digit  ||  hit)
                    {
                    i = (s->in_digit  &&  !hit)  ?  -99  :  rintf(log10f(s->energy)*10.0f - DTMF_POWER_OFFSET + DBM0_MAX_POWER);
                    s->realtime_callback(s->realtime_callback_data, hit, i, 0);
                    }
                    */
                }
                else
                {
                    if (hit)
                    {
                        if (s->current_digits < MAX_DTMF_DIGITS)
                        {
                            s->digits[s->current_digits++] = (char) hit;
                            s->digits[s->current_digits] = '\0';
                            /*
                            if (s->digits_callback)
                            {
                                s->digits_callback(s->digits_callback_data, s->digits, s->current_digits);
                                s->current_digits = 0;
                            }
                            */
                        }
                        else
                        {
                            s->lost_digits++;
                        }
                    }
                }
                s->in_digit = hit;
            }
        }
        s->last_hit = hit;
        s->energy = 0;
    }


    void TryCallback(dtmf_rx_state_t *s)
    {
        if (!s->current_digits  ||  !s->digits_callback) return;

        s->digits_callback(s->digits_callback_data, s->digits, s->current_digits);
        s->digits[0] = '\0';
        s->current_digits = 0;        
    }
} // namespace 


// ------------------------------------------------------------------------------------


int dtmf_rx(dtmf_rx_state_t *s, const int16_t amp[], int blockSize)
{	
    if (blockSize == 0) return 0;
    
    int processed = 0;
    while (processed < blockSize)
    {           
        processed += GoertzelProcess(s, amp + processed, blockSize - processed); // обработка не более         
        if (s->current_sample != 0) break;// накопление не закончено
        
        AnalyzeGoetzelResult(s);                
    }  

    ESS_ASSERT(processed == blockSize);
    
    TryCallback(s);                

    return 0;    
}

/*- End of function --------------------------------------------------------*/

int dtmf_rx_status(dtmf_rx_state_t *s) // что это?
{
    if (s->in_digit)
        return s->in_digit;
    if (s->last_hit)
        return 'x';
    return 0;
}
/*- End of function --------------------------------------------------------*/

size_t dtmf_rx_get(dtmf_rx_state_t *s, char *buf, int max)// что это?
{
    if (max > s->current_digits)
        max = s->current_digits;
    if (max > 0)
    {
        memcpy(buf, s->digits, max);
        memmove(s->digits, s->digits + max, s->current_digits - max);
        s->current_digits -= max;
    }
    buf[max] = '\0';
    return  max;
}
/*- End of function --------------------------------------------------------*/

void dtmf_rx_set_realtime_callback(dtmf_rx_state_t *s,
                                   tone_report_func_t callback,
                                   void *user_data)
{
    s->realtime_callback = callback;
    s->realtime_callback_data = user_data;
}
/*- End of function --------------------------------------------------------*/

void dtmf_rx_parms(dtmf_rx_state_t *s,
                   int filter_dialtone,
                   int twist,
                   int reverse_twist,
                   int threshold)
{
    float x;

    if (filter_dialtone >= 0)
    {
        s->z350[0] = 0.0f;
        s->z350[1] = 0.0f;
        s->z440[0] = 0.0f;
        s->z440[1] = 0.0f;
        s->filter_dialtone = filter_dialtone;
    }
    if (twist >= 0)
        s->normal_twist = powf(10.0f, twist/10.0f) * 1024;
    if (reverse_twist >= 0)
        s->reverse_twist = powf(10.0f, reverse_twist/10.0f) * 1024;
    if (threshold > -99)
    {        
        x = (DTMF_SAMPLES_PER_BLOCK*32768.0f/1.4142f)*powf(10.0f, (threshold - DBM0_MAX_SINE_POWER)/20.0f);
        s->threshold = x;        
    }
}
/*- End of function --------------------------------------------------------*/

dtmf_rx_state_t *dtmf_rx_init(dtmf_rx_state_t *s,
                              digits_rx_callback_t callback,
                              void *user_data)
{
    int i;
    static int initialised = FALSE;

    if (s == NULL)
    {
        if ((s = (dtmf_rx_state_t *) malloc(sizeof (*s))) == NULL)
            return  NULL;
    }
    s->digits_callback = callback;
    s->digits_callback_data = user_data;
    s->realtime_callback = NULL;
    s->realtime_callback_data = NULL;
    s->filter_dialtone = FALSE;
    s->normal_twist = CDtmfNormalTwist;// DTMF_NORMAL_TWIST;
    s->reverse_twist = CDtmfReverseTwist;//DTMF_REVERSE_TWIST;
    s->threshold = DTMF_THRESHOLD;

    s->in_digit = 0;
    s->last_hit = 0;

    if (!initialised)
    {
        for (i = 0;  i < 4;  i++)
        {
            make_goertzel_descriptor(&dtmf_detect_row[i], dtmf_row[i], DTMF_SAMPLES_PER_BLOCK);
            make_goertzel_descriptor(&dtmf_detect_col[i], dtmf_col[i], DTMF_SAMPLES_PER_BLOCK);
        }
        initialised = TRUE;
    }
    for (i = 0;  i < 4;  i++)
    {
        goertzel_init(&s->row_out[i], &dtmf_detect_row[i]);
        goertzel_init(&s->col_out[i], &dtmf_detect_col[i]);
    }

    s->energy = 0;
    s->current_sample = 0;
    s->lost_digits = 0;
    s->current_digits = 0;
    s->digits[0] = '\0';
    return s;
}
/*- End of function --------------------------------------------------------*/

int dtmf_rx_free(dtmf_rx_state_t *s)
{
    free(s);
    return 0;
}
/*- End of function --------------------------------------------------------*/


/*- End of file ------------------------------------------------------------*/
