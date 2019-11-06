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
package com.vivotouchscreen.sensortestsyna3908;

public class RawCommand {

    /* command type, could be the tcm command or rmi command */
    private int type;
    static final int TYPE_NONE = 0;
	static final int TYPE_RMI = 1;
    static final int TYPE_TCM = 2;

    /* command category, could belong to read, write, or a wait command */
    private int category;
    static final byte CMD_WAIT = 0x10;
    static final byte CMD_WRITE = 0x11;
    static final byte CMD_READ = 0x12;

    static final String STR_CMD_READ = "read";
    static final String STR_CMD_WRITE = "write";
    static final String STR_CMD_WAIT = "wait";

    int reg_cmd;
	int reg_addr;
    int rd_len;
    byte[] wr_data;
    int wait_t;
	
	/********************************************************
     * Constructor to create a type of command
     ********************************************************/
    RawCommand(int _type) {
        type = _type;
    }

    /********************************************************
     * save a command set
     ********************************************************/
    boolean onSaveCommands(int _para_1, int[] _para_2) {
        boolean ret = false;
        switch (type) {

            /* rmi command */
            case TYPE_RMI:

                if (CMD_READ == category) {
                    // _para_1 : register address
                    // _para_2 : read length
                    reg_addr = _para_1;
                    rd_len = _para_2[0];

                    ret = true;
                }
                else if (CMD_WRITE == category) {
                    // _para_1 : register address
                    // _para_2 : write data
                    reg_addr = _para_1;
                    wr_data = new byte[_para_2.length];
                    for (int i = 0; i < _para_2.length; i++) {
                        wr_data[i] = (byte)_para_2[i];
                    }

                    ret = true;
                }
                else if (CMD_WAIT == category) {
                    // _para_1 : waiting time
                    wait_t = _para_1;

                    ret = true;
                }

                break;

            /* tcm command */
            case TYPE_TCM:

                if (CMD_READ == category) {
                    // _para_1 : read length
                    rd_len = _para_1;
                    ret = true;
                }
                else if (CMD_WRITE == category) {
                    // _para_1 : tcm command
                    // _para_2 : tcm write data
                    reg_cmd = _para_1;
                    wr_data = new byte[_para_2.length];
                    for (int i = 0; i < _para_2.length; i++) {
                        wr_data[i] = (byte)_para_2[i];
                    }

                    ret = true;
                }
                else if (CMD_WAIT == category) {
                    // _para_1 : waiting time
                    wait_t = _para_1;

                    ret = true;
                }

                break;
        }
        return ret;
    }

    /********************************************************
     * helper to setup the command category
     ********************************************************/
    void onSetCommandCategory(int _category) {
        category = _category;
    }

    String onGetCommandCategory() {
        String str = null;
        switch (category) {
            case CMD_READ:
                str = STR_CMD_READ;
                break;
            case CMD_WRITE:
                str = STR_CMD_WRITE;
                break;
            case CMD_WAIT:
                str = STR_CMD_WAIT;
                break;
        }
        return str;
    }

    /********************************************************
     * helper to retrieve the command type
     ********************************************************/
    int onGetCommandType() {
        return type;
    }

}