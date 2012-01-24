/*
 * SpanDSP - a series of DSP components for telephony
 *
 * super_tone_rx.c - Flexible telephony supervisory tone detection.
 *
 * Written by Steve Underwood <steveu@coppice.org>
 *
 * Copyright (C) 2003 Steve Underwood
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
 * $Id: super_tone_rx.c,v 1.27 2008/06/13 14:46:52 steveu Exp $
 */

/*! \file */



#include "stdafx.h"

#include "span_telephony.h"
#include "span_complex.h"
#include "span_tone_detect.h"
#include "span_super_tone_rx.h"



#define DETECTION_THRESHOLD         16439           /* -42dBm0 */
#define TONE_TWIST                  4               /* 6dB */
#define TONE_TO_TOTAL_ENERGY        64              /* -3dB */


static int add_super_tone_freq(super_tone_rx_descriptor_t *desc, int freq)
{
    int i;

    if (freq == 0)
        return -1;
    /* Look for an existing frequency */
    for (i = 0;  i < desc->used_frequencies;  i++)
    {
        if (desc->pitches[i][0] == freq)
            return desc->pitches[i][1];
    }
    /* Look for an existing tone which is very close. We may need to merge
       the detectors. */
    for (i = 0;  i < desc->used_frequencies;  i++)
    {
        if ((desc->pitches[i][0] - 10) <= freq  &&  freq <= (desc->pitches[i][0] + 10))
        {
            /* Merge these two */
            desc->pitches[desc->used_frequencies][0] = freq;
            desc->pitches[desc->used_frequencies][1] = i;
            make_goertzel_descriptor(&desc->desc[desc->pitches[i][1]], (float) (freq + desc->pitches[i][0])/2, BINS);
            desc->used_frequencies++;
            return desc->pitches[i][1];
        }
    }
    desc->pitches[i][0] = freq;
    desc->pitches[i][1] = desc->monitored_frequencies;
    if (desc->monitored_frequencies%5 == 0)
    {
        desc->desc = (goertzel_descriptor_t *) realloc(desc->desc, (desc->monitored_frequencies + 5)*sizeof(goertzel_descriptor_t));
    }
    make_goertzel_descriptor(&desc->desc[desc->monitored_frequencies++], (float) freq, BINS);
    desc->used_frequencies++;
    return desc->pitches[i][1];
}
/*- End of function --------------------------------------------------------*/

int super_tone_rx_add_tone(super_tone_rx_descriptor_t *desc)
{
    if (desc->tones%5 == 0)
    {
        desc->tone_list = (super_tone_rx_segment_t **) realloc(desc->tone_list, (desc->tones + 5)*sizeof(super_tone_rx_segment_t *));
        desc->tone_segs = (int *) realloc(desc->tone_segs, (desc->tones + 5)*sizeof(int));
    }
    desc->tone_list[desc->tones] = NULL;
    desc->tone_segs[desc->tones] = 0;
    desc->tones++;
    return desc->tones - 1;
}
/*- End of function --------------------------------------------------------*/

int super_tone_rx_add_element(super_tone_rx_descriptor_t *desc,
                              int tone,
                              int f1,
                              int f2,
                              int min,
                              int max)
{
    int step;

    step = desc->tone_segs[tone];
    if (step%5 == 0)
    {
        desc->tone_list[tone] = (super_tone_rx_segment_t *) realloc(desc->tone_list[tone], (step + 5)*sizeof(super_tone_rx_segment_t));
    }
    desc->tone_list[tone][step].f1 = add_super_tone_freq(desc, f1);
    desc->tone_list[tone][step].f2 = add_super_tone_freq(desc, f2);
    desc->tone_list[tone][step].min_duration = min*8;
    desc->tone_list[tone][step].max_duration = (max == 0)  ?  0x7FFFFFFF  :  max*8;
    desc->tone_segs[tone]++;
    return step;
}
/*- End of function --------------------------------------------------------*/

