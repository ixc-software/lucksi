
#ifndef _ECHO_H_
#define _ECHO_H_

#include "fir.h"   
#include "EchoIntTypes.h"
#include "IEchoDebug.h"

/* Mask bits for the adaption mode */

#define ECHO_CAN_USE_ADAPTION       0x01
#define ECHO_CAN_USE_NLP            0x02
#define ECHO_CAN_USE_CNG            0x04
#define ECHO_CAN_USE_CLIP           0x08
#define ECHO_CAN_USE_TX_HPF         0x10
#define ECHO_CAN_USE_RX_HPF         0x20
#define ECHO_CAN_DISABLE            0x40

/*!
    G.168 echo canceller descriptor. This defines the working state for a line
    echo canceller.
*/

typedef struct 
{
    int16_t tx,rx;
    int16_t clean;
    int16_t clean_nlp;

    int nonupdate_dwell;
    int curr_pos;	
    int taps;
    int log2taps;
    int adaption_mode;

    int cond_met;
    int32_t Pstates;
    int16_t adapt;
    int32_t factor;
    int16_t shift;

    /* Average levels and averaging filter states */ 
    int Ltxacc, Lrxacc, Lcleanacc, Lclean_bgacc;
    int Ltx, Lrx;
    int Lclean;
    int Lclean_bg;
    int Lbgn, Lbgn_acc, Lbgn_upper, Lbgn_upper_acc;

    /* foreground and background filter states */
    fir16_state_t fir_state;
    fir16_state_t fir_state_bg;
    int16_t *fir_taps16[2];
    
    /* DC blocking filter states */
    int tx_1, tx_2, rx_1, rx_2;
   
    /* optional High Pass Filter states */
    int32_t xvtx[5], yvtx[5];
    int32_t xvrx[5], yvrx[5];
   
    /* Parameters for the optional Hoth noise generator */
    int cng_level;
    int cng_rndnum;
    int cng_filter;
    
    /* snapshot sample of coeffs used for development */
    int16_t *snapshot;       

    // debug
    int BgAdoptCalled;

#ifdef __cplusplus
    iDSP::IEchoDebug *EchoDebug;
#endif

} echo_can_state_t;

/*! Create a voice echo canceller context.
    \param len The length of the canceller, in samples.
    \return The new canceller context, or NULL if the canceller could not be created.
*/
echo_can_state_t *echo_can_create(int len, int adaption_mode);

/*! Free a voice echo canceller context.
    \param ec The echo canceller context.
*/
void echo_can_free(echo_can_state_t *ec);

/*! Flush (reinitialise) a voice echo canceller context.
    \param ec The echo canceller context.
*/
void echo_can_flush(echo_can_state_t *ec);

/*! Set the adaption mode of a voice echo canceller context.
    \param ec The echo canceller context.
    \param adapt The mode.
*/
void echo_can_adaption_mode(echo_can_state_t *ec, int adaption_mode);

void echo_can_snapshot(echo_can_state_t *ec);

/*! Process a sample through a voice echo canceller.
    \param ec The echo canceller context.
    \param tx The transmitted audio sample.
    \param rx The received audio sample.
    \return The clean (echo cancelled) received sample.
*/
int16_t echo_can_update(echo_can_state_t *ec, int16_t tx, int16_t rx);

/*! Process to high pass filter the tx signal.
    \param ec The echo canceller context.
    \param tx The transmitted auio sample.
    \return The HP filtered transmit sample, send this to your D/A.
*/
int16_t echo_can_hpf_tx(echo_can_state_t *ec, int16_t tx);

#endif



