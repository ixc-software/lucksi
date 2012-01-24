
#include "stdafx.h"

#include "echo.h"

#define MIN_TX_POWER_FOR_ADAPTION   64
#define MIN_RX_POWER_FOR_ADAPTION   64
#define DTD_HANGOVER               600     /* 600 samples, or 75ms     */
#define DC_LOG2BETA                  3     /* log2() of DC filter Beta */

// -----------------------------------------------------------------------

//  adapting coeffs using the traditional stochastic descent (N)LMS algorithm 



static int adapt_loop(int i, int endLoop, int16_t *pHist, int16_t *pCoef, int factor)
{
    while(i >= endLoop)
    {
        int exp = (*pHist) * factor;      // 16 x 16 give +9%
        exp = ((exp + (1 << 14)) >> 15);
        (*pCoef) += (int16_t)exp;
                
        i--;
        pHist--;
        pCoef--;
    }

    return i;

} 

static void lms_adapt_bg(echo_can_state_t *ec, int clean, int shift)
{    
    ec->BgAdoptCalled++;

    int factor;
    if (shift > 0) factor = clean << shift;     // возможен сдвиг от  -8...15 при максимальном сдвиге разрядность фактора может быть 13+15=28
              else factor = clean >> -shift;

    // debug callback
#ifdef __cplusplus
    if (ec->EchoDebug != 0) ec->EchoDebug->OnAdoptEnter(factor);
#endif

    /* Update the FIR taps */

    int offset2 = ec->curr_pos;
    int offset1 = ec->taps - offset2;

    
    int i = (ec->taps - 1);
    i = adapt_loop(i, offset1, &ec->fir_state_bg.history[i - offset1], &ec->fir_taps16[1][i], factor);
        adapt_loop(i, 0,       &ec->fir_state_bg.history[i + offset2], &ec->fir_taps16[1][i], factor);
    

    // debug callback
#ifdef __cplusplus
    if (ec->EchoDebug != 0) ec->EchoDebug->OnAdoptLeave();
#endif

}

// -------------------------------------

static int top_bit(unsigned int bits)
{
    int i;

    if (bits == 0)
        return -1;
    i = 0;
    if (bits & 0xFFFF0000)
    {
        bits &= 0xFFFF0000;
        i += 16;
    }
    if (bits & 0xFF00FF00)
    {
        bits &= 0xFF00FF00;
        i += 8;
    }
    if (bits & 0xF0F0F0F0)
    {
        bits &= 0xF0F0F0F0;
        i += 4;
    }
    if (bits & 0xCCCCCCCC)
    {
        bits &= 0xCCCCCCCC;
        i += 2;
    }
    if (bits & 0xAAAAAAAA)
    {
        bits &= 0xAAAAAAAA;
        i += 1;
    }
    return i;
}


// -------------------------------------

echo_can_state_t *echo_can_create(int len, int adaption_mode)
{
    echo_can_state_t *ec;
    int i;
    int j;

    ec = (echo_can_state_t *) malloc(sizeof(*ec));
    if (ec == NULL) return  NULL;
    memset(ec, 0, sizeof(*ec));

    ec->taps = len;
    ec->log2taps = top_bit(len);
    ec->curr_pos = ec->taps - 1;

    for (i = 0;  i < 2;  i++)
    {
        int size = (ec->taps) * sizeof(int16_t);
        ec->fir_taps16[i] = (int16_t *) malloc(size);
        memset(ec->fir_taps16[i], 0, size);
    }

    fir16_create(&ec->fir_state,
                 ec->fir_taps16[0],
                 ec->taps);
    fir16_create(&ec->fir_state_bg,
                 ec->fir_taps16[1],
                 ec->taps);

    for (i=0; i<5; i++)
    {
        ec->xvtx[i] = ec->yvtx[i] = ec->xvrx[i] = ec->yvrx[i] = 0;
    }

    ec->cng_level = 1000;
    echo_can_adaption_mode(ec, adaption_mode);

    ec->snapshot = (int16_t*)malloc(ec->taps*sizeof(int16_t));
    memset(ec->snapshot, 0, sizeof(int16_t)*ec->taps);

    ec->cond_met = 0;
    ec->Pstates = 0;
    ec->Ltxacc = ec->Lrxacc = ec->Lcleanacc = ec->Lclean_bgacc = 0;
    ec->Ltx = ec->Lrx = ec->Lclean = ec->Lclean_bg = 0;
    ec->tx_1 = ec->tx_2 = ec->rx_1 = ec->rx_2 = 0;
    ec->Lbgn = ec->Lbgn_acc = 0;
    ec->Lbgn_upper = 200;
    ec->Lbgn_upper_acc = ec->Lbgn_upper << 13;

    return  ec;
}
// -------------------------------------

