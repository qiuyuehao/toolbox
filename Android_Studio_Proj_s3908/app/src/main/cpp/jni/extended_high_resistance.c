/*
 * Copyright (C) 2017 Synaptics Incorporated. All rights reserved.
 *
 * This file contains information that is proprietary to Synaptics Incorporated
 * ("Synaptics"). The holder of this file shall treat all information contained
 * herein as confidential, shall use the information only for its intended
 * purpose, and shall not duplicate, disclose, or disseminate any of this
 * information in any manner unless Synaptics has otherwise provided express,
 * written permission.
 *
 * Use of the materials may require a license of intellectual property from a
 * third party or from Synaptics. Receipt or possession of this file conveys no
 * express or implied licenses to any intellectual property rights belonging to
 * Synaptics.
 *
 * INFORMATION CONTAINED IN THIS DOCUMENT IS PROVIDED "AS-IS," AND SYNAPTICS
 * EXPRESSLY DISCLAIMS ALL EXPRESS AND IMPLIED WARRANTIES, INCLUDING ANY
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE,
 * AND ANY WARRANTIES OF NON-INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHTS.
 * IN NO EVENT SHALL SYNAPTICS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, PUNITIVE, OR CONSEQUENTIAL DAMAGES ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OF THE INFORMATION CONTAINED IN THIS DOCUMENT, HOWEVER CAUSED
 * AND BASED ON ANY THEORY OF LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, AND EVEN IF SYNAPTICS WAS ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE. IF A TRIBUNAL OF COMPETENT JURISDICTION DOES
 * NOT PERMIT THE DISCLAIMER OF DIRECT DAMAGES OR ANY OTHER DAMAGES, SYNAPTICS'
 * TOTAL CUMULATIVE LIABILITY TO ANY PARTY SHALL NOT EXCEED ONE HUNDRED U.S.
 * DOLLARS.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "native_syna_lib.h"

struct ex_high_resistance_data{
    int tx_channel_count;
    int rx_channel_count;
    short **delta;
    short **baseline;
    short **reference;
    short **result;
};
struct ex_high_resistance_data g_test_data;

short** allocate_2d_array(int row, int column);
void free_2d_array(short **array_2d, int row);

short** allocate_2d_array(int row, int column)
{
    int i = 0;
    short **array_2d = (short **) malloc(sizeof(short *) * row);
    for(i = 0; i < row; i++)
        array_2d[i] = (short *) malloc(sizeof(short) * column);

    return array_2d;
}

void free_2d_array(short **array_2d, int row)
{
    short *ptr;
    for (int i = 0; i < row; i++){
        ptr = array_2d[i];
        free(ptr);
    }
    free(array_2d);
}

void bubble_sort(short *_bubble_sort, int length) {
    int i, j;
    short temp;

    for( i = 0; i < length; i++) {
        for (j = 0; j < length; j++) {
            if( _bubble_sort[j] < _bubble_sort[i] ) {
                temp = _bubble_sort[j];
                _bubble_sort[j] = _bubble_sort[i];
                _bubble_sort[i] = temp;
            }
        }
    }
    return;
}

short median(short *_median, int length)
{
    if (length % 2 == 0) {
        return (short)((_median[length / 2] + _median[(length / 2) - 1]) / 2);
    }
    else {
        return _median[length / 2];
    }
}

void fn_extended_high_resistance()
{
    short **corrected_rx = allocate_2d_array(g_test_data.tx_channel_count, g_test_data.rx_channel_count);
    short **corrected_rx_diff = allocate_2d_array(g_test_data.tx_channel_count, g_test_data.rx_channel_count);
    short **corrected_tx_rx = allocate_2d_array(g_test_data.tx_channel_count, g_test_data.rx_channel_count);

    short * rx_offset = malloc(sizeof(short) * g_test_data.rx_channel_count);
    short * tx_offset = malloc(sizeof(short) * g_test_data.tx_channel_count);
    short *rx_roe = malloc(sizeof(short) * g_test_data.rx_channel_count);
    short *tx_roe = malloc(sizeof(short) * g_test_data.tx_channel_count);

    short typ_rx_offset, typ_tx_offset;
    int i, j;
    if (g_test_data.rx_channel_count >= g_test_data.tx_channel_count)
        i = g_test_data.rx_channel_count;
    else
        i = g_test_data.tx_channel_count;

    short *_median = malloc(sizeof(short) * i);


    ////---------------step1 ( Delta = Baseline - Reference )
    // struct ex_high_resistance_data.delta

    //---------------step2 ( RxOffset = -Median(Rx of Delta) )

    for(j = 0; j < g_test_data.rx_channel_count; j++){
        for(i = 0; i < g_test_data.tx_channel_count; i++){
            _median[i] = g_test_data.delta[i][j];
        }
        bubble_sort(_median, g_test_data.tx_channel_count);
        rx_offset[j] = -median(_median, g_test_data.tx_channel_count);
    }
    //---------------step3 ( Typ_Rx_Offset = Median (RxOffset) )
    for(j = 0; j < g_test_data.rx_channel_count; j++){
        _median[j] = rx_offset[j];
    }
    bubble_sort(_median, g_test_data.rx_channel_count);
    typ_rx_offset = median(_median, g_test_data.rx_channel_count);
    //---------------step4 ( RxROE = RxOffset - Typ_Rx_Offset )
    for(j = 0; j < g_test_data.rx_channel_count; j++){
        rx_roe[j] = (short)abs(rx_offset[j] - typ_rx_offset);
        g_test_data.result[g_test_data.tx_channel_count][j] = rx_roe[j];
    }
    //---------------step5 ( CorrectedRx = Baseline - RxOffset )
    for ( i = 0; i < g_test_data.tx_channel_count; i++){
        for ( j = 0; j < g_test_data.rx_channel_count; j++){
            corrected_rx[i][j] = g_test_data.baseline[i][j] + rx_offset[j];
        }
    }
    //---------------step6 ( CorrectedRxDiff = CorrectedRx - Reference )
    for ( i = 0; i < g_test_data.tx_channel_count; i++){
        for ( j = 0; j < g_test_data.rx_channel_count; j++){
            corrected_rx_diff[i][j] = corrected_rx[i][j] - g_test_data.reference[i][j];

        }
    }
    //---------------step7 ( TxOffset = Median(CorrectedRxDiff) )
    for ( i = 0; i < g_test_data.tx_channel_count; i++){
        for ( j = 0; j < g_test_data.rx_channel_count; j++){
            _median[j] = corrected_rx_diff[i][j];
        }
        bubble_sort(_median, g_test_data.rx_channel_count);
        tx_offset[i] = -median(_median, g_test_data.rx_channel_count);
    }
    //---------------step8 ( Typ_Tx_Offset = -Median(TxOffset) )
    for(i = 0; i < g_test_data.tx_channel_count; i++){
        _median[i] = tx_offset[i];
    }
    bubble_sort(_median, g_test_data.tx_channel_count);
    typ_tx_offset = median(_median, g_test_data.tx_channel_count);

    //---------------step9 ( TxROE = abs(TxOffset - Typ_Tx_Offset) )
    for(i = 0; i < g_test_data.tx_channel_count; i++){
        tx_roe[i] = (short)abs(tx_offset[i] - typ_tx_offset);
        g_test_data.result[i][g_test_data.rx_channel_count] = tx_roe[i];
    }
    //---------------step10 ( CorrectedTx & Rx = CorrectedRx + TxOffset )
    for ( i = 0; i < g_test_data.tx_channel_count; i++){
        for ( j = 0; j < g_test_data.rx_channel_count; j++){
            corrected_tx_rx[i][j] = corrected_rx[i][j] + tx_offset[i];
        }
    }
    //---------------step11 ( SufaceError = CorrectedTx_Rx - Reference )
    for ( i = 0; i < g_test_data.tx_channel_count; i++){
        for ( j = 0; j < g_test_data.rx_channel_count; j++){
            g_test_data.result[i][j] = corrected_tx_rx[i][j] - g_test_data.reference[i][j];
        }
    }

    free_2d_array(corrected_rx, g_test_data.tx_channel_count);
    free_2d_array(corrected_rx_diff, g_test_data.tx_channel_count);
    free_2d_array(corrected_tx_rx, g_test_data.tx_channel_count);

    free(rx_offset);
    free(tx_offset);
    free(rx_roe);
    free(tx_roe);
    free(_median);
}

void init_array_delta(short *src_array, int row, int column)
{
    int i, j;
    g_test_data.delta = (short **) malloc(sizeof(short *) * row);
    for(i = 0; i < row; i++) {
        g_test_data.delta[i] = (short *) malloc(sizeof(short) * column);
        if (src_array != NULL) {
            for (j = 0; j < column; j++)
                g_test_data.delta[i][j] = src_array[(i * column) + j];
        }
    }
}
void init_array_basline(short *src_array, int row, int column)
{
    int i, j;
    g_test_data.baseline = (short **) malloc(sizeof(short *) * row);
    for(i = 0; i < row; i++) {
        g_test_data.baseline[i] = (short *) malloc(sizeof(short) * column);
        if (src_array != NULL) {
            for (j = 0; j < column; j++)
                g_test_data.baseline[i][j] = src_array[(i * column) + j];
        }
    }
}
void init_array_reference(short *src_array, int row, int column)
{
    int i, j;
    g_test_data.reference = (short **) malloc(sizeof(short *) * row);
    for(i = 0; i < row; i++) {
        g_test_data.reference[i] = (short *) malloc(sizeof(short) * column);
        if (src_array != NULL) {
            for (j = 0; j < column; j++)
                g_test_data.reference[i][j] = src_array[(i * column) + j];
        }
    }
}
void init_array_result(int row, int column)
{
    int i;
    g_test_data.result = (short **) malloc(sizeof(short *) * row);
    for(i = 0; i < row; i++) {
        g_test_data.result[i] = (short *) malloc(sizeof(short) * column);
        memset(g_test_data.result[i], 0x00, (sizeof(short) * column));
    }
}

void extended_high_resistance_test(
        unsigned char rx_2d_channel, 	/* IN: Rx Channel number for 2D area*/
        unsigned char tx_2d_channel, 	/* IN: Tx Channel number for 2D area */
        signed short * delta_2d_image, 	/* IN: Pointer to the delta image for 2D area */
        signed short * baseline_image, 	/* IN: Pointer to the baseline image for 2D area */
        signed short * ref_2d_image, 	/* IN: Pointer to the reference raw image for 2D area */
        signed short * rx_Result, 		/* OUT: Pointer to Rx_Result arry, Rx result will be kept in this arry */
        signed short * tx_Result, 		/* OUT: Pointer to Tx_Result arry, Tx result will be kept in this arry */
        signed short * surface_Result	/* OUT: Pointer to Surface_Result arry, surface result will be kept in this arry */
)
{
    g_test_data.rx_channel_count = rx_2d_channel;
    g_test_data.tx_channel_count = tx_2d_channel;

    init_array_delta(delta_2d_image, tx_2d_channel, rx_2d_channel);
    init_array_basline(baseline_image, tx_2d_channel, rx_2d_channel);
    init_array_reference(ref_2d_image, tx_2d_channel, rx_2d_channel);
    init_array_result(tx_2d_channel + 1, rx_2d_channel + 1);

    fn_extended_high_resistance();

    /* copy result to out buffer*/
    int column = rx_2d_channel;
    int row = tx_2d_channel;
    for (int i = 0; i <= row; i++)
    {
        for (int j = 0; j <= column; j++)
        {
            if (i == row)
                rx_Result[j] = g_test_data.result[i][j];
            else if (j == column)
                tx_Result[i] = g_test_data.result[i][j];
            else
                surface_Result[(i * column) + j] = g_test_data.result[i][j];
        }
    }

    free_2d_array(g_test_data.delta, tx_2d_channel);
    free_2d_array(g_test_data.baseline, tx_2d_channel);
    free_2d_array(g_test_data.reference, tx_2d_channel);
    free_2d_array(g_test_data.result, tx_2d_channel + 1);
}
