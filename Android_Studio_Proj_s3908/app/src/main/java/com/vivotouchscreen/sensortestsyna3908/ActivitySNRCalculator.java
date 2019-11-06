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
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.RadioGroup;
import android.widget.Switch;
import android.widget.TableLayout;
import android.widget.TableRow;
import android.widget.TextView;
import android.widget.Toast;
import java.util.Locale;
import java.util.Vector;
import static java.lang.Math.abs;

public class ActivitySNRCalculator extends Activity {

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
     * variables of drawing
     ********************************************************/
    private Drawing drawing_obj;

    /********************************************************
     * components for SNA Calculator Page
     ********************************************************/
    private FrameLayout root_layout;
    private TableLayout tlayout_img_data;
    private LinearLayout llayout_canvas_base;
    private TableLayout tlayout_canvas;
    private LinearLayout llayout_control_panel;
    private View view_control_panel;
    private Button btn_exit;
    private Button btn_start;
    private TextView text_msg_to_stop;
    private TextView text_msg_1;
    private TextView text_msg_2;
    private ProgressBar progressbar;
    private TextView text_snr_result;
    private CheckBox cbtn_property_log_saving;
    private TextView text_property_log_saving;
    /********************************************************
     * variables of display shown
     ********************************************************/
    private int display_width;
    private int display_height;
    /********************************************************
     * flag to save the status of process
     ********************************************************/
    private boolean b_running;
    /********************************************************
     * variables for log saving
     ********************************************************/
    private boolean b_log_save;
    /********************************************************
     * variables for snr calculation using
     ********************************************************/
    private int required_frames;
    private int frame_row;
    private int frame_column;

    private final int N_FRAMES_WAIT_STABLE = 10;

    /********************************************************
     * variables result frames
     ********************************************************/
    private double[] result_img_untouched_avg;
    private double[] result_img_delta_avg;
    private double[] result_img_noise_rms;
    private double[] result_img_snr;
    private double result_snr;
    /********************************************************
     * variables for ui update
     ********************************************************/
    private boolean b_color_white;
    private int color_font;
    private boolean b_wait_touch;
    
    /********************************************************
     * called from MainActivity if button 'SNR Calculator' is pressed
     * to initial all UI objects
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
        setContentView(R.layout.page_snr_calculator);

        Log.i(SYNA_TAG, "ActivitySNRCalculator onCreate() + " );

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
        Log.i(SYNA_TAG, "ActivitySNRCalculator onCreate() display (width,height) = " +
                "(" + display_width + " , " + display_height + ")" );

        /* create an object to handle native methods calling */
        native_lib = new NativeWrapper();

        /* get parameters from intent */
        Intent intent = this.getIntent();
        String str_dev_node = intent.getStringExtra(Common.STR_DEV_NODE);
        String str_dev_rmi_en = intent.getStringExtra(Common.STR_DEV_RMI_EN);
        String str_dev_tcm_en = intent.getStringExtra(Common.STR_DEV_TCM_EN);
        Log.i(SYNA_TAG, "ActivitySNRCalculator onCreate() dev_node: " +
                str_dev_node + " (rmi: " + str_dev_rmi_en + ") (tcm: " + str_dev_tcm_en + ")");

        String str_log_en = intent.getStringExtra(Common.STR_CFG_LOG_SAVE_EN);
        if (str_log_en != null) {
            try {
                b_log_save = Boolean.valueOf(str_log_en);
            } catch (NumberFormatException e) {
                Log.e(SYNA_TAG, "ActivitySNRCalculator onCreate() fail to convert str_log_en" );
                b_log_save = false;
            }
        }
        else {
            Log.e(SYNA_TAG, "ActivitySNRCalculator onCreate() str_log_en is invalid, use false");
            b_log_save = false;
        }

        String str_log_path = intent.getStringExtra(Common.STR_CFG_LOG_PATH);
        Log.i(SYNA_TAG, "ActivitySNRCalculator onCreate() log saving ( " + b_log_save + " ), " +
                "path: " + str_log_path );

        /* load preference settings */
        onLoadPreferences();

        /* create an object to handle log file access */
        log_manager = new LogSNRData(native_lib, version_apk);

        /* UI components - root layout */
        root_layout = findViewById(R.id.root_snr_view);

        /* UI components - table layout */
        tlayout_img_data = findViewById(R.id.table_snr_image_data);
        tlayout_img_data.setVisibility(View.INVISIBLE);

        /* UI components - canvas */
        llayout_canvas_base = findViewById(R.id.canvas_snr_view);
        tlayout_canvas = findViewById(R.id.table_snr_canvas);

        /* UI components - texts */
        text_msg_to_stop = findViewById(R.id.text_snr_msg_to_stop);
        text_msg_to_stop.setVisibility(View.INVISIBLE);
        text_msg_1 = findViewById(R.id.text_snr_message_1);
        text_msg_1.setVisibility(View.INVISIBLE);
        text_msg_2 = findViewById(R.id.text_snr_message_2);
        text_msg_2.setVisibility(View.INVISIBLE);

        /* UI components - control panel layout */
        llayout_control_panel = findViewById(R.id.layout_snr_ctrl_panel);

        /* UI components - control panel window */
        Context mContext = ActivitySNRCalculator.this;
        LayoutInflater layout_inflater =
                (LayoutInflater) mContext.getSystemService(LAYOUT_INFLATER_SERVICE);
        assert layout_inflater != null;
        final ViewGroup nullParent = null;
        view_control_panel = layout_inflater.inflate(R.layout.sub_page_snr_calculator, nullParent);
        llayout_control_panel.addView(view_control_panel);

        /* UI components - switch button */
        Switch btn_switch = findViewById(R.id.btn_snr_switch);
        btn_switch.setChecked(false);
        btn_switch.setOnCheckedChangeListener(_switch_listener_hide);

        /* UI components - progressbar */
        progressbar = findViewById(R.id.progress_snr_bar);

        /* initialize control panel */
        onInitCtrlPanel();