static int test_cadence(super_tone_rx_segment_t *pattern,
                        int steps,
                        super_tone_rx_segment_t *test,
                        int rotation)
{
    int i;
    int j;

    if (rotation >= 0)
    {
        /* Check only for the sustaining of a tone in progress. This means
           we only need to check each block if the latest step is compatible
           with the tone template. */
        if (steps < 0)
        {
            /* A -ve value for steps indicates we just changed step, and need to
               check the last one ended within spec. If we don't do this
               extra test a low duration segment might be accepted as OK. */
            steps = -steps;
            j = (rotation + steps - 2)%steps;
            if (pattern[j].f1 != test[8].f1  ||  pattern[j].f2 != test[8].f2)
                return  0;
            if (pattern[j].min_duration > test[8].min_duration*BINS
                ||
                pattern[j].max_duration < test[8].min_duration*BINS)
            {
                return  0;
            }
        }
        j = (rotation + steps - 1)%steps;
        if (pattern[j].f1 != test[9].f1  ||  pattern[j].f2 != test[9].f2)
            return  0;
        if (pattern[j].max_duration < test[9].min_duration*BINS)
            return  0;
    }
    else
    {
        /* Look for a complete template match. */
        for (i = 0;  i < steps;  i++)
        {
            j = i + 10 - steps;
            if (pattern[i].f1 != test[j].f1  ||  pattern[i].f2 != test[j].f2)
                return  0;
            if (pattern[i].min_duration > test[j].min_duration*BINS
                ||
                pattern[i].max_duration < test[j].min_duration*BINS)
            {
                return  0;
            }
        }
    }
    return  1;
}
/*- End of function --------------------------------------------------------*/

super_tone_rx_descriptor_t *super_tone_rx_make_descriptor(super_tone_rx_descriptor_t *desc)
{
    if (desc == NULL)
    {
        if ((desc = (super_tone_rx_descriptor_t *) malloc(sizeof(*desc))) == NULL)
            return NULL;
    }
    desc->tone_list = NULL;
    desc->tone_segs = NULL;

    desc->used_frequencies = 0;
    desc->monitored_frequencies = 0;
    desc->desc = NULL;
    desc->tones = 0;
    return desc;
}
/*- End of function --------------------------------------------------------*/

int super_tone_rx_free_descriptor(super_tone_rx_descriptor_t *desc)
{
    if (desc)
        free(desc);
    return 0;
}
/*- End of function --------------------------------------------------------*/

void super_tone_rx_segment_callback(super_tone_rx_state_t *s,
                                    void (*callback)(void *data, int f1, int f2, int duration))
{
    s->segment_callback = callback;
}
/*- End of function --------------------------------------------------------*/

super_tone_rx_state_t *super_tone_rx_init(super_tone_rx_state_t *s,
                                          super_tone_rx_descriptor_t *desc,
                                          tone_report_func_t callback,
                                          void *user_data)
{
    int i;

    if (desc == NULL)
        return NULL;
    if (callback == NULL)
        return NULL;
    if (s == NULL)
    {
        if ((s = (super_tone_rx_state_t *) malloc(sizeof(*s) + desc->monitored_frequencies*sizeof(goertzel_state_t))) == NULL)
            return NULL;
    }

    for (i = 0;  i < 11;  i++)
    {
        s->segments[i].f1 = -1;
        s->segments[i].f2 = -1;
        s->segments[i].min_duration = 0;
    }
    s->segment_callback = NULL;
    s->tone_callback = callback;
    s->callback_data = user_data;
    s->desc = desc;
    s->detected_tone = -1;
    s->energy = 0.0f;
    for (i = 0;  i < desc->monitored_frequencies;  i++)
        goertzel_init(&s->state[i], &s->desc->desc[i]);
    return  s;
}
/*- End of function --------------------------------------------------------*/

int super_tone_rx_free(super_tone_rx_state_t *s)
{
    if (s)
        free(s);
    return 0;
}
/*- End of function --------------------------------------------------------*/

