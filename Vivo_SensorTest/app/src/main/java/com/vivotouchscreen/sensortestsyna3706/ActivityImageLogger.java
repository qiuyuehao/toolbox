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

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.graphics.Point;
import android.os.Bundle;
import android.util.Log;
import android.view.Display;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.RadioGroup;
import android.widget.Switch;
import android.widget.TableLayout;
import android.widget.TableRow;
import android.widget.TextView;
import android.widget.Toast;
import java.util.LinkedList;
import java.util.Locale;
import java.util.Queue;
import static java.lang.Math.abs;

public class ActivityImageLogger extends Activity {

    private final String SYNA_TAG = "syna-apk";


    /********************************************************
     * object for native function access
     ********************************************************/
    private NativeWrapper native_lib;

    /********************************************************
     * handler for log file access
     ********************************************************/
    private LogFileManager log_manager = null;

    /********************************************************
     * components for Image Data Logger Page
     ********************************************************/
    private TableLayout table_layout;
    private LinearLayout layout_control_panel;
    private View view_control_panel;
    private Button btn_exit;
    private Button btn_start;
    private TextView text_statistics;
    private TextView text_msg_to_stop;
    private CheckBox cbtn_property_log_saving;
    private TextView text_property_log_saving;
    /********************************************************
     * variables of display shown
     ********************************************************/
    private int display_width;
    private int display_height;
    /********************************************************
     * flag to save the status of logger process
     ********************************************************/
    private boolean b_running;
    /********************************************************
     * variables for log saving
     ********************************************************/
    private boolean b_log_save;
    /********************************************************
     * variables for image logger using
     ********************************************************/
    private Queue<int[]> queue_image_frames = new LinkedList<>();
    private boolean b_do_nosleep;
    private boolean b_do_rezero;
    private int image_row;
    private int image_column;
    private byte report_type;
    private int n_delta_threshold_1;
    private int n_delta_threshold_2;
    private int n_raw_threshold_1;
    private int n_raw_threshold_2;
    private int n_value_max;
    private int n_value_min;
    private boolean b_do_rotation;
    private boolean b_enable_readtime;
    private int n_frame_type;
    private final static int TYPE_CURRENT_FRAME = 0;
    private final static int TYPE_MAX_PIXELS_FRAME = 1;
    private final static int TYPE_MIN_PIXELS_FRAME = 2;

    /********************************************************
     * to sync up between the ui thread and the process
     ********************************************************/
    private final Object ui_sync = new Object();

    /********************************************************
     * called from MainActivity if button 'DIAGNOSTICS REPORT' is pressed
     * initialize all UI objects
     ********************************************************/
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        /* keep screen always on */
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
                WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        /* to hide the navigation bar */
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        /* set view */
        setContentView(R.layout.page_image_logger);

        Log.i(SYNA_TAG, "ActivityImageLogger onCreate() + " );