        /* confirm that the path of log file is valid */
        if (b_log_save) {
            boolean ret = log_manager.onCheckLogFilePath(str_log_path);
            if (!ret) {
                b_log_save = false; // set flag to false if the input path is invalid
                onShowErrorDialog(str_log_path + " is invalid. Disable the log saving.");
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

        /* initialize background color */
        if (b_color_white)
            root_layout.setBackgroundColor(Color.WHITE);
        else
            root_layout.setBackgroundColor(Color.BLACK);

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


        frame_row = native_lib.getDevImageRow(true);
        frame_column = native_lib.getDevImageCol(true);
        result_img_untouched_avg = new double[frame_row * frame_column];
        result_img_delta_avg = new double[frame_row * frame_column];
        result_img_noise_rms = new double[frame_row * frame_column];
        result_img_snr = new double[frame_row * frame_column];

    } /* end onCreate() */

    /********************************************************
     * called when this application is closed
     ********************************************************/
    @Override
    public void onStop() {
        Log.i(SYNA_TAG, "ActivitySNRCalculator onStop() + " );
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
                if  (event.getRepeatCount() ==  0 ) {
                    event.startTracking();
                }
                // stop the running logger
                if (b_running) {
                    b_running = false;
                    /* enable control panel */
                    llayout_control_panel.setVisibility(View.VISIBLE);
                    text_msg_to_stop.setVisibility(View.INVISIBLE);
                }
                // perform the button click, if the process is not running
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
                    current_btn_idx += 1;
                    if (current_btn_idx > BTN_START)
                        current_btn_idx = BTN_EXIT;
                    onSetBtnFocus(current_btn_idx);
                }
                break;
        }
        return false;
    } /* end onKeyDown() */

    /********************************************************
     * function to listen the TouchEvent
     ********************************************************/
    @Override
    public boolean onTouchEvent(MotionEvent event) {

        int x, y;
        int masked_action = event.getActionMasked();

        switch( masked_action ) {
            case MotionEvent.ACTION_DOWN:
                if (0 == event.getActionIndex()) {
                    Log.i(SYNA_TAG, "ActivitySNRCalculator ACTION_DOWN" );
                    if (b_wait_touch) {
                        x = (int)event.getX(0);
                        y = (int)event.getY(0);

                        onShowUIMessage(getResources().getString(R.string.snr_msg_touch_collect),
                                ( y < (display_height/3)));
                        onDrawCircle(x, y);

                        b_wait_touch = false;
                    }
                }
                break;
            case MotionEvent.ACTION_UP:
                Log.i(SYNA_TAG, "ActivitySNRCalculator ACTION_UP" );
                break;
        }

        return true;
    } /* end onTouchEvent() */

    /********************************************************
     * implementation if 'Switch' button is changed
     * hide the control panel window when setting to true
     ********************************************************/
    private Switch.OnCheckedChangeListener _switch_listener_hide =
            new Switch.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {

            if (isChecked) { /* hide the control panel */
                llayout_control_panel.removeView(view_control_panel);
            }
            else {  /* show the control panel */
                llayout_control_panel.addView(view_control_panel);
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
            intent.setClass(ActivitySNRCalculator.this, MainActivity.class);
            startActivity(intent);
            ActivitySNRCalculator.this.finish();
        }
    }; /* end Button.OnClickListener() */

    /********************************************************
     * implementation if 'START' button is pressed
     * do image logging again
     ********************************************************/
    private View.OnClickListener _button_listener_start = new View.OnClickListener() {
        @Override
        public void onClick(View view) {

            /* clear the result frame */
            for (int i = 0; i < frame_row * frame_column; i++) {
                result_img_untouched_avg[i] = 0;
                result_img_delta_avg[i] = 0;
                result_img_noise_rms[i] = 0;
                result_img_snr[i] = 0;
            }
            result_snr = 0;

            /* update the required_frames */
            EditText edit_frames = findViewById(R.id.edit_snr_frames);
            try {
                required_frames = Integer.valueOf(edit_frames.getText().toString());
            } catch (NumberFormatException e) {
                onShowToastMsg("Error\nInput of SNR frames is invalid.");
            }

            /* hide the input keyboard always */
            InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
            assert imm != null;
            imm.hideSoftInputFromWindow(view.getWindowToken(), 0);

            /* save preference */
            onSavePreferences();

            /* initial progress bar */
            progressbar.setMax(required_frames + (N_FRAMES_WAIT_STABLE * 2));
            progressbar.setProgress(0);


            /* call function to perform SNR calculation */
            doSNRCalculation();

        }
    }; /* end Button.OnClickListener() */

    /********************************************************
     * implementation if the 'BACKGROUND COLOR' is changed
     ********************************************************/
    private RadioGroup.OnCheckedChangeListener _rbn_listener_background =
            new RadioGroup.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(RadioGroup group, int checkedId) {
            b_color_white = (R.id.rbtn_snr_background_white == checkedId);

            if (b_color_white)
                root_layout.setBackgroundColor(Color.WHITE);
            else
                root_layout.setBackgroundColor(Color.BLACK);
        }
    }; /* end Button.OnClickListener() */

    /********************************************************
     * implementation if the 'OUTPUT RESULT' is changed
     ********************************************************/
    private RadioGroup.OnCheckedChangeListener _rbn_listener_frame_type =
            new RadioGroup.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(RadioGroup group, int checkedId) {

            tlayout_img_data.removeAllViews();

            if (R.id.rbtn_snr_frame_untouch == checkedId) {
                onShowImage(result_img_untouched_avg, frame_column, frame_row);
            }
            else if (R.id.rbtn_snr_frame_touch == checkedId) {
                onShowImage(result_img_delta_avg, frame_column, frame_row);
            }
            else {
                onShowImage(result_img_snr, frame_column, frame_row);
            }
        }
    }; /* end Button.OnClickListener() */

    /********************************************************
     * function to initialize the Control Panel
     ********************************************************/
    private void onInitCtrlPanel() {
        /* UI components - button exit */
        btn_exit = findViewById(R.id.btn_snr_exit);
        btn_exit.setOnClickListener(_button_listener_exit);
        /* UI components - button start */
        btn_start = findViewById(R.id.btn_snr_start);
        btn_start.setOnClickListener(_button_listener_start);

        /* UI components - radio buttons, background color */
        RadioGroup rbtn_background = findViewById(R.id.rbtn_snr_background);
        rbtn_background.setOnCheckedChangeListener(_rbn_listener_background);
        if (b_color_white) {
            rbtn_background.check(R.id.rbtn_snr_background_white);
        }
        else {
            rbtn_background.check(R.id.rbtn_snr_background_black);
        }

        /* UI components - edit texts, number of captured frames */
        EditText edit_frames = findViewById(R.id.edit_snr_frames);
        edit_frames.setText(String.valueOf(required_frames));

        /* UI components - texts, SNR result */
        onInitSnrResult(false);

        /* UI components - radio buttons, output frame selection */
        onInitSnrOutputSelection(false);

        /* UI components - property of log saving */
        cbtn_property_log_saving = findViewById(R.id.cbtn_snr_log_saving);
        text_property_log_saving = findViewById(R.id.text_snr_log);
    }

    /********************************************************
     * function to initialize the Text of SNR Result
     ********************************************************/
    private void onInitSnrResult(boolean en) {

        TextView text_snr_title = findViewById(R.id.text_snr_result_title);
        text_snr_title.setEnabled(en);
        text_snr_result = findViewById(R.id.text_snr_result);
        text_snr_result.setEnabled(en);
    }

    /********************************************************
     * function to initialize the rButton of Output Frame
     ********************************************************/
    private void onInitSnrOutputSelection(boolean en) {

        tlayout_img_data.setVisibility(View.VISIBLE);

        TextView text_frame_type_title = findViewById(R.id.text_snr_frame_type_title);
        text_frame_type_title.setEnabled(en);
        RadioGroup rbtn_frame_review = findViewById(R.id.rbtn_snr_frame_type);
        rbtn_frame_review.setOnCheckedChangeListener(_rbn_listener_frame_type);
        for (int i = 0; i < rbtn_frame_review.getChildCount(); i++)
            rbtn_frame_review.getChildAt(i).setEnabled(en);

        if (en) {
            rbtn_frame_review.check(R.id.rbtn_snr_frame_snr);
            onShowImage(result_img_snr, frame_column, frame_row);
        }
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
                Log.i(SYNA_TAG, "ActivitySNRCalculator setBtnFocus() unknown button index "
                        + focus_idx );
            }
        }
        current_btn_idx = focus_idx;
    }

    /********************************************************
     * function to perform the SNR calculation
     * basically, there are four main steps
     *
     * 1. collect the untouched frames
     * 2. collect the touch frames
     * 3. calculate the delta frames
     * 4. calculate the SNR
     *
     ********************************************************/
    int ret_code;

    private void doSNRCalculation() {
        Log.i(SYNA_TAG, "ActivitySNRCalculator doSNRCalculation() + ");
        /* disable the control panel */
        llayout_control_panel.setVisibility(View.INVISIBLE);
        /* disable the image data layout */
        tlayout_img_data.setVisibility(View.INVISIBLE);
        tlayout_img_data.removeAllViews();

        /* prepare the canvas */
        llayout_canvas_base.setVisibility(View.VISIBLE);
        int color_background = (b_color_white) ? Color.WHITE : Color.BLACK;
        color_font = (b_color_white)? 0xFF585858 : 0xFFD8D8D8;
        drawing_obj = new Drawing(this, display_width, display_height, color_background);
        tlayout_canvas.addView(drawing_obj);
        tlayout_canvas.setVisibility(View.VISIBLE);

        /* show helping message */
        text_msg_to_stop.setVisibility(View.VISIBLE);

        /* clear the flag */
        b_running = false;
        b_wait_touch = false;

        /* create a thread to run report image logger */
        new Thread(new Runnable() {
            public void run() {
                boolean ret;

                Vector<int[]> vector_frames_untouched = new Vector<>();
                Vector<int[]> vector_frames_touched = new Vector<>();
                Vector<int[]> vector_frames_delta = new Vector<>();

                /* prepare the log file */
                if (b_log_save) {
                    log_manager.onPrepareLogFile("SNRLog", log_manager.FILE_CSV);
                }

                /* set flag to true to indicate the process is ongoing */
                b_running = true;
                ret_code = RET_NO_ERROR;

                frame_row = native_lib.getDevImageRow(true);
                frame_column = native_lib.getDevImageCol(true);

                /* step 1. collect untouched frames */
                ret_code = collectUnTouchedFrames(native_lib,
                                            required_frames,
                                            vector_frames_untouched,
                                            native_lib.SYNA_DELTA_REPORT_IMG,
                                            frame_row * frame_column);
                if (ret_code != RET_NO_ERROR) {
                    Log.e(SYNA_TAG, "ActivitySNRCalculator doSNRCalculation() " +
                            "fail to collect untouched frames" );
                }

                /* step 2. collect touched frames */
                ret_code = collectTouchedFrames(native_lib,
                                            required_frames,
                                            vector_frames_touched,
                                            native_lib.SYNA_DELTA_REPORT_IMG,
                                            frame_row * frame_column);
                if (ret_code != RET_NO_ERROR) {
                    Log.e(SYNA_TAG, "ActivitySNRCalculator doSNRCalculation() " +
                            "fail to collect touched frames");
                }


                /* step 3. calculate touch strength */
                ret_code = calculateStrength(vector_frames_delta,
                                            vector_frames_untouched,
                                            vector_frames_touched,
                                            frame_row,
                                            frame_column);

                // step 4. calculate noise RMS
                ret_code = calculateNoiseRMS(vector_frames_delta,
                                            result_img_delta_avg,
                                            frame_row,
                                            frame_column);

                // step 5. calculate the SNR
                int[] pos = new int[2];
                StringBuilder result_str = new StringBuilder() ;
                ret_code = calculateSNR(result_img_delta_avg,
                                        result_img_noise_rms,
                                        frame_row,
                                        frame_column,
                                        result_str,
                                        pos);

                /* save data to the log file */
                if (b_log_save && (ret_code == RET_NO_ERROR)) {
                    writeLogSNR(result_str.toString(), result_img_snr, result_img_delta_avg,
                            result_img_noise_rms, result_img_untouched_avg, required_frames,
                            pos[0], pos[1], pos[1] * frame_row + pos[0],
                            vector_frames_delta, vector_frames_touched, vector_frames_untouched);
                }

                /* save data to the log file */
                if (b_log_save) {
                    ret = log_manager.onCreateLogFile();
                    if (!ret) {
                        Log.e(SYNA_TAG, "ActivitySNRCalculator doSNRCalculation() " +
                                "fail to create the log file" );
                    }
                }

                /* disable and remove the canvas */
                runOnUiThread(new Runnable() {
                    public void run(){
                        onDrawClear();
                        text_msg_to_stop.setVisibility(View.INVISIBLE);
                        tlayout_canvas.removeView(drawing_obj);
                        llayout_canvas_base.setVisibility(View.INVISIBLE);
                        drawing_obj = null;
                    }
                });

                /* set result string */
                onShowSnrResult();
                b_running = false;

                /* output result frame and recovery the control panel */
                runOnUiThread(new Runnable() {
                    public void run(){

                        if (b_log_save) {
                            onShowToastMsg("Log file is saved,\n" + log_manager.onGetLogFileName());
                        }

                        /* recovery the control panel */
                        llayout_control_panel.setVisibility(View.VISIBLE);

                        /* enable the ui components */
                        onInitSnrResult(true);
                        onInitSnrOutputSelection(true);
                    }
                });

            } /* end of run() */
        }).start(); /* start the Thread */


    }
    /********************************************************
     * function to collect the untouched frames
     *
     * update the message on ui
     * perform report image acquisition, called F_Untouched[n]
     ********************************************************/
    private int collectUnTouchedFrames(NativeWrapper lib, int total_required_frames,
                                       Vector<int[]> v, byte report_type, int size)
    {
        if (ret_code != RET_NO_ERROR) {
            Log.e(SYNA_TAG, "ActivitySNRCalculator getUnTouchedFrames() " +
                    "ret_code is not RET_NO_ERROR, exit directly");
            if (b_log_save) {
                log_manager.onAddErrorMessages("Error: Error occurs " +
                        "before collecting untouched frames, exit");
            }
            return ret_code;
        }
        /* update the warning messages before collecting  */
        onShowUIMessage(getResources().getString(R.string.snr_msg_untouch_collect),false);
        /* reset the progress bar */
        onShowProgress(0);

        if (v.size() != 0)
            v.clear();

        /* start the delta report stream */
        boolean ret = native_lib.onStartReport(native_lib.SYNA_DELTA_REPORT_IMG, false,
                true, false);
        if (!ret) {
            runOnUiThread(new Runnable() {
                public void run(){
                    onShowErrorDialog("Error\n\nFail to enable delta report stream.\n");
                }
            });
            Log.e(SYNA_TAG, "ActivitySNRCalculator collectUnTouchedFrames() " +
                    "fail to enable the syna report" );

            if (b_log_save) {
                log_manager.onAddErrorMessages("Error: Fail to start report image streaming",
                        native_lib);
                /* create a log file with errors only */
                ret = log_manager.onCreateLogFile();
                if (!ret) {
                    Log.e(SYNA_TAG, "ActivitySNRCalculator collectUnTouchedFrames() " +
                            "fail to create the log file" );
                }
            }
            ret_code = RET_FAIL_TO_START;
            return ret_code;
        }

        int frame_idx = 0;
        int total_frames = total_required_frames + (N_FRAMES_WAIT_STABLE*2);
        int latest_frame_idx = total_required_frames + N_FRAMES_WAIT_STABLE;

        /* collect the report images, F_Untouched[n],
         * until reaching the required frames               */
        while (b_running && (frame_idx < total_frames))
        {
            /* allocate a buffer to save image frame */
            int[] data_buf = new int[size];

            /* call native method to retrieve a report image from syna device */
            ret = lib.onRequestReport(report_type, frame_row, frame_column,
                    data_buf, data_buf.length);
            if (!ret) {
                Log.e(SYNA_TAG, "ActivitySNRCalculator collectUnTouchedFrames() " +
                        "fail to collect untouched frames");
                b_running = false;
                if (b_log_save) {
                    log_manager.onAddErrorMessages("Error: Fail to retrieve untouched frames",
                            native_lib);
                }
                ret_code = RET_FAIL_TO_GET_UNTOUCHED_FRAME;
                break;
            }

            /* push one image data into queue only when the frame is stable */
            if ((frame_idx >= N_FRAMES_WAIT_STABLE) && (frame_idx < latest_frame_idx)) {
                v.add(data_buf);
            }


            frame_idx++;
            onShowProgress(frame_idx);
        }

        /* disable the syna report stream */
        ret = native_lib.onStopReport(native_lib.SYNA_DELTA_REPORT_IMG);
        if (!ret) {
            Log.e(SYNA_TAG, "ActivitySNRCalculator collectUnTouchedFrames() " +
                    "fail to disable the syna report" );
            if (b_log_save) {
                log_manager.onAddErrorMessages("Error: Fail to stop report image streaming",
                        native_lib);
            }
        }

        if (v.size() < total_required_frames) {
            Log.e(SYNA_TAG, "ActivitySNRCalculator collectUnTouchedFrames() " +
                    "the number of untouched frames is insufficient");
            if (b_log_save) {
                log_manager.onAddErrorMessages("Error: The number of untouched frames is insufficient" +
                        ", required = " + total_required_frames + ", captured frames = " + v.size());
            }
            ret_code = RET_FAIL_TO_GET_UNTOUCHED_FRAME;
        }

        Log.i(SYNA_TAG, "ActivitySNRCalculator collectUnTouchedFrames() complete");

        return ret_code;
    }
    /********************************************************
     * function to collect the touched frames
     *
     * change the ui message to ask user to put a finger on the panel
     * the loop inside will be triggered when a finger down event occurs
     * next, do report image acquisition, called F_Touched[n]
     ********************************************************/
    private int collectTouchedFrames(NativeWrapper lib, int total_required_frames,
                                 Vector<int[]> v, byte report_type, int size)
    {
        if (ret_code != RET_NO_ERROR) {
            Log.e(SYNA_TAG, "ActivitySNRCalculator collectTouchedFrames() " +
                    "ret_code is not RET_NO_ERROR, exit directly");
            if (b_log_save) {
                log_manager.onAddErrorMessages("Error: Error occurs " +
                        "before the touched frames collection, exit");
            }
            return ret_code;
        }

        /* change the ui message to inform user to put a finger */
        onShowUIMessage(getResources().getString(R.string.snr_msg_to_touch), false);

        /* reset the progress bar */
        onShowProgress(0);

        if (v.size() != 0)
            v.clear();

        /* set the flag true to wait for a touch down event */
        b_wait_touch = true;

        /* busy loop to detect the finger down event */
        /* once finger down, draw a circle on the touched position and update the message */
        while (b_running && b_wait_touch) {
            try {
                Thread.sleep(20);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        /* to enable the delta report stream */
        boolean ret = native_lib.onStartReport(native_lib.SYNA_DELTA_REPORT_IMG, true,
                true, false);
        if (!ret) {
            runOnUiThread(new Runnable() {
                public void run(){
                    onShowErrorDialog("Error\n\nFail to enable delta report stream.\n");
                }
            });
            Log.e(SYNA_TAG, "ActivitySNRCalculator collectTouchedFrames() " +
                    "fail to enable the syna report" );

            if (b_log_save) {
                log_manager.onAddErrorMessages("Error: Fail to start report image streaming",
                        native_lib);
                /* create a log file with errors only */
                ret = log_manager.onCreateLogFile();
                if (!ret) {
                    Log.e(SYNA_TAG, "ActivitySNRCalculator collectTouchedFrames() " +
                            "fail to create the log file" );
                }
            }
            ret_code = RET_FAIL_TO_START;
            return ret_code;
        }

        int frame_idx = 0;
        int total_frames = total_required_frames + (N_FRAMES_WAIT_STABLE*2);
        int latest_frame_idx = total_required_frames + N_FRAMES_WAIT_STABLE;

        /* collect the report images, F_Touched[n],
         * until reaching the required frames               */
        while (b_running && (frame_idx < total_frames)) {
            /* allocate a buffer to save image frame */
            int[] data_buf = new int[size];
            /* call native method to retrieve a report image from syna device */
            ret = lib.onRequestReport(report_type, frame_row, frame_column,
                    data_buf, data_buf.length);
            if (!ret) {
                Log.e(SYNA_TAG, "ActivitySNRCalculator collectTouchedFrames() " +
                        "fail to collect touched frames");
                b_running = false;

                log_manager.onAddErrorMessages("Error: Fail to retrieve touched frames",
                        native_lib);

                ret_code = RET_FAIL_TO_GET_TOUCHED_FRAME;
                break;
            }

            /* push one image data into queue only when the frame is stable */
            if ((frame_idx >= N_FRAMES_WAIT_STABLE) && (frame_idx < latest_frame_idx)) {
                v.add(data_buf);
            }

            frame_idx++;
            onShowProgress(frame_idx);
        }

        if (v.size() < total_required_frames) {
            Log.e(SYNA_TAG, "ActivitySNRCalculator collectTouchedFrames() data is not enough");
            b_running = false;
            if (b_log_save) {
                log_manager.onAddErrorMessages("Error: The number of touched frames is insufficient" +
                        ", required = " + total_required_frames + ", captured frames = " + v.size());
            }
        }

        /* disable the syna report stream */
        ret = native_lib.onStopReport(native_lib.SYNA_DELTA_REPORT_IMG);
        if (!ret) {
            Log.e(SYNA_TAG, "ActivitySNRCalculator collectTouchedFrames() " +
                    "fail to disable the syna report" );
            if (b_log_save) {
                log_manager.onAddErrorMessages("Error: Fail to stop report image streaming",
                        native_lib);
            }
        }

        Log.i(SYNA_TAG, "ActivitySNRCalculator collectTouchedFrames() complete");

        return ret_code;
    }
    /********************************************************
     * function to calculate the strength frame
     *
     * calculate the delta images, each frame comes from
     *          F_Delta[n] = F_Touched[n] - F_Untouched[n]
     *
     * calculate the average of F_Untouched[n]
     *          Untouched_AVG = ( ΣF_Untouched[i] ) / size
     *
     * calculate the average of F_Delta[n] as well
     *          Touched_AVG = ( ΣF_Delta[i] ) / size
     ********************************************************/
    private int calculateStrength(Vector<int[]> v_delta, Vector<int[]> v_untouched,
                                  Vector<int[]> v_touched, int row, int col) {
        int[] buf;

        if (ret_code != RET_NO_ERROR) {
            Log.e(SYNA_TAG, "ActivitySNRCalculator calculateStrength() " +
                    "err_value is not RET_NO_ERROR, do nothing");
            if (b_log_save) {
                log_manager.onAddErrorMessages("Error: Error occurs " +
                        "before the Delta calculation, exit");
            }
            return ret_code;
        }

        if (v_untouched.size() != v_touched.size()) {
            Log.e(SYNA_TAG, "ActivitySNRCalculator calculateStrength() " +
                    "number of frames is mismatching");
            if (b_log_save) {
                log_manager.onAddErrorMessages("Error: The number of touched and untouched frames" +
                        " are mismatching, touched frames = " + v_touched.size() +
                        ", untouched frames = " + v_untouched.size());
            }
            ret_code = RET_SIZE_IS_MISMATCHING;
            return ret_code;
        }

        Log.i(SYNA_TAG, "ActivitySNRCalculator calculateStrength() " +
                "calculate the delta image");

        int size = v_touched.size();

        /* get the delta frames, F_Delta[n] */
        for (int idx = 0; idx < size; idx++) {
            int[] untouched_frame = v_untouched.elementAt(idx);
            int[] touched_frame = v_touched.elementAt(idx);

            /* allocate the buffer for delta image using */
            buf = new int[row * col];

            /* calculate the delta */
            int offset;
            for(int i = 0; i < row; i++) {
                for (int j = 0; j < col; j++) {
                    offset = j * row + i;
                    buf[offset] = (short)(touched_frame[offset] - untouched_frame[offset]);
                }
            }

            /* put one delta image into queue */
            v_delta.add(buf);
        }

        if (result_img_untouched_avg.length != (row * col) ) {
            Log.e(SYNA_TAG, "ActivitySNRCalculator calculateStrength() " +
                    "size of result_img_untouched_avg is mismatching");
            if (b_log_save) {
                log_manager.onAddErrorMessages("Error: The size of Untouched_AVG frame is incorrect" +
                        ", size = " + result_img_untouched_avg.length + ", required = " + (row * col));
            }

            ret_code = RET_FAIL_TO_WRITE_RESULT;
            return ret_code;
        }

        /* calculate the average of untouched frame, Untouched_AVG */
        for(int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                int offset = j * row + i;

                long tmp = 0;
                for (int idx = 0; idx < v_untouched.size(); idx++) {
                    int[] tmp_frame = v_untouched.elementAt(idx);
                    tmp += tmp_frame[offset];
                }

                result_img_untouched_avg[offset] = (double)tmp / (double)v_untouched.size();
            }
        }

        if (result_img_delta_avg.length != (row * col) ) {
            Log.e(SYNA_TAG, "ActivitySNRCalculator calculateStrength() " +
                    "size of result_img_delta_avg is mismatching");
            if (b_log_save) {
                log_manager.onAddErrorMessages("Error: The size of Touched_AVG frame is incorrect" +
                        ", size = " + result_img_delta_avg.length + ", required = " + (row * col));
            }

            ret_code = RET_FAIL_TO_WRITE_RESULT;
            return ret_code;
        }

        Log.i(SYNA_TAG, "ActivitySNRCalculator calculateStrength() " +
                "calculate the Touch AVG");

        /* calculate the average of delta frame, Touched_AVG */
        for(int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                int offset = j * row + i;

                long tmp = 0;
                for (int idx = 0; idx < v_delta.size(); idx++) {
                    int[] delta_frame = v_delta.elementAt(idx);
                    tmp += abs(delta_frame[offset]);
                }

                result_img_delta_avg[offset] = (double)tmp / (double)v_delta.size();
            }
        }

        return ret_code;
    }
    /********************************************************
     * function to calculate the noise RMS frame
     * each pixel is calculated by the following formula
     *          Noise_RMS = sqrt( ( Σ(F_Delta[i] - Touched_AVG)2 ) / size )
     ********************************************************/
    private int calculateNoiseRMS(Vector<int[]> v_delta, double[] delta_avg, int row, int col) {

        if (ret_code != RET_NO_ERROR) {
            Log.e(SYNA_TAG, "ActivitySNRCalculator calculateNoiseRMS() " +
                    "err_value is not RET_NO_ERROR, do nothing");
            if (b_log_save) {
                log_manager.onAddErrorMessages("Error: Error occurs " +
                        "before the Noise RMS calculation, exit");
            }
            return ret_code;
        }

        if (result_img_noise_rms.length != (row * col) ) {
            Log.e(SYNA_TAG, "ActivitySNRCalculator calculateNoiseRMS() " +
                    "size of result_img_noise_rms is mismatching, do nothing");
            if (b_log_save) {
                log_manager.onAddErrorMessages("Error: The size of Noise_RMS frame is incorrect" +
                        ", size = " + result_img_noise_rms.length + ", required = " + (row * col));
            }

            ret_code = RET_FAIL_TO_WRITE_RESULT;
            return ret_code;
        }

        Log.i(SYNA_TAG, "ActivitySNRCalculator calculateStrength() " +
                "calculate the Noise RMS");

        /* calculate the noise RMS frame, Noise_RMS */
        int size = v_delta.size();
        for(int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                int offset = j * row + i;

                double avg = delta_avg[offset];
                double tmp = 0;

                for (int idx = 0; idx < v_delta.size(); idx++) {
                    int[] signal = v_delta.elementAt(idx);

                    tmp += Math.pow((signal[offset] - avg), 2);
                }
                tmp = tmp/(double)size;
                result_img_noise_rms[offset] = Math.sqrt(tmp);
            }
        }

        return ret_code;
    }
    /********************************************************
     * function to calculate the SNR frame
     *
     * SNR = 20 * log10( Touched_AVG / Noise_RMS )
     ********************************************************/
    private int calculateSNR(double[] signal_touch, double[] signal_noise,
                             int row, int col, StringBuilder result_str, int[] pos)
    {

        if (ret_code != RET_NO_ERROR) {
            Log.e(SYNA_TAG, "ActivitySNRCalculator calculateSNR() " +
                    "err_value is not RET_NO_ERROR, do nothing");
            if (b_log_save) {
                log_manager.onAddErrorMessages("Error: Error occurs " +
                        "before the SNR calculation, exit");
            }
            return ret_code;
        }

        if (signal_touch.length != signal_noise.length ) {
            Log.e(SYNA_TAG, "ActivitySNRCalculator calculateSNR() " +
                    "size of result is mismatching, do nothing");
            if (b_log_save) {
                log_manager.onAddErrorMessages("Error: The size of Touched_AVG and Noise_RMS frame" +
                        " are mismatching, Touched_AVG size = " + signal_touch.length +
                        ", Noise_RMS size = " +signal_noise.length);
            }

            ret_code = RET_FAIL_TO_WRITE_RESULT;
            return ret_code;
        }

        Log.i(SYNA_TAG, "ActivitySNRCalculator calculateStrength() " +
                "calculate the SNR");

        result_snr = 0;
        double d;
        int di = 0, dj = 0;

        /* calculate the SNR */
        for(int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                int offset = j * row + i;

                if (signal_noise[offset] == 0) {
                    result_img_snr[offset] = 0;
                }
                else {
                    d = Math.log10(signal_touch[offset]/signal_noise[offset]);
                    if (d <= 0)
                        result_img_snr[offset] = 0;
                    else
                        result_img_snr[offset] = 20 * d;
                }

                if (result_img_snr[offset] > result_snr) {
                    result_snr = result_img_snr[offset];
                    di = i;
                    dj = j;
                }

            }
        }

        /* keep the target pixel */
        pos[0] = di;
        pos[1] = dj;

        /* generate result string */
        result_str.append("SNR                       : ")
                .append(String.format(Locale.getDefault(), "%4.2f db\n\n",result_snr));

        result_str.append("SNR, ")
                .append(String.format(Locale.getDefault(), "the max. value % 6.2f  at (%02d,%02d)",
                        result_snr, di, dj));

        return ret_code;
    }

    /********************************************************
     * helper to write down the log for the SNR calculation
     ********************************************************/
    private void writeLogSNR(String result_str, double[] img_snr, double[] img_delta_avg,
                             double[] img_noise_rms, double[] img_untouched_avg, int frames,
                             int target_pos_i, int target_pos_j, int target_offset,
                             Vector<int[]> v_delta, Vector<int[]> v_touched, Vector<int[]> v_untouched)
    {
        if(!b_log_save) {
            return;
        }

        boolean ret = log_manager.onAddLogData(img_snr, result_str);
        if (!ret) {
            Log.e(SYNA_TAG, "ActivitySNRCalculator writeLogSNR() " +
                    "fail to add the SNR result into log" );
            log_manager.onAddErrorMessages("Error: Fail to add the SNR result into log");
        }
        ret = log_manager.onAddLogData(img_delta_avg,
                "Touched Signal ( = Delta AVG in " + frames + " frames)");
        if (!ret) {
            Log.e(SYNA_TAG, "ActivitySNRCalculator doSNRCalculation() " +
                    "fail to add the average delta into log" );
            log_manager.onAddErrorMessages("Error: Fail to add the average delta into log");
        }
        ret = log_manager.onAddLogData(img_noise_rms,
                "Noise RMS ");
        if (!ret) {
            Log.e(SYNA_TAG, "ActivitySNRCalculator doSNRCalculation() " +
                    "fail to add the Noise RMS into log" );
            log_manager.onAddErrorMessages("Error: Fail to add the Noise RMS into log");
        }
        ret = log_manager.onAddLogData(img_untouched_avg,
                "Baseline AVG  (the average of " + frames + " untouched frames)");
        if (!ret) {
            Log.e(SYNA_TAG, "ActivitySNRCalculator doSNRCalculation() " +
                    "fail to add the average untouched frame into log" );
            log_manager.onAddErrorMessages("Error: Fail to add the average untouched frame into log");
        }

        int[] temp_frame;
        String temp_str;
        int max = -100;
        int min = 1000;
        StringBuilder sb_delta_logs = new StringBuilder();

        for (int i = 0; i < v_delta.size(); i++) {
            temp_frame = v_delta.elementAt(i);
            temp_str = String.format(Locale.getDefault(), "Frame %4d: at (%02d,%02d), Delta value = %5d\n",
                    i+1, target_pos_i, target_pos_j, temp_frame[target_offset]);
            sb_delta_logs.append(temp_str);

            max = Math.max(max, temp_frame[target_offset]);
            min = Math.min(min, temp_frame[target_offset]);
        }

        log_manager.onAddLogData("\n\nFrames Details in summary : \n");

        temp_str = "\nDelta Frames, total " + v_delta.size() + " frames been collected\n" +
                "            Data range = (" + min + " - " + max + ")\n" ;
        sb_delta_logs.insert(0, temp_str);
        log_manager.onAddLogData(sb_delta_logs.toString());


        StringBuilder sb_touched = new StringBuilder();
        max = -100;
        min = 1000;

        for (int i = 0; i < v_touched.size(); i++) {
            temp_frame = v_touched.elementAt(i);
            temp_str = String.format(Locale.getDefault(), "Frame %4d: at (%02d,%02d), data value = %5d\n",
                    i+1, target_pos_i, target_pos_j, temp_frame[target_offset]);
            sb_touched.append(temp_str);

            max = Math.max(max, temp_frame[target_offset]);
            min = Math.min(min, temp_frame[target_offset]);
        }

        temp_str = "\nTouched Frames, total " + v_touched.size() + " frames been collected\n" +
                "            Data range = (" + min + " - " + max + ")\n" ;
        sb_touched.insert(0, temp_str);
        log_manager.onAddLogData(sb_touched.toString());

        StringBuilder sb_untouched = new StringBuilder();
        max = -100;
        min = 1000;

        for (int i = 0; i < v_untouched.size(); i++) {
            temp_frame = v_untouched.elementAt(i);
            temp_str = String.format(Locale.getDefault(), "Frame %4d: at (%02d,%02d), data value = %5d\n",
                    i+1, target_pos_i, target_pos_j, temp_frame[target_offset]);
            sb_untouched.append(temp_str);

            max = Math.max(max, temp_frame[target_offset]);
            min = Math.min(min, temp_frame[target_offset]);
        }

        temp_str = "\nUnTouched Frames, total " + v_untouched.size() + " frames been collected\n" +
                "            Data range = (" + min + " - " + max + ")\n" ;
        sb_untouched.insert(0, temp_str);
        log_manager.onAddLogData(sb_untouched.toString());


        log_manager.onAddLogData("\n\nFrames Details            : \n");

        for (int i = 0; i < v_delta.size(); i++) {
            log_manager.onAddLogData(v_delta.elementAt(i), String.format(Locale.getDefault(),
                    "Delta Frame %4d\n",i+1));
        }
        for (int i = 0; i < v_touched.size(); i++) {
            log_manager.onAddLogData(v_touched.elementAt(i), String.format(Locale.getDefault(),
                    "Touched Frame %4d\n",i+1));
        }
        for (int i = 0; i < v_untouched.size(); i++) {
            log_manager.onAddLogData(v_untouched.elementAt(i), String.format(Locale.getDefault(),
                    "UnTouched Frame %4d\n",i+1));
        }
    }
    /********************************************************
     * function to update the message shown on ui
     ********************************************************/
    String ui_str;
    private void onShowUIMessage(String str, boolean en_msg_2) {
        ui_str = str;
        if (en_msg_2) {
            runOnUiThread(new Runnable() {
                public void run(){
                    text_msg_1.setVisibility(View.INVISIBLE);

                    text_msg_2.setText(ui_str);
                    text_msg_2.setTextColor(color_font);
                    text_msg_2.setVisibility(View.VISIBLE);
                }
            });
        }
        else {
            runOnUiThread(new Runnable() {
                public void run(){
                    text_msg_1.setText(ui_str);
                    text_msg_1.setTextColor(color_font);
                    text_msg_1.setVisibility(View.VISIBLE);

                    text_msg_2.setVisibility(View.INVISIBLE);
                }
            });
        }
    }

    /********************************************************
     * function to update the progress bar
     ********************************************************/
    private void onShowProgress(int progress) {

        if (progress == 0) {
            runOnUiThread(new Runnable() {
                public void run(){
                    progressbar.setProgress(0);
                }
            });
        }
        else {
            runOnUiThread(new Runnable() {
                public void run(){
                    progressbar.incrementProgressBy(1);
                }
            });
        }
    }
    /********************************************************
     * function to draw a circle
     ********************************************************/
    private int draw_x;
    private int draw_y;
    private int draw_radius;
    private int draw_color;
    private void onDrawCircle(int center_x, int center_y) {
        final int RADIUS_CIRCLE = 160;

        draw_x = center_x;
        draw_y = center_y;
        draw_color = 0xFF81bef7;
        draw_radius = RADIUS_CIRCLE;

        /* call drawing_obj to draw a red circle on ui */
        runOnUiThread(new Runnable() {
            public void run(){
                drawing_obj.drawCircle(draw_x, draw_y, draw_radius, draw_color);
            }
        });
    }
    /********************************************************
     * function to clear the drawing view
     ********************************************************/
    private void onDrawClear() {

        /* call drawing_obj to clear */
        runOnUiThread(new Runnable() {
            public void run(){
                drawing_obj.clearDraw();
            }
        });
    }
    /********************************************************
     * function to show a image data in 2D layout to the ui
     ********************************************************/
    private void onShowImage(double[] image, int col_num, int row_num) {
        final int color_positive = 0xffff0000;
        final int color_negative = 0xff0000ff;

        tlayout_img_data.setShrinkAllColumns(true);
        tlayout_img_data.setStretchAllColumns(true);

        String str_value;
        for(int i = 0; i < row_num; i++) {
            TableRow row_data = new TableRow(ActivitySNRCalculator.this);
            for(int j = 0; j < col_num; j++) {
                int offset = j * row_num + i;
                int color = color_font;

                if (image[offset] > 10) color = color_positive;
                if (image[offset] < -10) color = color_negative;

                if (image[offset] >= 1000)
                    str_value = String.format(Locale.getDefault(),"%4.1f", image[offset]);
                else
                    str_value = String.format(Locale.getDefault(),"%3.2f", image[offset]);

                onAddTextToTableRow(row_data, str_value, color, row_num, col_num);
            }
            tlayout_img_data.addView(row_data);
        }

    } /* end showReportData() */
    /********************************************************
     * add text to one Table Row
     ********************************************************/
    private void onAddTextToTableRow(TableRow row, String str, int color,
                                     int row_num, int col_num) {
        int FONT_SIZE = 6;

        TextView text = new TextView(ActivitySNRCalculator.this);
        text.setText(str);
        text.setGravity(Gravity.CENTER);
        text.setTextColor(color);
        if (b_color_white)
            text.setBackgroundResource(R.drawable.text_view_border_white);
        else
            text.setBackgroundResource(R.drawable.text_view_border);
        text.setWidth(display_width/col_num);
        text.setHeight(display_height/row_num);
        text.setTextSize(FONT_SIZE);
        /* add text to table row */
        row.addView(text);
    } /* end addTextToTableRow() */
    /********************************************************
     * function to update the result string
     ********************************************************/
    private void onShowSnrResult() {
        runOnUiThread(new Runnable() {
            public void run(){
                if (ret_code == RET_NO_ERROR) {
                    text_snr_result.setTextColor(Color.WHITE);
                    text_snr_result.setTextSize(50);
                    text_snr_result.setText(String.format(Locale.getDefault(),"%-3.2f",
                            result_snr));
                }
                else {  /* show error message as the result */
                    text_snr_result.setTextColor(Color.RED);
                    text_snr_result.setTextSize(12);
                    text_snr_result.setText(onGetResultString(ret_code));
                }
            }
        });
    }
    private String onGetResultString(int type) {
        switch (type) {
            case RET_FAIL_TO_START:
                return "Fail to start SNR calculation";
            case RET_FAIL_TO_GET_UNTOUCHED_FRAME:
                return "Fail to collect untouched frames";
            case RET_FAIL_TO_GET_TOUCHED_FRAME:
                return "Fail to collect touched frames";
            case RET_SIZE_IS_MISMATCHING:
                return "The size of collected frames is incorrect";
            case RET_FAIL_TO_WRITE_RESULT:
                return "Unable to generate the SNR result";
            case RET_NO_ERROR:
            default:
                return null;
        }
    }

    private final int RET_NO_ERROR = 0;
    private final int RET_FAIL_TO_START = 1;
    private final int RET_FAIL_TO_GET_UNTOUCHED_FRAME = 2;
    private final int RET_FAIL_TO_GET_TOUCHED_FRAME = 3;
    private final int RET_FAIL_TO_WRITE_RESULT = 4;
    private final int RET_SIZE_IS_MISMATCHING = 5;


    /********************************************************
     * function to save preferences
     ********************************************************/
    public void onSavePreferences() {
        Log.i(SYNA_TAG, "ActivitySNRCalculator savePreferences() + " );
        SharedPreferences preference = getSharedPreferences("SynaAPKPreference_SNRCal",
                Context.MODE_PRIVATE);
        preference.edit()
                .putInt(STR_CFG_SNR_FRAMES, required_frames)
                .putBoolean(STR_CFG_BACKGROUND, b_color_white)
                .apply();
    }
    /********************************************************
     * function to load preferences
     ********************************************************/
    public void onLoadPreferences() {
        Log.i(SYNA_TAG, "ActivitySNRCalculator loadPreferences() + " );
        SharedPreferences preference = getSharedPreferences("SynaAPKPreference_SNRCal",
                Context.MODE_PRIVATE);
        required_frames = preference.getInt(STR_CFG_SNR_FRAMES, 100);
        b_color_white = preference.getBoolean(STR_CFG_BACKGROUND, true);
    }

    /********************************************************
     * function to popup toast message
     ********************************************************/
    private void onShowToastMsg(String msg) {
        Toast toast = Toast.makeText(ActivitySNRCalculator.this,
                msg, Toast.LENGTH_LONG);
        toast.setGravity(Gravity.CENTER_HORIZONTAL | Gravity.CENTER, 0, 0);
        toast.show();
    } /* end onShowToastMsg() */

    /********************************************************
     * create a dialog with message
     *******************************************************/
    private void onShowErrorDialog(String msg) {
        new AlertDialog.Builder(ActivitySNRCalculator.this)
                .setMessage(msg)
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        llayout_control_panel.setVisibility(View.VISIBLE);
                        text_msg_to_stop.setVisibility(View.INVISIBLE);
                    }
                }).show();
    } /* end onShowErrorDialog() */

    /********************************************************
     * create a dialog with message
     * once clicking 'OK' button, go back to the main page
     *******************************************************/
    private void onShowErrorOut(String msg) {
        new AlertDialog.Builder(ActivitySNRCalculator.this)
                .setMessage(msg)
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        Intent intent = new Intent();
                        intent.setClass(ActivitySNRCalculator.this, MainActivity.class);
                        startActivity(intent);
                        ActivitySNRCalculator.this.finish();
                    }
                }).show();
    } /* end onShowErrorOut() */


    private final String STR_CFG_SNR_FRAMES = "CFG_SNR_FRAMES";
    private final String STR_CFG_BACKGROUND = "CFG_BACKGROUND_WHITE";

}
