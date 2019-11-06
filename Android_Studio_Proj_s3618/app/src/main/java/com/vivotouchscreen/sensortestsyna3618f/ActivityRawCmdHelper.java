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
package com.vivotouchscreen.sensortestsyna3618f;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.os.Bundle;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.style.ForegroundColorSpan;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;
import java.util.Locale;
import java.util.Vector;

public class ActivityRawCmdHelper extends Activity {

    private final String SYNA_TAG = "syna-apk";

    /********************************************************
     * object for native function access
     ********************************************************/
    private NativeWrapper native_lib;

    /********************************************************
     * handler for log file access
     ********************************************************/
    private LogFileManager log_manager = null;
    private TextCmdFileManager cmd_file_manager = null;

    /********************************************************
     * components for SNA Calculator Page
     ********************************************************/
    private Button btn_exit;
    private Button btn_run;
    private TextView cmd_file_status;
    private TextView cmd_text_win;
    /********************************************************
     * variables for files
     ********************************************************/
    private boolean b_log_save;
    private String str_cmd_file;

    /********************************************************
     * called from MainActivity if button 'COMMANDS HELPER' is pressed
     * to initial all UI objects
     ********************************************************/
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        /* to hide the navigation bar */
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        /* set view */
        setContentView(R.layout.page_command_helper);

        Log.i(SYNA_TAG, "ActivityRawCmdHelper onCreate() + " );