void echo_can_free(echo_can_state_t *ec)
{
    int i;

    fir16_free(&ec->fir_state);
    fir16_free(&ec->fir_state_bg);
    for (i = 0;  i < 2;  i++)
        free(ec->fir_taps16[i]);
    free(ec->snapshot);
    free(ec);
}
// -------------------------------------

void echo_can_adaption_mode(echo_can_state_t *ec, int adaption_mode)
{
    ec->adaption_mode = adaption_mode;
}
// -------------------------------------

void echo_can_flush(echo_can_state_t *ec)
{
    int i;

    ec->Ltxacc = ec->Lrxacc = ec->Lcleanacc = ec->Lclean_bgacc = 0;
    ec->Ltx = ec->Lrx = ec->Lclean = ec->Lclean_bg = 0;
    ec->tx_1 = ec->tx_2 = ec->rx_1 = ec->rx_2 = 0;

    ec->Lbgn = ec->Lbgn_acc = 0;
    ec->Lbgn_upper = 200;
    ec->Lbgn_upper_acc = ec->Lbgn_upper << 13;

    ec->nonupdate_dwell = 0;

    fir16_flush(&ec->fir_state);
    fir16_flush(&ec->fir_state_bg);
    ec->fir_state.curr_pos = ec->taps - 1;
    ec->fir_state_bg.curr_pos = ec->taps - 1;
    for (i = 0;  i < 2;  i++)
        memset(ec->fir_taps16[i], 0, ec->taps*sizeof(int16_t));

    ec->curr_pos = ec->taps - 1;
    ec->Pstates = 0;
}
// -------------------------------------

void echo_can_snapshot(echo_can_state_t *ec)
{
    memcpy(ec->snapshot, ec->fir_taps16[0], ec->taps*sizeof(int16_t));
}
// -------------------------------------

/* Dual Path Echo Canceller ------------------------------------------------*/