static void super_tone_chunk(super_tone_rx_state_t *s)
{
    int i;
    int j;
    int k1;
    int k2;
    int32_t res[BINS/2];

    for (i = 0;  i < s->desc->monitored_frequencies;  i++)
        res[i] = goertzel_result(&s->state[i]);
    /* Find our two best monitored frequencies, which also have adequate energy. */
    if (s->energy < DETECTION_THRESHOLD)
    {
        k1 = -1;
        k2 = -1;
    }
    else
    {
        if (res[0] > res[1])
        {
            k1 = 0;
            k2 = 1;
        }
        else
        {
            k1 = 1;
            k2 = 0;
        }
        for (j = 2;  j < s->desc->monitored_frequencies;  j++)
        {
            if (res[j] >= res[k1])
            {
                k2 = k1;
                k1 = j;
            }
            else if (res[j] >= res[k2])
            {
                k2 = j;
            }
        }
        if ((res[k1] + res[k2]) < TONE_TO_TOTAL_ENERGY*s->energy)
        {
            k1 = -1;
            k2 = -1;
        }
        else if (res[k1] > TONE_TWIST*res[k2])
        {
            k2 = -1;
        }
        else if (k2 < k1)
        {
            j = k1;
            k1 = k2;
            k2 = j;
        }
    }
    /* See if this differs from last time. */
    if (k1 != s->segments[10].f1  ||  k2 != s->segments[10].f2)
    {
        /* It is different, but this might just be a transitional quirk, or
           a one shot hiccup (eg due to noise). Only if this same thing is
           seen a second time should we change state. */
        s->segments[10].f1 = k1;
        s->segments[10].f2 = k2;
        /* While things are hopping around, consider this a continuance of the
           previous state. */
        s->segments[9].min_duration++;
    }
    else
    {
        if (k1 != s->segments[9].f1  ||  k2 != s->segments[9].f2)
        {
            if (s->detected_tone >= 0)
            {
                /* Test for the continuance of the existing tone pattern, based on our new knowledge of an
                   entire segment length. */
                if (!test_cadence(s->desc->tone_list[s->detected_tone], -s->desc->tone_segs[s->detected_tone], s->segments, s->rotation++))
                {
                    s->detected_tone = -1;
                    s->tone_callback(s->callback_data, s->detected_tone, -10, 0);
                }
            }
            if (s->segment_callback)
            {
                s->segment_callback(s->callback_data,
                                    s->segments[9].f1,
                                    s->segments[9].f2,
                                    s->segments[9].min_duration*BINS/8);
            }
            memcpy (&s->segments[0], &s->segments[1], 9*sizeof(s->segments[0]));
            s->segments[9].f1 = k1;
            s->segments[9].f2 = k2;
            s->segments[9].min_duration = 1;
        }
        else
        {
            /* This is a continuance of the previous state */
            if (s->detected_tone >= 0)
            {
                /* Test for the continuance of the existing tone pattern. We must do this here, so we can sense the
                   discontinuance of the tone on an excessively long segment. */
                if (!test_cadence(s->desc->tone_list[s->detected_tone], s->desc->tone_segs[s->detected_tone], s->segments, s->rotation))
                {
                    s->detected_tone = -1;
                    s->tone_callback(s->callback_data, s->detected_tone, -10, 0);
                }
            }
            s->segments[9].min_duration++;
        }
    }
    if (s->detected_tone < 0)
    {
        /* Test for the start of any of the monitored tone patterns */
        for (j = 0;  j < s->desc->tones;  j++)
        {
            if (test_cadence(s->desc->tone_list[j], s->desc->tone_segs[j], s->segments, -1))
            {
                s->detected_tone = j;
                s->rotation = 0;
                s->tone_callback(s->callback_data, s->detected_tone, -10, 0);
                break;
            }
        }
    }
    s->energy = 0;
}
/*- End of function --------------------------------------------------------*/

int super_tone_rx(super_tone_rx_state_t *s, const int16_t amp[], int samples)
{
    int i;
    int x;
    int sample;
    int16_t xamp;

    for (sample = 0;  sample < samples;  sample += x)
    {
        for (i = 0;  i < s->desc->monitored_frequencies;  i++)
            x = goertzel_update(&s->state[i], amp + sample, samples - sample);
        for (i = 0;  i < x;  i++)
        {
            xamp = goertzel_preadjust_amp(amp[sample + i]);
            s->energy += ((int32_t) xamp*xamp);
        }
        if (s->state[0].current_sample >= BINS)
        {
            /* We have finished a Goertzel block. */
            super_tone_chunk(s);
            s->energy = 0;
        }
    }
    return samples;
}
/*- End of function --------------------------------------------------------*/
/*- End of file ------------------------------------------------------------*/
