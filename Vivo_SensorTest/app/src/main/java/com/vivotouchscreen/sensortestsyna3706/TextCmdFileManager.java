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
package com.vivotouchscreen.sensortestsyna3706;

import android.util.Log;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.Vector;

class TextCmdFileManager {

    private final String SYNA_TAG = "syna-apk";

    /********************************************************
     * constructor
     ********************************************************/
    TextCmdFileManager() {

    }

    /********************************************************
     * helper function to ensure the appointed file is available
     *******************************************************/
    boolean onCheckFile(String str_file) {

        /* handle the empty string */
        if(str_file.length() < 1){
            return false;
        }

        File file = new File(str_file);
        if (!file.exists()) {
            Log.e(SYNA_TAG, "TextCmdFileManager: onCheckFile() file not exist, " + str_file );
            return false;
        }
		else {
            return true;
		}

    } /* end onCheckFile() */

    /********************************************************
     * helper function to parse the file content
     * and save all commands into the vector_syna_cmds
     *******************************************************/
    boolean onParseContent(String str_file, Vector<RawCommand> v) {

        final String STR_RMI = "rmi";
        final String STR_TCM = "tcm";

        v.clear();

        Log.i(SYNA_TAG, "TextCmdFileManager: onParseContent: +");
        int dev_type;

        try {
            FileReader file_reader = new FileReader(str_file);
            BufferedReader buffered_reader = new BufferedReader(file_reader);
            String text_line = buffered_reader.readLine().toLowerCase();

            if (text_line.contains(STR_RMI))
                dev_type = RawCommand.TYPE_RMI;
            else if (text_line.contains(STR_TCM))
                dev_type = RawCommand.TYPE_TCM;
            else {
                Log.e(SYNA_TAG, "TextCmdFileManager: onParseContent: " +
                        "unknown start line, " + text_line);
                return false;
            }

            text_line = buffered_reader.readLine().toLowerCase();

            do {
                /* goto next line if containing '#' */
                if (text_line.contains("#")) {
                    text_line = buffered_reader.readLine();
                    continue;
                }
                /* goto next line if it is empty */
                if (text_line.length() < 1) {
                    text_line = buffered_reader.readLine();
                    continue;
                }

                /* dispatch commands */
                switch (dev_type) {
                    case RawCommand.TYPE_RMI:
                        addRMICommand(text_line, v);
                        break;
                    case RawCommand.TYPE_TCM:
                        addTCMCommand(text_line, v);
                        break;
                }

                text_line = buffered_reader.readLine().toLowerCase();
            } while( text_line != null );

        } catch(Exception e) {
            e.printStackTrace();
            Log.e(SYNA_TAG, "TextCmdFileManager: onParseContent: " +
                    "Exception, " + str_file);
            return false;
        }

        return true;
    } /* end onCheckFile() */

    /********************************************************
     * function to add one rmi commands into vector_syna_cmds
     *
     * the format of each command must be
     *    - Read  operation :  read [HEX_REG] [LEN]
     *    - Write operation :  write [HEX_REG] [HEX_DATA_0] [HEX_DATA_1] ...
     *    - wait  operation :  wait [TIME_IN_MS]
     *******************************************************/
    private void addRMICommand(String str_line, Vector<RawCommand> v) {

        boolean ret = false;

        RawCommand cmd = new RawCommand(RawCommand.TYPE_RMI);

        String[] parts = str_line.split(" ");
        int len = parts.length - 2;

        /*                      [0]   [1]     [2]        */
        /*  Read  operation :  read [HEX_REG] [LEN]      */
        if (parts[0].contains(RawCommand.STR_CMD_READ)) {

            cmd.onSetCommandCategory(RawCommand.CMD_READ);

            int reg = Integer.decode(getHexString(parts[1]));
            int[] rd_len = new int[len];
            rd_len[0] = Integer.valueOf(parts[2]);

            ret = cmd.onSaveCommands(reg, rd_len);
        }
        /*                      [0]   [1]        [2]          [3]             */
        /*  Write operation :  write [HEX_REG] [HEX_DATA_0] [HEX_DATA_1] ...  */
        else if (parts[0].contains(RawCommand.STR_CMD_WRITE)) {

            cmd.onSetCommandCategory(RawCommand.CMD_WRITE);

            int reg = Integer.decode(getHexString(parts[1]));

            int[] wr_data = new int[len];
            for (int i = 0; i < len; i++) {
                wr_data[i] = Integer.decode(getHexString(parts[2 + i]));
            }

            ret = cmd.onSaveCommands(reg, wr_data);
        }
        /*                      [0]    [1]           */
        /*  wait  operation :  wait [TIME_IN_MS]     */
        else if (parts[0].contains(RawCommand.STR_CMD_WAIT)) {

            cmd.onSetCommandCategory(RawCommand.CMD_WAIT);

            int time = Integer.valueOf(parts[1]);

            ret = cmd.onSaveCommands(time, null);
        }

        if (ret)
            v.add(cmd);

    }

    /********************************************************
     * function to add one tcm commands into vector_syna_cmds
     *******************************************************/
    private void addTCMCommand(String str_line, Vector<RawCommand> v) {

        boolean ret = false;

        RawCommand cmd = new RawCommand(RawCommand.TYPE_TCM);

        String[] parts = str_line.split(" ");

        /*                      [0]  [1]       */
        /*  Read  operation :  read [LEN]      */
        if (parts[0].contains(RawCommand.STR_CMD_READ)) {

            cmd.onSetCommandCategory(RawCommand.CMD_READ);

            int rd_len = Integer.valueOf(parts[1]);

            ret = cmd.onSaveCommands(rd_len, null);
        }
        /*                      [0]    [1]        [2]          [3]            */
        /*  Write operation :  write [HEX_CMD] [HEX_DATA_0] [HEX_DATA_1] ...  */
        else if (parts[0].contains(RawCommand.STR_CMD_WRITE)) {

            cmd.onSetCommandCategory(RawCommand.CMD_WRITE);

            int tcm_cmd = Integer.decode(getHexString(parts[1]));

            int len = parts.length - 2;
            int[] wr_data = new int[len];
            for (int i = 0; i < len; i++) {
                wr_data[i] = Integer.decode(getHexString(parts[2 + i]));
            }

            ret = cmd.onSaveCommands(tcm_cmd, wr_data);

        }
        /*                      [0]    [1]           */
        /*  wait  operation :  wait [TIME_IN_MS]     */
        else if (parts[0].contains(RawCommand.STR_CMD_WAIT)) {

            cmd.onSetCommandCategory(RawCommand.CMD_WAIT);

            int time = Integer.valueOf(parts[1]);

            ret = cmd.onSaveCommands(time, null);
        }

        if (ret)
            v.add(cmd);

    }

    private String getHexString(String in_str) {
        String hex_str;

        if (in_str.startsWith("0x"))
            hex_str = in_str;
        else
            hex_str = "0x" + in_str;

        return hex_str;
    }
}