        /* get the apk version and change the app tittle*/
        String version_apk = "";
        try {
            PackageInfo pInfo = this.getPackageManager().getPackageInfo(getPackageName(), 0);
            version_apk = pInfo.versionName;
        }
        catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }

        /* get display resolution */
        Display display = getWindowManager().getDefaultDisplay();
        Point size = new Point();
        display.getSize(size);
        display_width = size.x;
        display_height = size.y;
        Log.i(SYNA_TAG, "ActivityImageLogger onCreate() display (width,height) = " +
                "(" + display_width + " , " + display_height + ")" );

        /* create an object to handle native methods calling */
        native_lib = new NativeWrapper();

        /* get parameters from intent */
        Intent intent = this.getIntent();
        String str_dev_node = intent.getStringExtra(Common.STR_DEV_NODE);
        String str_dev_rmi_en = intent.getStringExtra(Common.STR_DEV_RMI_EN);
        String str_dev_tcm_en = intent.getStringExtra(Common.STR_DEV_TCM_EN);
        Log.i(SYNA_TAG, "ActivityImageLogger onCreate() dev_node: " +
                str_dev_node + " (rmi: " + str_dev_rmi_en + ") (tcm: " + str_dev_tcm_en + ")");

        String str_report = intent.getStringExtra(Common.STR_CFG_REPORT_TYPE);
        if (str_report != null) {
            if (str_report.contains(Common.STR_REPORT_TYPE_RAW))
                report_type = native_lib.SYNA_RAW_REPORT_IMG;
            else
                report_type = native_lib.SYNA_DELTA_REPORT_IMG;
        }
        else {
            Log.i(SYNA_TAG, "ActivityImageLogger onCreate() str_report is invalid, use default");
            report_type = native_lib.SYNA_DELTA_REPORT_IMG;
        }
        Log.i(SYNA_TAG, "ActivityImageLogger onCreate() report_type " +
                        String.format(Locale.getDefault(), "( %s )",
                                (report_type == native_lib.SYNA_RAW_REPORT_IMG)?"raw":"delta"));

        String str_log_en = intent.getStringExtra(Common.STR_CFG_LOG_SAVE_EN);
        if (str_log_en != null) {
            try {
                b_log_save = Boolean.valueOf(str_log_en);
            } catch (NumberFormatException e) {
                Log.e(SYNA_TAG, "ActivityImageLogger onCreate() fail to convert str_log_en" );
                b_log_save = false;
            }
        }
        else {
            Log.e(SYNA_TAG, "ActivityImageLogger onCreate() str_log_en is invalid, use false");
            b_log_save = false;
        }

        String str_log_path = intent.getStringExtra(Common.STR_CFG_LOG_PATH);
        Log.i(SYNA_TAG, "ActivityImageLogger onCreate() log saving ( " + b_log_save + " ), " +
                "path: " + str_log_path );

        /* load preference settings */
        onLoadPreferences();

        /* create an object to handle log file access */
        log_manager = new LogReportData(native_lib, report_type, version_apk);

        /* UI components - table layout */
        table_layout = findViewById(R.id.table_img_image_data);
        /* UI components - texts od image logger  */
        text_statistics = findViewById(R.id.text_img_statistics);
        text_msg_to_stop = findViewById(R.id.text_img_msg_to_stop);
        text_msg_to_stop.setVisibility(View.INVISIBLE);
        /* UI components - control panel layout */
        layout_control_panel = findViewById(R.id.layout_img_ctrl_panel);
        /* UI components - switch button */
        Switch btn_switch = findViewById(R.id.btn_img_switch);
        btn_switch.setChecked(false);
        btn_switch.setOnCheckedChangeListener(_switch_listener_hide);

        /* UI components - control panel window */
        Context mContext = ActivityImageLogger.this;
        LayoutInflater layout_inflater = (LayoutInflater) mContext.getSystemService(LAYOUT_INFLATER_SERVICE);
        assert layout_inflater != null;
        final ViewGroup nullParent = null;
        view_control_panel = layout_inflater.inflate(R.layout.sub_page_image_logger, nullParent);
        layout_control_panel.addView(view_control_panel);

        /* initialize control panel */
        onInitCtrlPanel();

        /* confirm that the path of log file is valid */
        if (b_log_save) {
            boolean ret = log_manager.onCheckLogFilePath(str_log_path);
            if (!ret) {
                b_log_save = false; // set flag to false if the input path is invalid
                onShowWarningDialog(str_log_path + " is invalid. Disable the log saving.");
            }
        }

        /* update the status of log saving */
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

        /* initialize the flag */
        b_running = false;

        /* initialize the default button selection */
        onSetBtnFocus(BTN_START);

        /* confirm that the device is available */
        boolean ret = native_lib.onSetupDev(str_dev_node, str_dev_rmi_en, str_dev_tcm_en);
        if (!ret) {
            onShowErrorOut("Fail to find Synaptics device node, " + str_dev_node +
                    "\n\nPlease go to \"Setting\" page to setup a valid interface," +
                    " and please confirm its permission is proper as well.");
        }

        ret = native_lib.onIdentifyDev(null);
        if (!ret) {
            onShowErrorOut("Fail to perform device identification\n");
        }

    } /* end onCreate() */

    /********************************************************
     * called when this application is closed
     ********************************************************/
    @Override
    protected void onStop() {
        Log.i(SYNA_TAG, "ActivityImageLogger onStop() + " );
        if (b_running) {
            b_running = false;
        }

        super.onStop();
    } /* end onStop() */


    /********************************************************
     * function to listen the KeyEvent
     * VOLUME_UP - stop the report logger
     * VOLUME_DOWN - move the button focus
     ********************************************************/
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        switch(keyCode) {
            case KeyEvent.KEYCODE_VOLUME_UP:

                Log.i(SYNA_TAG, "ActivityImageLoggerKEYCODE_VOLUME_UP " );

                // stop the running logger
                if (b_running) {
                    b_running = false;
                    /* enable control panel */
                    layout_control_panel.setVisibility(View.VISIBLE);
                    text_msg_to_stop.setVisibility(View.INVISIBLE);
                }
                // perform the button click, if it is not
                else {
                    if (BTN_EXIT == current_btn_idx)
                        btn_exit.performClick();
                    else if  (BTN_START == current_btn_idx) {
                        btn_start.performClick();
                    }
                }
                break;

            case KeyEvent.KEYCODE_VOLUME_DOWN:
                if (!b_running) {
                    current_btn_idx = (current_btn_idx == BTN_EXIT)? BTN_START : BTN_EXIT;
                    onSetBtnFocus(current_btn_idx);
                }
                break;
        }
        return false;
    } /* end onKeyDown() */

    /********************************************************
     * implementation if 'Switch' button is changed
     * hide the control panel window when setting to true
     ********************************************************/
    private Switch.OnCheckedChangeListener _switch_listener_hide =
            new Switch.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {

            if (isChecked) { /* hide the control panel */
                layout_control_panel.removeView(view_control_panel);
            }
            else {  /* show the control panel */
                layout_control_panel.addView(view_control_panel);
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
            /* save preference */
            onSavePreferences();

            Intent intent = new Intent();
            intent.setClass(ActivityImageLogger.this, MainActivity.class);
            startActivity(intent);
            ActivityImageLogger.this.finish();
        }
    }; /* end Button.OnClickListener() */

    /********************************************************
     * implementation if 'START' button is pressed
     * do image logging again
     ********************************************************/
    private View.OnClickListener _button_listener_start = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            /* save preference */
            onSavePreferences();

            Log.i(SYNA_TAG, "ActivityImageLogger button onClick() " +
                    "do_no_sleep: " + b_do_nosleep + ", do_rezero: " + b_do_rezero );

            EditText threshold_1 = findViewById(R.id.edit_img_threshold_1);
            EditText threshold_2 = findViewById(R.id.edit_img_threshold_2);
            try {
                int value_1 = Integer.valueOf(threshold_1.getText().toString());
                int value_2 = Integer.valueOf(threshold_2.getText().toString());
                if (report_type == native_lib.SYNA_RAW_REPORT_IMG) {
                    n_raw_threshold_1 = value_1;
                    n_raw_threshold_2 = value_2;
                    Log.i(SYNA_TAG, "ActivityImageLogger button onClick() n_raw_threshold_1: "
                            + n_raw_threshold_1);
                    Log.i(SYNA_TAG, "ActivityImageLogger button onClick() n_raw_threshold_2: "
                            + n_raw_threshold_2);
                }
                else {
                    n_delta_threshold_1 = value_1;
                    n_delta_threshold_2 = value_2;
                    Log.i(SYNA_TAG, "ActivityImageLogger button onClick() n_delta_threshold_1: "
                            + n_delta_threshold_1);
                    Log.i(SYNA_TAG, "ActivityImageLogger button onClick() n_delta_threshold_2: "
                            + n_delta_threshold_2);
                }
            } catch (NumberFormatException e) {
                onShowToastMsg("Error\nthreshold input is invalid");
            }

            /* hide the input keyboard always */
            InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
            assert imm != null;
            imm.hideSoftInputFromWindow(view.getWindowToken(), 0);

            image_frame = null;

            /* perform report image acquisition */
            doImageAcquisition();
        }
    }; /* end Button.OnClickListener() */

    /********************************************************
     * implementation if the 'NO SLEEP' is checked
     ********************************************************/
    private CheckBox.OnCheckedChangeListener _checkbox_listener_do_nosleep =
            new CheckBox.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            b_do_nosleep = isChecked;
        }
    }; /* end Button.OnClickListener() */

    /********************************************************
     * implementation if the 'REZERO' is checked
     ********************************************************/
    private CheckBox.OnCheckedChangeListener _checkbox_listener_do_rezero =
            new CheckBox.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            b_do_rezero = isChecked;
        }
    }; /* end Button.OnClickListener() */

    /********************************************************
     * implementation if the 'IMAGE ROTATION' is checked
     ********************************************************/
    private CheckBox.OnCheckedChangeListener _checkbox_listener_do_rotation =
            new CheckBox.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            b_do_rotation = isChecked;
        }
    }; /* end Button.OnClickListener() */

    /********************************************************
     * implementation if the 'IMAGE TYPE' is changed
     ********************************************************/
    private RadioGroup.OnCheckedChangeListener _rbn_listener_frame_type =
            new RadioGroup.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(RadioGroup group, int checkedId) {

            if (R.id.rbtn_snr_frame_max_pixel == checkedId) {
                n_frame_type = TYPE_MAX_PIXELS_FRAME;
            }
            else if (R.id.rbtn_snr_frame_min_pixel == checkedId) {
                n_frame_type = TYPE_MIN_PIXELS_FRAME;
            }
            else {
                n_frame_type = TYPE_CURRENT_FRAME;
            }
        }
    }; /* end Button.OnClickListener() */

    /********************************************************
     * implementation if the 'ENABLE REAL-TIME SHOWING' is checked
     ********************************************************/
    private CheckBox.OnCheckedChangeListener _checkbox_listener_enable_realtime =
            new CheckBox.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            b_enable_readtime = isChecked;
        }
    }; /* end Button.OnClickListener() */

    /********************************************************
     * function to initialize the Control Panel
     ********************************************************/
    private void onInitCtrlPanel() {
        /* UI components - button exit */
        btn_exit = findViewById(R.id.btn_img_exit);
        btn_exit.setOnClickListener(_button_listener_exit);
        /* UI components - button start */
        btn_start = findViewById(R.id.btn_img_start);
        btn_start.setOnClickListener(_button_listener_start);

        /* UI components - checked button, no sleep */
        CheckBox cbtn_do_no_sleep = findViewById(R.id.cbtn_img_no_sleep);
        cbtn_do_no_sleep.setOnCheckedChangeListener(_checkbox_listener_do_nosleep);
        cbtn_do_no_sleep.setChecked(b_do_nosleep);

        /* UI components - checked button, rezero */
        CheckBox cbtn_do_rezero = findViewById(R.id.cbtn_img_rezero);
        cbtn_do_rezero.setOnCheckedChangeListener(_checkbox_listener_do_rezero);
        cbtn_do_rezero.setChecked(b_do_rezero);

        /* UI components - edit texts, threshold */
        EditText edit_threshold_1 = findViewById(R.id.edit_img_threshold_1);
        EditText edit_threshold_2 = findViewById(R.id.edit_img_threshold_2);
        int threshold_1 = (report_type == native_lib.SYNA_RAW_REPORT_IMG)?
                n_raw_threshold_1 : n_delta_threshold_1;
        int threshold_2 = (report_type == native_lib.SYNA_RAW_REPORT_IMG)?
                n_raw_threshold_2 : n_delta_threshold_2;
        edit_threshold_1.setText(String.valueOf(threshold_1));
        edit_threshold_2.setText(String.valueOf(threshold_2));

        /* UI components - checked button, rezero */
        CheckBox cbtn_do_rotation = findViewById(R.id.cbtn_img_do_rotation);
        cbtn_do_rotation.setOnCheckedChangeListener(_checkbox_listener_do_rotation);
        cbtn_do_rotation.setChecked(b_do_rotation);

        /* UI components - property of log saving */
        cbtn_property_log_saving = findViewById(R.id.cbtn_img_log_saving);
        text_property_log_saving = findViewById(R.id.text_img_log);

        /* UI components - radio buttons, output frame type */
        RadioGroup rbtn_frame_type = findViewById(R.id.rbtn_img_frame_type);
        rbtn_frame_type.setOnCheckedChangeListener(_rbn_listener_frame_type);
        rbtn_frame_type.getChildAt(n_frame_type).performClick();

        /* UI components - checked button, real-time showing images */
        CheckBox cbtn_real_time_en = findViewById(R.id.cbtn_img_realtime_showing);
        cbtn_real_time_en.setOnCheckedChangeListener(_checkbox_listener_enable_realtime);
        cbtn_real_time_en.setChecked(false);
        b_enable_readtime = false;
    }

    /********************************************************
     * function to change the text color on selected button
     ********************************************************/
    private final int BTN_EXIT  = 0;
    private final int BTN_START = 1;
    private int current_btn_idx;
    private void onSetBtnFocus(int focus_idx) {
        for (int i = 0; i < 2; i++) {
            if (focus_idx == BTN_EXIT) {
                btn_exit.setTextColor(0xffb2d0b2);
                btn_start.setTextColor(Color.WHITE);
            }
            else if (focus_idx == BTN_START)  {
                btn_start.setTextColor(0xffb2d0b2);
                btn_exit.setTextColor(Color.WHITE);
            }
            else {
                Log.i(SYNA_TAG, "ActivityImageLogger setBtnFocus() unknown button index "
                        + focus_idx );
            }
        }
        current_btn_idx = focus_idx;
    }


    /********************************************************
     * perform report image acquisition
     * this function will retrieve the requested report
     * and then, save it to the log file if enabled
     ********************************************************/
    private void doImageAcquisition() {
        Log.i(SYNA_TAG, "ActivityImageLogger doImageAcquisition() + " );
        /* disable the control panel */
        layout_control_panel.setVisibility(View.INVISIBLE);
        text_msg_to_stop.setVisibility(View.VISIBLE);  // enable the message about how to terminate

        n_value_max = 0;
        n_value_min = 65535;

        /* clear the queue */
        queue_image_frames.clear();

        /* create a thread to retrieve the report image from syna device node */
        /* the main thread will do ui updating  */
        Thread t = new Thread(ThreadImageAcquisition);
        t.start();

    }/* end doDeltaAcquisition() */


    /********************************************************
     * the thread to retrieve the report image
     ********************************************************/
    private boolean flag_err;

    private Runnable ThreadImageAcquisition = new Runnable() {
        @Override
        public void run() {
            boolean ret;
            int[] data_buf;

            /* prepare the log file */
            if (b_log_save) {
                ret = log_manager.onPrepareLogFile("ImageLog",
                        log_manager.FILE_CSV);
                if (!ret) {
                    Log.e(SYNA_TAG, "ActivityImageLogger ThreadImageAcquisition() " +
                            "fail to prepare the log file" );
                }
            }

            /* start the report stream */
            ret = native_lib.onStartReport(report_type, false, b_do_nosleep, b_do_rezero);
            if (!ret) {
                runOnUiThread(new Runnable() {
                    public void run(){
                        onShowErrorDialog("Error\n\nFail to enable report stream.\n");
                    }
                });
                Log.e(SYNA_TAG, "ActivityImageLogger ThreadImageAcquisition() " +
                        "fail to enable the report image" );

                if (b_log_save) {
                    log_manager.onAddErrorMessages("Error: Fail to start report image streaming",
                            native_lib);
                    /* create a log file with errors only */
                    ret = log_manager.onCreateLogFile();
                    if (!ret) {
                        Log.e(SYNA_TAG, "ActivityImageLogger ThreadImageAcquisition() " +
                                "fail to create the log file" );
                    }
                }
                return;
            }

            /* set flag to true to indicate the process is ongoing */
            b_running = true;
            flag_err = false;

            /* if the real-time mode is disabled, */
            /* create a thread to be in charge of output display */
            /* otherwise, perform the report image collection and showing in this thread */
            if (!b_enable_readtime) {
                Thread t = new Thread(ThreadImageOutput);
                t.start();
            }

            /* get the information of report image  */
            image_row = native_lib.getDevImageRow(true);
            image_column = native_lib.getDevImageCol(true);
            int btn_cnt = native_lib.getDevButtonNum();
            boolean has_hybrid = native_lib.isDevImageHasHybrid();
            int force_elecs = native_lib.getDevForceChannels();
            int size;
            if (has_hybrid)
                size = (image_row * image_column) +
                        (image_row + image_column + btn_cnt + force_elecs);
            else
                size = image_row * image_column;

            Log.i(SYNA_TAG, "ActivityImageLogger ThreadImageAcquisition() row = " + image_row
                    + ", col = " + image_column+ ",  num button = " + btn_cnt );
            if ((0 == image_row) || (0 == image_column) || (0 == size)) {
                Log.e(SYNA_TAG, "ActivityImageLogger ThreadImageAcquisition() invalid image size");

                if (b_log_save) {
                    log_manager.onAddErrorMessages("error: invalid image size " +
                            "(row,col) = (" + image_row + "," + image_column + ")");
                }

                b_running = false;
                flag_err = true;
            }

            /* collect the report image until the state of is_running is changed */
            while (b_running && !flag_err) {

                /* allocate a buffer to save image frame */
                data_buf = new int[size];

                /* call native method to retrieve a report image from syna device */
                ret = native_lib.onRequestReport(report_type, image_row, image_column,
                        data_buf, data_buf.length);
                if (!ret) {
                    Log.e(SYNA_TAG, "ActivityImageLogger ThreadImageAcquisition() " +
                            "fail to retrieve the report image");
                    b_running = false;
                    flag_err = true;
                }
                else {
                    /* push one image frame into the queue */
                    queue_image_frames.offer(data_buf);

                    Log.i(SYNA_TAG, "ActivityImageLogger ThreadImageAcquisition() " +
                            "captured frame idx = " + queue_image_frames.size());

                    /* if real-time showing is enabled, show report data */
                    if (b_enable_readtime) {

                        /* fetch one image frame */
                        onFetchImageFromQueue();

                        runOnUiThread(new Runnable() {
                            public void run() {
                                /* remove the previous display */
                                table_layout.removeAllViews();
                                table_layout.setStretchAllColumns(true);
                                /* show the image onto the UI */
                                onShowReportImage(image_frame, image_column, image_row);

                                synchronized(ui_sync) { ui_sync.notify(); }
                            }
                        });
                    }
                }
                /* if real-time showing is enabled, wait for the UI completion */
                if (b_enable_readtime) {
                    try{
                        synchronized(ui_sync){ui_sync.wait();}
                    }catch(InterruptedException ignored){}
                }

                /* add the data frame into the file_manager */
                if(b_log_save) {
                    if (flag_err) {
                        log_manager.onAddErrorMessages("Error: Fail to retrieve the report image",
                                native_lib);
                    }
                    else {
                        ret = log_manager.onAddLogData(data_buf,
                                String.format(Locale.getDefault(), "frame id = %d",
                                        queue_image_frames.size()));
                        if (!ret) {
                            Log.e(SYNA_TAG, "ActivityImageLogger ThreadImageAcquisition() " +
                                    "fail to add a report image into the log file");
                        }
                    }
                }

            } /* end while (b_running && !flag_err) */

            /* disable the syna report stream */
            ret = native_lib.onStopReport(report_type);
            if (!ret) {
                Log.e(SYNA_TAG, "ActivityImageLogger ThreadImageAcquisition() " +
                        "fail to disable the syna report" );
                if (b_log_save) {
                    log_manager.onAddErrorMessages("Error: Fail to stop report image streaming",
                            native_lib);
                }
            }

            /* create a log file */
            if (b_log_save) {
                ret = log_manager.onCreateLogFile();
                if (!ret) {
                    Log.e(SYNA_TAG, "ActivityImageLogger ThreadImageAcquisition() " +
                            "fail to create the log file" );
                }
            }

            /* show result */
            runOnUiThread(new Runnable() {
                public void run(){
                    if ((b_log_save) && (!flag_err)) {
                        onShowToastMsg("Log file is saved,\n" + log_manager.onGetLogFileName());
                    }
                    else if ((b_log_save) && (flag_err)) {
                        String msg = "Error:\n\nFail to get the syna report image\n" +
                                "See error messages in " + log_manager.onGetLogFileName();
                        onShowToastMsg(msg);
                    }
                    else if (flag_err) {
                        onShowErrorDialog("Error:\n\nFail to get the syna report image");
                    }

                    /* show control panel */
                    layout_control_panel.setVisibility(View.VISIBLE);
                    text_msg_to_stop.setVisibility(View.INVISIBLE);
                }
            });
            /* set flag to false to indicate the process is terminated */
            b_running = false;
        } /* end of run() */
    }; /* end thread_collect_image Runnable */


    /********************************************************
     * fetch a stored image frame from queue
     ********************************************************/
    private void onFetchImageFromQueue() {
        if (queue_image_frames.size() > 0) {

            if (n_frame_type != TYPE_CURRENT_FRAME) {
                int[] f = queue_image_frames.poll();

                if (image_frame == null)
                    image_frame = f;
                else {
                    for(int i = 0; i < image_row; i++) {
                        for(int j = 0; j < image_column; j++) {
                            int offset = j * image_row + i;

                            if (n_frame_type == TYPE_MIN_PIXELS_FRAME)
                                image_frame[offset] = (f[offset] < image_frame[offset])?
                                        f[offset] : image_frame[offset];
                            else
                                image_frame[offset] = (f[offset] > image_frame[offset])?
                                        f[offset] : image_frame[offset];
                        }
                    }
                }
            }
            else {  // TYPE_CURRENT_FRAME
                image_frame = queue_image_frames.poll();
            }
        }
    }

    /* to store one image frame */
    private int[] image_frame;

    /********************************************************
     * the thread to handle the image output
     ********************************************************/
    private Runnable ThreadImageOutput = new Runnable() {
        @Override
        public void run() {

            while (b_running) {

                /* fetch one image frame */
                if (queue_image_frames.size() > 0)
                    onFetchImageFromQueue();

                if (image_frame != null)
                {
                    runOnUiThread(new Runnable() {
                        public void run() {
                            /* remove the previous display */
                            table_layout.removeAllViews();
                            table_layout.setStretchAllColumns(true);
                            /* show the image onto the UI */
                            onShowReportImage(image_frame, image_column, image_row);

                            synchronized(ui_sync) { ui_sync.notify(); }
                        }
                    });
                    
                    try{
                        synchronized(ui_sync) { ui_sync.wait(); }
                    } catch(InterruptedException ignored){}
                }

            }

        } /* end of run() */
    };

    /********************************************************
     * show one report image to the table layout
     * the image is always placed in the landscape format (row > col)
     *
     *         row_0  row_1  row_2  ...  ow_(n-2) row_(n-1) row_n
     *  col_0    0      1      2    ...   (n-2)     (n-1)     n
     *  col_1  1*n+0  1*n+1  1*n+2  ... 1*n+(n-2) 1*n+(n-1) 1*n+n
     *  col_2  2*n+0  2*n+1  2*n+2  ... 2*n+(n-2) 2*n+(n-1) 2*n+n
     *    .
     *    .                     ...
     *    .
     * col_(m) m*n+0  m*n+1  m*n+2  ... m*n+(n-2) m*n+(n-1) m*n+n
     ********************************************************/
    private void onShowReportImage(int[] image, int col_num, int row_num) {
        table_layout.setShrinkAllColumns(true);
        table_layout.setStretchAllColumns(true);

        String str = " Max. = " + n_value_max + " ,  Min. = " + n_value_min;
        text_statistics.setText(str);

        int offset;
        String str_value;

        if (b_do_rotation) {

            for(int i = row_num - 1; i >= 0; i--) {
                TableRow t_row = new TableRow(ActivityImageLogger.this);

                for(int j = 0; j < col_num; j++) {

                    offset = j * row_num + i;

                    str_value = String.format(Locale.getDefault(),"% 6d", image[offset]);
                    onAddTextToTableRow(t_row,
                            str_value,
                            onGetTextColor(abs(image[offset])),
                            row_num,
                            col_num);

                    n_value_max = Math.max(n_value_max, image[offset]);
                    n_value_min = Math.min(n_value_min, image[offset]);
                }

                table_layout.addView(t_row);
            }
        }
        else {

            for(int i = 0; i < row_num; i++) {
                TableRow t_row = new TableRow(ActivityImageLogger.this);

                for(int j = 0; j < col_num; j++) {

                    offset = j * row_num + i;

                    str_value = String.format(Locale.getDefault(),"% 6d", image[offset]);
                    onAddTextToTableRow(t_row,
                            str_value,
                            onGetTextColor(abs(image[offset])),
                            row_num,
                            col_num);

                    n_value_max = Math.max(n_value_max, image[offset]);
                    n_value_min = Math.min(n_value_min, image[offset]);
                }

                table_layout.addView(t_row);
            }
        }

    } /* end showReportData() */
    /********************************************************
     * fill text into Table Row
     ********************************************************/
    private void onAddTextToTableRow(TableRow row, String str, int color,
                                     int row_num, int col_num) {
        int FONT_SIZE = 6;

        TextView text = new TextView(ActivityImageLogger.this);
        text.setText(str);
        text.setGravity(Gravity.CENTER);
        text.setTextColor(color);
        text.setBackgroundResource(R.drawable.text_view_border);
        text.setWidth(display_width/col_num);
        text.setHeight(display_height/(row_num+1));
        text.setTextSize(FONT_SIZE);
        /* add text to table row */
        row.addView(text);
    } /* end addTextToTableRow() */
    /********************************************************
     * function to determine the text color
     ********************************************************/
    private int onGetTextColor(int value) {
        int value_1 = (report_type == native_lib.SYNA_RAW_REPORT_IMG)?
                n_raw_threshold_1 : n_delta_threshold_1;
        int value_2 = (report_type == native_lib.SYNA_RAW_REPORT_IMG)?
                n_raw_threshold_2 : n_delta_threshold_2;

        int threshold_1 = Math.min(value_1, value_2);
        int threshold_2 = Math.max(value_1, value_2);

        int text_color;
        if (value >= threshold_2)
            text_color = Color.RED;
        else if (value >= threshold_1)
            text_color = Color.YELLOW;
        else
            text_color = Color.LTGRAY;

        return text_color;
    } /* end getTextColor() */
    /********************************************************
     * function to save preferences
     ********************************************************/
    private void onSavePreferences() {
        Log.i(SYNA_TAG, "ActivityImageLogger savePreferences() + " );
        SharedPreferences preference = getSharedPreferences("SynaAPKPreference_ImageLogger",
                Context.MODE_PRIVATE);
        preference.edit()
                .putBoolean(Common.STR_CFG_DO_NOSLEEP, b_do_nosleep)
                .putBoolean(Common.STR_CFG_DO_REZERO, b_do_rezero)
                .putInt(STR_CFG_DELTA_THRESHOLD_1, n_delta_threshold_1)
                .putInt(STR_CFG_DELTA_THRESHOLD_2, n_delta_threshold_2)
                .putInt(STR_CFG_RAW_THRESHOLD_1, n_raw_threshold_1)
                .putInt(STR_CFG_RAW_THRESHOLD_2, n_raw_threshold_2)
                .putBoolean(STR_CFG_DO_ROTATION, b_do_rotation)
                .putInt(STR_CFG_DISPLAYED_FRAME_TYPE, n_frame_type)
                .apply();
    }
    /********************************************************
     * function to load preferences
     ********************************************************/
    private void onLoadPreferences() {
        Log.i(SYNA_TAG, "ActivityImageLogger loadPreferences() + " );
        SharedPreferences preference = getSharedPreferences("SynaAPKPreference_ImageLogger",
                Context.MODE_PRIVATE);
        b_do_nosleep = preference.getBoolean(Common.STR_CFG_DO_NOSLEEP, true);
        b_do_rezero = preference.getBoolean(Common.STR_CFG_DO_REZERO, false);
        n_delta_threshold_1 = preference.getInt(STR_CFG_DELTA_THRESHOLD_1, 50);
        n_delta_threshold_2 = preference.getInt(STR_CFG_DELTA_THRESHOLD_2, 100);
        n_raw_threshold_1 = preference.getInt(STR_CFG_RAW_THRESHOLD_1, 5000);
        n_raw_threshold_2 = preference.getInt(STR_CFG_RAW_THRESHOLD_2, 10000);
        b_do_rotation = preference.getBoolean(STR_CFG_DO_ROTATION, false);
        n_frame_type = preference.getInt(STR_CFG_DISPLAYED_FRAME_TYPE, TYPE_CURRENT_FRAME);
    }

    /********************************************************
     * function to popup toast message
     ********************************************************/
    private void onShowToastMsg(String msg) {
        Toast toast = Toast.makeText(ActivityImageLogger.this,
                msg, Toast.LENGTH_LONG);
        toast.setGravity(Gravity.CENTER_HORIZONTAL | Gravity.CENTER, 0, 0);
        toast.show();
    } /* end onShowToastMsg() */

    /********************************************************
     * create a dialog with warning messages only
     * this function will not terminate the current process
     *******************************************************/
    private void onShowWarningDialog(String msg) {
        new AlertDialog.Builder(ActivityImageLogger.this)
                .setTitle("Warning")
                .setMessage(msg)
                .setPositiveButton("OK", null).show();
    } /* end onShowWarningDialog() */

    /********************************************************
     * create a dialog with message
     *******************************************************/
    private void onShowErrorDialog(String msg) {
        new AlertDialog.Builder(ActivityImageLogger.this)
                .setMessage(msg)
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        layout_control_panel.setVisibility(View.VISIBLE);
                        text_msg_to_stop.setVisibility(View.INVISIBLE);
                    }
                }).show();
    } /* end onShowErrorDialog() */

    /********************************************************
     * create a dialog with message
     * once clicking 'OK' button, go back to the main page
     *******************************************************/
    private void onShowErrorOut(String msg) {
        new AlertDialog.Builder(ActivityImageLogger.this)
                .setMessage(msg)
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        Intent intent = new Intent();
                        intent.setClass(ActivityImageLogger.this, MainActivity.class);
                        startActivity(intent);
                        ActivityImageLogger.this.finish();
                    }
                }).show();
    } /* end onShowErrorOut() */


    private final String STR_CFG_DELTA_THRESHOLD_1 = "CFG_DELTA_THRESHOLD_1";
    private final String STR_CFG_DELTA_THRESHOLD_2 = "CFG_DELTA_THRESHOLD_2";

    private final String STR_CFG_RAW_THRESHOLD_1 = "CFG_RAW_THRESHOLD_1";
    private final String STR_CFG_RAW_THRESHOLD_2 = "CFG_RAW_THRESHOLD_2";

    private final String STR_CFG_DO_ROTATION = "CFG_DO_ROTATION";

    private final String STR_CFG_DISPLAYED_FRAME_TYPE = "CFG_DISPLAYED_FRAME_TYPE";
}