int16_t echo_can_update(echo_can_state_t *ec, int16_t tx, int16_t rx)
{
    int32_t echo_value;
    int clean_bg;
    int tmp, tmp1;

    /* Input scaling was found be required to prevent problems when tx
       starts clipping.  Another possible way to handle this would be the
       filter coefficent scaling. */

    ec->tx = tx; ec->rx = rx;
    tx >>=1;
    rx >>=1;

    /* 
       Filter DC, 3dB point is 160Hz (I think), note 32 bit precision required
       otherwise values do not track down to 0. Zero at DC, Pole at (1-Beta)
       only real axis.  Some chip sets (like Si labs) don't need
       this, but something like a $10 X100P card does.  Any DC really slows
       down convergence.

       Note: removes some low frequency from the signal, this reduces
       the speech quality when listening to samples through headphones
       but may not be obvious through a telephone handset.
                                                                    
       Note that the 3dB frequency in radians is approx Beta, e.g. for
       Beta = 2^(-3) = 0.125, 3dB freq is 0.125 rads = 159Hz.
    */

    if (ec->adaption_mode & ECHO_CAN_USE_RX_HPF)
    {
        tmp = rx << 15;

        /* Make sure the gain of the HPF is 1.0. This can still saturate a little under
           impulse conditions, and it might roll to 32768 and need clipping on sustained peak
           level signals. However, the scale of such clipping is small, and the error due to
           any saturation should not markedly affect the downstream processing. */
        tmp -= (tmp >> 4);

        ec->rx_1 += -(ec->rx_1>>DC_LOG2BETA) + tmp - ec->rx_2;

        /* hard limit filter to prevent clipping.  Note that at this stage
       rx should be limited to +/- 16383 due to right shift above */
        tmp1 = ec->rx_1 >> 15;
        if (tmp1 > 16383) tmp1 = 16383;
        if (tmp1 < -16383) tmp1 = -16383;
        rx = tmp1;
        ec->rx_2 = tmp;
    }

    /* Block average of power in the filter states.  Used for
       adaption power calculation. */

    {
        int newVal, old;

        /* efficient "out with the old and in with the new" algorithm so
           we don't have to recalculate over the whole block of
           samples. */
        newVal = (int)tx * (int)tx; //квадрат передачи в линию новый
        old = (int)ec->fir_state.history[ec->fir_state.curr_pos] *  
              (int)ec->fir_state.history[ec->fir_state.curr_pos];	//квадрат передачи в линию в предыдущем семпле 
        ec->Pstates += ((newVal - old) + (1<<ec->log2taps)) >> ec->log2taps; // средняя мощность передачи в линию с округлением 
        if (ec->Pstates < 0) ec->Pstates = 0;		//онграничение ниже 0  возможные значения  0..2^26
    }

    /* Calculate short term average levels using simple single pole IIRs */

    ec->Ltxacc += abs(tx) - ec->Ltx;           //аккумулирующее среднее значение уровня передачи (в сторону гибридной цепи)
    ec->Ltx = (ec->Ltxacc + (1<<4)) >> 5;      //деление на 32 с округлением  
    ec->Lrxacc += abs(rx) - ec->Lrx;		   // акк. среднее значение уровня приема (от гибридной схемы)
    ec->Lrx = (ec->Lrxacc + (1<<4)) >> 5;		//тоже /32 c округл

    /* Foreground filter ---------------------------------------------------*/

    ec->fir_state.coeffs = ec->fir_taps16[0];
    echo_value = fir16(&ec->fir_state, tx);
    ec->clean = rx - echo_value;           // очищенное от эха значение выход компенсатора
    ec->Lcleanacc += abs(ec->clean) - ec->Lclean; // акк. усредненного значения модуля очищенного сигнала 
    ec->Lclean = (ec->Lcleanacc + (1<<4)) >> 5;//  деленное на 32 с округлением

    /* Background filter ---------------------------------------------------*/

    echo_value = fir16(&ec->fir_state_bg, tx);
    clean_bg = rx - echo_value;
    ec->Lclean_bgacc += abs(clean_bg) - ec->Lclean_bg;
    ec->Lclean_bg = (ec->Lclean_bgacc + (1<<4)) >> 5;

    /* Background Filter adaption -----------------------------------------*/

    /* Almost always adap bg filter, just simple DT and energy
       detection to minimise adaption in cases of strong double talk.
       However this is not critical for the dual path algorithm.
    */
    ec->factor = 0;
    ec->shift = 0;
    if ((ec->nonupdate_dwell == 0))
    {
        int   P, logP, shift;

        /* Determine:
    
           f = Beta * clean_bg_rx/P ------ (1)
    
           where P is the total power in the filter states.
           
           The Boffins have shown that if we obey (1) we converge
           quickly and avoid instability.  
           
           The correct factor f must be in Q30, as this is the fixed
           point format required by the lms_adapt_bg() function,
           therefore the scaled version of (1) is:
    
           (2^30) * f  = (2^30) * Beta * clean_bg_rx/P    
               factor  = (2^30) * Beta * clean_bg_rx/P         ----- (2)
    
           We have chosen Beta = 0.25 by experiment, so:
    
               factor  = (2^30) * (2^-2) * clean_bg_rx/P  
    
                                           (30 - 2 - log2(P))
               factor  = clean_bg_rx 2                         ----- (3)
           
           To avoid a divide we approximate log2(P) as top_bit(P),
           which returns the position of the highest non-zero bit in
           P.  This approximation introduces an error as large as a
           factor of 2, but the algorithm seems to handle it OK.
    
           Come to think of it a divide may not be a big deal on a 
           modern DSP, so its probably worth checking out the cycles
           for a divide versus a top_bit() implementation.
        */

        P = MIN_TX_POWER_FOR_ADAPTION + ec->Pstates;	// 64+(0..2^26)=2^6...2^26
        logP = top_bit(P) + ec->log2taps;				// (6..26)+(7...10)     для 16мс...128мс, т.е.  13...36
        shift = 30 - 2 - logP;							//возможные значения     -8..15  чем выше мощность ТХ тем меньше shift, при тишине и коротком алг 15
        ec->shift = shift;

        lms_adapt_bg(ec, clean_bg, shift);
    }

    /* very simple DTD to make sure we dont try and adapt with strong
       near end speech */

    ec->adapt = 0;
    if ((ec->Lrx > MIN_RX_POWER_FOR_ADAPTION) && (ec->Lrx > ec->Ltx))
        ec->nonupdate_dwell = DTD_HANGOVER;
    if (ec->nonupdate_dwell)
        ec->nonupdate_dwell--;

    /* Transfer logic ------------------------------------------------------*/

    /* These conditions are from the dual path paper [1], I messed with
       them a bit to improve performance. */

    if ((ec->adaption_mode & ECHO_CAN_USE_ADAPTION) &&
        (ec->nonupdate_dwell == 0) && 
        (8*ec->Lclean_bg < 7*ec->Lclean) /* (ec->Lclean_bg < 0.875*ec->Lclean) */ && 
        (8*ec->Lclean_bg < ec->Ltx)      /* (ec->Lclean_bg < 0.125*ec->Ltx)    */ )
    {
        if (ec->cond_met == 6)
        {
            /* BG filter has had better results for 6 consecutive samples */
            ec->adapt = 1;
            memcpy(ec->fir_taps16[0], ec->fir_taps16[1], ec->taps*sizeof(int16_t));
        }
        else
            ec->cond_met++;
    }
    else
        ec->cond_met = 0;

    /* Non-Linear Processing ---------------------------------------------------*/

    ec->clean_nlp = ec->clean;
    if (ec->adaption_mode & ECHO_CAN_USE_NLP)
    {
        /* Non-linear processor - a fancy way to say "zap small signals, to avoid
           residual echo due to (uLaw/ALaw) non-linearity in the channel.". */

        if ((16*ec->Lclean < ec->Ltx))
        {
            /* Our e/c has improved echo by at least 24 dB (each factor of 2 is 6dB,
               so 2*2*2*2=16 is the same as 6+6+6+6=24dB) */
            if (ec->adaption_mode & ECHO_CAN_USE_CNG)
            {
                ec->cng_level = ec->Lbgn;

                /* Very elementary comfort noise generation.  Just random
                   numbers rolled off very vaguely Hoth-like.  DR: This
                   noise doesn't sound quite right to me - I suspect there
                   are some overlfow issues in the filtering as it's too
                   "crackly".  TODO: debug this, maybe just play noise at
                   high level or look at spectrum.
                */

                ec->cng_rndnum = 1664525U*ec->cng_rndnum + 1013904223U;
                ec->cng_filter = ((ec->cng_rndnum & 0xFFFF) - 32768 + 5*ec->cng_filter) >> 3;
                ec->clean_nlp = (ec->cng_filter*ec->cng_level*8) >> 14;

            }
            else if (ec->adaption_mode & ECHO_CAN_USE_CLIP)
            {
                /* This sounds much better than CNG */
                if (ec->clean_nlp > ec->Lbgn)
                    ec->clean_nlp = ec->Lbgn;
                if (ec->clean_nlp < -ec->Lbgn)
                    ec->clean_nlp = -ec->Lbgn;
            }
            else
            {
                /* just mute the residual, doesn't sound very good, used mainly
                   in G168 tests */
                ec->clean_nlp = 0;
            }
        }
        else
        {
            /* Background noise estimator.  I tried a few algorithms
               here without much luck.  This very simple one seems to
               work best, we just average the level using a slow (1 sec
               time const) filter if the current level is less than a
               (experimentally derived) constant.  This means we dont
               include high level signals like near end speech.  When
               combined with CNG or especially CLIP seems to work OK.
            */
            if (ec->Lclean < 40)
            {
                ec->Lbgn_acc += abs(ec->clean) - ec->Lbgn;
                ec->Lbgn = (ec->Lbgn_acc + (1<<11)) >> 12;
            }
        }
    }

    /* Roll around the taps buffer */
    if (ec->curr_pos <= 0)
        ec->curr_pos = ec->taps;
    ec->curr_pos--;

    if (ec->adaption_mode & ECHO_CAN_DISABLE)
        ec->clean_nlp = rx;

    /* Output scaled back up again to match input scaling */

    return(int16_t) ec->clean_nlp << 1;
}