        /* get the apk version and change the app tittle*/
        String version_apk = "";
        try {
            PackageInfo pInfo = this.getPackageManager().getPackageInfo(getPackageName(), 0);
            version_apk = pInfo.versionName;
        }
        catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }

        /* create an object to handle native methods calling */
        native_lib = new NativeWrapper();

        /* get parameters from intent */
        Intent intent = this.getIntent();
        String str_dev_node = intent.getStringExtra(Common.STR_DEV_NODE);
        String str_dev_rmi_en = intent.getStringExtra(Common.STR_DEV_RMI_EN);
        String str_dev_tcm_en = intent.getStringExtra(Common.STR_DEV_TCM_EN);
        Log.i(SYNA_TAG, "ActivityRawCmdHelper onCreate() dev_node: " +
                str_dev_node + " (rmi: " + str_dev_rmi_en + ") (tcm: " + str_dev_tcm_en + ")");

        String str_log_en = intent.getStringExtra(Common.STR_CFG_LOG_SAVE_EN);
        if (str_log_en != null) {
            try {
                b_log_save = Boolean.valueOf(str_log_en);
            } catch (NumberFormatException e) {
                Log.e(SYNA_TAG, "ActivityRawCmdHelper fail to convert str_log_en" );
                b_log_save = false;
            }
        }
        else {
            Log.e(SYNA_TAG, "ActivityRawCmdHelper onCreate() str_log_en is invalid, use false");
            b_log_save = false;
        }

        String str_log_path = intent.getStringExtra(Common.STR_CFG_LOG_PATH);
        Log.i(SYNA_TAG, "ActivityRawCmdHelper onCreate() log saving ( " + b_log_save + " ), " +
                "path: " + str_log_path );

        str_cmd_file = intent.getStringExtra(STR_CFG_CMD_TXT_FILE);
        if (str_cmd_file == null) {
            onLoadPreferences();
        }
        Log.i(SYNA_TAG, "ActivityRawCmdHelper input file (" + str_cmd_file + " )");

        /* confirm that the device is available */
        boolean ret = native_lib.onSetupDev(str_dev_node, str_dev_rmi_en, str_dev_tcm_en);
        if (!ret) {
            onShowErrorOut("Fail to find Synaptics device node, " + str_dev_node +
                    "\n\nPlease go to \"Setting\" page to setup a valid interface," +
                    " and please confirm its permission is proper as well.");
        }
        /* at first, perform the device identification */
        ret = native_lib.onIdentifyDev(null);
        if (!ret) {
            onShowErrorOut("Fail to perform device identification\n");
        }

        /* create an object to handle log file access */
        log_manager = new LogRawCmdData(native_lib, version_apk);
        cmd_file_manager = new TextCmdFileManager();

        /* UI components - button exit */
        btn_exit = findViewById(R.id.btn_cmd_exit);
        btn_exit.setOnClickListener(_button_listener_exit);
        /* UI components - button run commands text file */
        btn_run = findViewById(R.id.btn_cmd_start);
        btn_run.setOnClickListener(_button_listener_run);
        btn_run.setEnabled(false);
        /* UI components - command text window */
        cmd_text_win = findViewById(R.id.text_cmd_window);
        /* UI components - command file input */
        cmd_file_status = findViewById(R.id.text_cmd_txt_file_status);
        cmd_file_status.setVisibility(View.INVISIBLE);
        EditText edit_cmd_file = findViewById(R.id.edit_cmd_txt_file);
        edit_cmd_file.setOnEditorActionListener(_edit_listener_cmd_file);
        if (str_cmd_file != null && !str_cmd_file.isEmpty()) {
            edit_cmd_file.setText(str_cmd_file);
            onCheckTextCmdFile(str_cmd_file);
        }

        /* confirm that the path of log file is valid */
        if (b_log_save) {
            ret = log_manager.onCheckLogFilePath(str_log_path);
            if (!ret) {
                b_log_save = false; // set flag to false if the input path is invalid
                onShowWarningDialog(str_log_path + " is invalid. Disable the log saving.");
            }
        }


        /* update the status of log saving */
        CheckBox cbtn_property_log_saving = findViewById(R.id.cbtn_cmd_log_saving);
        TextView text_property_log_saving = findViewById(R.id.text_cmd_log);
        if (b_log_save) {
            cbtn_property_log_saving.setText(R.string.str_saving_enabled);
            cbtn_property_log_saving.setChecked(true);
            text_property_log_saving.append(str_log_path);
            text_property_log_saving.setVisibility(View.VISIBLE);
        }
        else {
            cbtn_property_log_saving.setText(R.string.str_saving_disabled);
            cbtn_property_log_saving.setChecked(false);
            text_property_log_saving.setVisibility(View.INVISIBLE);
        }

        /* set default button selection */
        onSetBtnFocus(BTN_EXIT);


    } /* end onCreate() */

    /********************************************************
     * called when this application is closed
     ********************************************************/
    @Override
    public void onStop() {
        Log.i(SYNA_TAG, "ActivityRawCmdHelper onStop() + " );
        onSavePreferences();
        super.onStop();
    } /* end onStop() */

    /********************************************************
     * function to listen the KeyEvent
     * VOLUME_UP - click the button
     * VOLUME_DOWN - move button selection
     ********************************************************/
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        switch(keyCode) {
            case KeyEvent.KEYCODE_VOLUME_UP:
                if  (event.getRepeatCount() ==  0 ) {
                    event.startTracking();
                }
                if (BTN_EXIT == current_btn_idx)
                    btn_exit.performClick();
                else if  (BTN_RUN == current_btn_idx) {
                    btn_run.performClick();
                }
                break;

            case KeyEvent.KEYCODE_VOLUME_DOWN:
                current_btn_idx += 1;
                if ((current_btn_idx == BTN_RUN) && !btn_run.isEnabled())
                    current_btn_idx += 1;
                if (current_btn_idx > BTN_RUN)
                    current_btn_idx = BTN_EXIT;

                onSetBtnFocus(current_btn_idx);
                break;
        }
        return false;
    } /* end onKeyDown() */

    /********************************************************
     * implementation if 'RUN' button is pressed
     * run the commands defined in text file
     ********************************************************/
    private View.OnClickListener _button_listener_run = new View.OnClickListener() {
        @Override
        public void onClick(View view) {

            /* set default button selection */
            onSetBtnFocus(BTN_RUN);

            btn_run.setEnabled(false);
            btn_exit.setEnabled(false);

            /* parse the file content if it is available */
            if (cmd_file_manager.onParseContent(str_cmd_file, vector_cmds)) {
                doCommandExecution();
            }
            else {
                onShowWarningDialog("Fail to parse " + str_cmd_file + ".\n" +
                                "Please confirm the input file is valid.");
            }

        }
    }; /* end Button.OnClickListener() */
    /********************************************************
     * implementation if 'EXIT' button is pressed
     * go back to the MAIN page
     ********************************************************/
    private View.OnClickListener _button_listener_exit = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            Intent intent = new Intent();
            intent.setClass(ActivityRawCmdHelper.this, MainActivity.class);
            startActivity(intent);
            ActivityRawCmdHelper.this.finish();
        }
    }; /* end Button.OnClickListener() */
    /********************************************************
     * implementation if the 'TEXT COMMAND FILE' is edited
     ********************************************************/
    private TextView.OnEditorActionListener _edit_listener_cmd_file = new TextView.OnEditorActionListener() {
        @Override
        public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
            if ((actionId == EditorInfo.IME_ACTION_DONE) || (actionId == EditorInfo.IME_ACTION_NEXT)) {
                v.clearFocus();
                InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
                assert imm != null;
                imm.hideSoftInputFromWindow(v.getWindowToken(), 0);

                /* check the commands file is available */
                str_cmd_file = v.getText().toString();
                Log.i(SYNA_TAG, "ActivityRawCmdHelper onEditorAction() str_cmd_file: "
                        + str_cmd_file );

                onCheckTextCmdFile(str_cmd_file);

                return true;
            }
            return false;
        }
    }; /* end Edit.OnEditorActionListener() */
    /********************************************************
     * check the input file and parse the content if
     * it is available
     ********************************************************/
    private void onCheckTextCmdFile(String str_file) {
        Log.i(SYNA_TAG, "ActivityRawCmdHelper onCheckTextCmdFile() + " + str_file);

        boolean ret = cmd_file_manager.onCheckFile(str_file);

        /* update the status */
        if (ret) {
            cmd_file_status.setText(R.string.cmd_file_ok);
            cmd_file_status.setTextColor(Color.GREEN);
            cmd_file_status.setVisibility(View.VISIBLE);

            // enable the start button
            btn_run.setEnabled(true);
        }
        else {
            cmd_file_status.setText(R.string.cmd_file_ng);
            cmd_file_status.setTextColor(Color.RED);
            cmd_file_status.setVisibility(View.VISIBLE);

            // disable the start button
            btn_run.setEnabled(false);
        }

    }
    /********************************************************
     * function to perform the command execution
     ********************************************************/

    private Vector<RawCommand> vector_cmds = new Vector<>();

    private void doCommandExecution() {
        Log.i(SYNA_TAG, "ActivityRawCmdHelper doCommandExecution() + ");

        cmd_text_win.setText(""); // clear the text first

        boolean ret;

        /* open device */
        ret = native_lib.onOpenDev();
        if (!ret) {
            onShowWarningDialog("Error\n\nFail to open syna device\n");
            Log.e(SYNA_TAG, "ActivityRawCmdHelper doCommandExecution() fail to open syna device" );
            return;
        }

        /* prepare the log file */
        if (b_log_save) {
            log_manager.onPrepareLogFile("CmdsLog", log_manager.FILE_CSV);
        }

        for (int i = 0; i < vector_cmds.size(); i++) {

            RawCommand c = vector_cmds.elementAt(i);

            switch (c.onGetCommandCategory()) {
                case RawCommand.STR_CMD_READ:
                    onExecuteReadCommand(c, native_lib);
                    break;
                case RawCommand.STR_CMD_WRITE:
                    onExecuteWriteCommand(c, native_lib);
                    break;
                case RawCommand.STR_CMD_WAIT:
                    onExecuteWaitCommand(c);
                    break;
            }

        }

        /* close the Synaptics device as well */
        ret = native_lib.onCloseDev();
        if (!ret) {
            Log.e(SYNA_TAG, "ActivityRawCmdHelper doCommandExecution()" +
                    " fail to close syna device" );
        }

        /* save data to the log file */
        if (b_log_save) {
            log_manager.onCreateLogFile();
            onShowToastMsg("log file is saved,\n" + log_manager.onGetLogFileName());
        }

        // recovery the buttons
        btn_run.setEnabled(true);
        btn_exit.setEnabled(true);
    }

    /********************************************************
     * function to perform the command execution
     * Read Operations
     ********************************************************/
    private void onExecuteReadCommand(RawCommand c, NativeWrapper lib) {

        String str_cmd = "";
        StringBuilder str_rd_data = new StringBuilder();

        if (RawCommand.TYPE_RMI == c.onGetCommandType()) {
            /* generate the rmi read command */
            str_cmd = String.format(Locale.getDefault(),"%s 0x%04X %d\n",
                    c.onGetCommandCategory(), c.reg_addr, c.rd_len);

            onAppendCommand("◦ " + str_cmd, 0xff848484);

            /* retrieve read data  */
            byte[] rd_buf = new byte[c.rd_len];
            int retval = lib.onRunRawCommands(RawCommand.CMD_READ, c.reg_addr, rd_buf, null);
            if (retval < 0) {
                str_rd_data.append("Fail to perform a RMI command read\n");
                onAppendCommand(str_rd_data.toString(), Color.RED);
            }
            else {
                for (int i = 0; i < c.rd_len; i++) {
                    str_rd_data.append(String.format(Locale.getDefault(), "0x%02X ", rd_buf[i]));
                    if ((i != 0) && (i+1)%8 == 0) str_rd_data.append("\n");
                }
                str_rd_data.append("\n");
                onAppendCommand(str_rd_data.toString(), Color.WHITE);
            }

        }
        else if (RawCommand.TYPE_TCM == c.onGetCommandType()) {
            /* generate the tcm read command */
            str_cmd = String.format(Locale.getDefault(),"%s %d\n",
                    c.onGetCommandCategory(), c.rd_len);

            onAppendCommand("◦ " + str_cmd, 0xff848484);

            /* retrieve read data  */
            byte[] rd_buf = new byte[c.rd_len];

            int retval = lib.onRunRawCommands(RawCommand.CMD_READ, 0, rd_buf, null);
            if (retval < 0) {
                str_rd_data.append("Fail to perform a TCM command read\n");
                onAppendCommand(str_rd_data.toString(), Color.RED);
            }
            else {
                for (int i = 0; i < c.rd_len; i++) {
                    str_rd_data.append(String.format(Locale.getDefault(), "0x%02X ", rd_buf[i]));
                    if ((i != 0) && (i+1)%8 == 0) str_rd_data.append("\n");
                }
                str_rd_data.append("\n");
                onAppendCommand(str_rd_data.toString(), Color.WHITE);
            }
        }

        /* save string to the log */
        if (b_log_save) {
            log_manager.onAddLogData(str_cmd);
            log_manager.onAddLogData(str_rd_data.toString());
        }
    }

    /********************************************************
     * function to perform the command execution
     * Write Operations
     ********************************************************/
    private void onExecuteWriteCommand(RawCommand c, NativeWrapper lib) {

        StringBuilder str_cmd = new StringBuilder();
        StringBuilder str_wr_result = new StringBuilder();

        if (RawCommand.TYPE_RMI == c.onGetCommandType()) {
            byte[] wr_buf = c.wr_data;

            /* generate the rmi write command */
            str_cmd.append(String.format(Locale.getDefault(),"%s 0x%04X",
                    c.onGetCommandCategory(), c.reg_addr));
            for (byte aWr_buf : wr_buf) {
                str_cmd.append(String.format(Locale.getDefault(), " 0x%02X", aWr_buf));
            }
            str_cmd.append("\n");

            onAppendCommand("◦ " + str_cmd.toString(), 0xff848484);

            /* send a write command */
            int retval = lib.onRunRawCommands(RawCommand.CMD_WRITE, c.reg_addr, wr_buf, null);
            if (retval < 0) {
                str_wr_result.append("Fail to perform a RMI command write\n");
                onAppendCommand(str_wr_result.toString(), Color.RED);
            }
            else {
                str_wr_result.append("\n");
                onAppendCommand(str_wr_result.toString(), Color.WHITE);
            }

        }
        else if (RawCommand.TYPE_TCM ==  c.onGetCommandType()) {
            byte[] wr_buf = c.wr_data;

            /* generate the tcm write command */
            str_cmd.append(String.format(Locale.getDefault(),"%s 0x%02X",
                    c.onGetCommandCategory(), c.reg_cmd));
            for (byte aWr_buf : wr_buf) {
                str_cmd.append(String.format(Locale.getDefault(), " 0x%02X", aWr_buf));
            }
            str_cmd.append("\n");

            onAppendCommand("◦ " + str_cmd.toString(), 0xff848484);

            /* send a write command, and get the response */
            byte[] resp = new byte[4];
            int retval = lib.onRunRawCommands(RawCommand.CMD_WRITE, c.reg_cmd, wr_buf, resp);
            if (retval < 0) {
                str_wr_result.append("Fail to perform a TCM command write\n");
                onAppendCommand(str_wr_result.toString(), Color.RED);
            }
            else {
                str_wr_result.append(String.format(Locale.getDefault(),
                        "0x%02x 0x%02x 0x%02x 0x%02x \n", resp[0], resp[1], resp[2], resp[3]));
                onAppendCommand(str_wr_result.toString(), Color.WHITE);
                str_wr_result.append("-"); // add a special character for the log format

                StringBuilder str_resp = new StringBuilder();

                if ((resp[1] == 0x01) && (retval > 0)) {
                    Log.i(SYNA_TAG, "ActivityRawCmdHelper onExecuteWriteCommand" +
                            " payload size = " + retval );

                    byte[] payload = new byte[retval + 3];
                    retval = lib.onRunRawCommands(RawCommand.CMD_READ, 0,
                            payload, null);
                    if (retval < 0) {
                        str_resp.append("Fail to read TCM payload data\n");
                        onAppendCommand(str_resp.toString(), Color.RED);
                    }
                    else {
                        for (int i = 0; i < payload.length; i++) {
                            str_resp.append(String.format(Locale.getDefault(), "0x%02X ",
                                    payload[i]));
                            if ((i != 0) && (i+1)%8 == 0) str_resp.append("\n");
                        }
                        str_resp.append("\n");
                        onAppendCommand(str_resp.toString(), Color.WHITE);
                    }

                }
                else if ((resp[1] != 0x01)) {
                    str_resp.append("Timeout\n");
                    onAppendCommand(str_resp.toString(), Color.RED);
                }

                str_wr_result.append(str_resp.toString());
            }

        }

        /* save string to the log */
        if (b_log_save) {
            log_manager.onAddLogData(str_cmd.toString());
            log_manager.onAddLogData(str_wr_result.toString());
        }
    }

    /********************************************************
     * function to perform the command execution
     * Wait Operations
     ********************************************************/
    private void onExecuteWaitCommand(RawCommand c) {

        /* generate the waiting command */
        String str_given = String.format(Locale.getDefault(),"%s %d\n",
                c.onGetCommandCategory(), c.wait_t);

        onAppendCommand("◦ " + str_given, 0xff848484);

        try{
            Thread.sleep(c.wait_t);
        }catch(InterruptedException ie){
            ie.printStackTrace();
            Log.e(SYNA_TAG, "ActivityRawCmdHelper onExecuteWaitCommand() fail to do sleep" );
        }

        onAppendCommand("\n", Color.WHITE);

        /* save string to the log */
        if (b_log_save) {
            log_manager.onAddLogData(str_given);
            log_manager.onAddLogData("\n");
        }
    }

    /********************************************************
     * function to append a string onto the user interface
     *******************************************************/
    private void onAppendCommand(String str, int color) {
        SpannableStringBuilder sb = new SpannableStringBuilder(str);
        sb.setSpan(new ForegroundColorSpan(color), 0, str.length(),
                Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);

        cmd_text_win.append(sb);
    }


    /********************************************************
     * function to change the text color on specified button
     ********************************************************/
    private final int BTN_EXIT  = 0;
    private final int BTN_RUN   = 1;
    private int current_btn_idx;
    private void onSetBtnFocus(int focus_idx) {
        for (int i = 0; i < 2; i++) {
            if (focus_idx == BTN_EXIT) {
                btn_exit.setTextColor(0xffb2d0b2);
                if (btn_run.isEnabled())
                    btn_run.setTextColor(Color.WHITE);
            }
            else if (focus_idx == BTN_RUN)  {
                btn_run.setTextColor(0xffb2d0b2);
                btn_exit.setTextColor(Color.WHITE);
            }
            else {
                Log.i(SYNA_TAG, "ActivityRawCmdHelper setBtnFocus() unknown button index "
                        + focus_idx );
            }
        }
        current_btn_idx = focus_idx;
    }
    /********************************************************
     * function to save preferences
     ********************************************************/
    public void onSavePreferences() {
        SharedPreferences preference = getSharedPreferences("SynaAPKPreference_CmdHelper",
                Context.MODE_PRIVATE);
        preference.edit()
                .putString(STR_CFG_CMD_TXT_FILE, str_cmd_file)
                .apply();
    }
    /********************************************************
     * function to load from the preferences
     ********************************************************/
    public void onLoadPreferences() {
        SharedPreferences preference = getSharedPreferences("SynaAPKPreference_CmdHelper",
                Context.MODE_PRIVATE);
        str_cmd_file = preference.getString(STR_CFG_CMD_TXT_FILE, "");

        Log.i(SYNA_TAG, "ActivityRawCmdHelper onLoadPreferences() str_cmd_file: " +
                str_cmd_file);
    }

    /********************************************************
     * function to popup toast message
     ********************************************************/
    private void onShowToastMsg(String msg) {
        Toast toast = Toast.makeText(ActivityRawCmdHelper.this,
                msg, Toast.LENGTH_LONG);
        toast.setGravity(Gravity.CENTER_HORIZONTAL | Gravity.CENTER, 0, 0);
        toast.show();
    } /* end showToast() */

    /********************************************************
     * create a dialog with warning messages only
     * this function will not terminate the current process
     *******************************************************/
    private void onShowWarningDialog(String msg) {
        new AlertDialog.Builder(ActivityRawCmdHelper.this)
                .setTitle("Warning")
                .setMessage(msg)
                .setPositiveButton("OK", null).show();
    } /* end onShowWarningDialog() */

    /********************************************************
     * create a dialog with message
     * once clicking 'OK' button, go back to the main page
     *******************************************************/
    private void onShowErrorOut(String msg) {
        new AlertDialog.Builder(ActivityRawCmdHelper.this)
                .setMessage(msg)
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        Intent intent = new Intent();
                        intent.setClass(ActivityRawCmdHelper.this, MainActivity.class);
                        startActivity(intent);
                        ActivityRawCmdHelper.this.finish();
                    }
                }).show();
    }  /* end onShowErrorOut() */

    private final String STR_CFG_CMD_TXT_FILE = "CMD_TXT_FILE";
}
