/*
 * Copyright (c)  2012-2018 Synaptics Incorporated. All rights reserved.
 * This file contains information that is proprietary to Synaptics
 * Incorporated ("Synaptics"). The holder of this file shall treat all
 * information contained herein as confidential, shall use the
 * information only for its intended purpose, and shall not duplicate,
 * disclose, or disseminate any of this information in any manner unless
 * Synaptics has otherwise provided express, written permission.
 * Use of the materials may require a license of intellectual property
 * from a third party or from Synaptics. Receipt or possession of this
 * file conveys no express or implied licenses to any intellectual
 * property rights belonging to Synaptics.
 * INFORMATION CONTAINED IN THIS DOCUMENT IS PROVIDED "AS-IS," AND
 * SYNAPTICS EXPRESSLY DISCLAIMS ALL EXPRESS AND IMPLIED WARRANTIES,
 * INCLUDING ANY IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE, AND ANY WARRANTIES OF NON-INFRINGEMENT OF ANY
 * INTELLECTUAL PROPERTY RIGHTS. IN NO EVENT SHALL SYNAPTICS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, PUNITIVE, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF OR IN CONNECTION WITH THE USE OF
 * THE INFORMATION CONTAINED IN THIS DOCUMENT, HOWEVER CAUSED AND BASED
 * ON ANY THEORY OF LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, AND EVEN IF SYNAPTICS WAS ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE. IF A TRIBUNAL OF COMPETENT
 * JURISDICTION DOES NOT PERMIT THE DISCLAIMER OF DIRECT DAMAGES OR ANY
 * OTHER DAMAGES, SYNAPTICS' TOTAL CUMULATIVE LIABILITY TO ANY PARTY
 * SHALL NOT EXCEED ONE HUNDRED U.S. DOLLARS.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "err_msg_ctrl.h"

/* variables to keep the error message */
struct error_message {
    bool is_fill;
    char *msg;
};
static struct error_message err_msg[MAX_ERR_MSG_CNT];

/* global variables to indicate the index of the array */
static int idx_err;
/*
 * Function:  add_error_msg
 * --------------------
 * add one error message into the buffer list
 *
 * return: void
 */
void add_error_msg(const char *msg)
{
    if (idx_err > MAX_ERR_MSG_CNT) {
        return;
    }

    err_msg[idx_err].is_fill = true;
    err_msg[idx_err].msg = strdup(msg);

    idx_err += 1;
}
/*
 * Function:  clear_all_error_msg
 * --------------------
 * clear all error messages
 *
 * return: void
 */
void clear_all_error_msg()
{
    int i;
    for (i = 0 ; i < idx_err; i++) {
        if ((err_msg[i].msg) && (strlen(err_msg[i].msg) != 0)) {
            err_msg[i].is_fill = false;
            free(err_msg[i].msg);
            err_msg[i].msg = NULL;
        }
    }
    idx_err = 0;
}
/*
 * Function:  get_num_err_msg
 * --------------------
 * return the number of err messages being stored
 *
 * return: integer
 */
int get_num_err_msg() {
    return idx_err;
}
/*
 * Function:  get_err_msg
 * --------------------
 * return the appointed err messages
 *
 * return: char*
 */
char* get_err_msg(int idx) {

    if (idx > MAX_ERR_MSG_CNT) {
        return NULL;
    }

    if (err_msg[idx].is_fill)
        return err_msg[idx].msg;
    else
        return NULL;
}