// -------------------------------------

/* This function is seperated from the echo canceller is it is usually called
   as part of the tx process.  See rx HP (DC blocking) filter above, it's
   the same design.

   Some soft phones send speech signals with a lot of low frequency
   energy, e.g. down to 20Hz.  This can make the hybrid non-linear
   which causes the echo canceller to fall over.  This filter can help
   by removing any low frequency before it gets to the tx port of the
   hybrid.

   It can also help by removing and DC in the tx signal.  DC is bad
   for LMS algorithms.

   This is one of the classic DC removal filters, adjusted to provide sufficient
   bass rolloff to meet the above requirement to protect hybrids from things that
   upset them. The difference between successive samples produces a lousy HPF, and
   then a suitably placed pole flattens things out. The final result is a nicely
   rolled off bass end. The filtering is implemented with extended fractional
   precision, which noise shapes things, giving very clean DC removal.
*/

int16_t echo_can_hpf_tx(echo_can_state_t *ec, int16_t tx)
{
    int tmp, tmp1;

    if (ec->adaption_mode & ECHO_CAN_USE_TX_HPF)
    {
        tmp = tx << 15;

        /* Make sure the gain of the HPF is 1.0. The first can still saturate a little under
           impulse conditions, and it might roll to 32768 and need clipping on sustained peak
           level signals. However, the scale of such clipping is small, and the error due to
           any saturation should not markedly affect the downstream processing. */
        tmp -= (tmp >> 4);

        ec->tx_1 += -(ec->tx_1>>DC_LOG2BETA) + tmp - ec->tx_2;
        tmp1 = ec->tx_1 >> 15;
        if (tmp1 > 32767) tmp1 = 32767;
        if (tmp1 < -32767) tmp1 = -32767;
        tx = tmp1;
        ec->tx_2 = tmp;
    }

    return tx;
}
