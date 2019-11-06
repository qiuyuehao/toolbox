package com.vivotouchscreen.synadeltadiff;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.Spinner;
import android.widget.TextView;

import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.Vector;

public class SynaBspTest extends AppCompatActivity {

    /**
     * Global variables
     */
    private final String SYNA_TAG = "syna-apk";

    private static final String VIVO_FOLDER = "/sdcard/Android/data/com.touchscreen.tptest/";

    private String versionAPK;
    private int FRAMES_PER_SECOND = 15;

    private boolean is_initialized;
    private int report_type;
    private int delta_threshold;
    private int delta_threshold_numerator;
    private int delta_threshold_denominator;
    private int record_time;
    private  String path_file;
    private Vector<Integer> gears = new Vector<>();
    private int test_frames_total;
    private int test_frames_current;
    private String test_gear_current;
    private int test_failure_cnt;
    private boolean is_testing;
    private StringBuilder test_result_detail;
    private String asic_type;
    private String firmware_id;
    private boolean is_tddi_rmi;

    /**
     * UI Components
     */
    private Button BtnStart;
    private Spinner SpinnerRT;
    private EditText EditThreshold;
    private EditText EditRecordTime;
    private EditText EditPathFile;
    private ProgressBar Progress;
    private TextView TextAsic;
    private TextView TextFwID;
    private TextView TextFrames_testing;
    private TextView TextGears_testing;
    private TextView TextResult;
    private TextView TextResult_detail;
    private TextView TextErrCnt;
    private TextView TextFrames_total;
    private TextView TextFrames_gear;
    private TextView TextFile;

    private static final String[] REORT_TYPE_LIST = {
            "Delta Image",    // position = 0
            "RT 94 Image",    // position = 1
    };

    Handler handler = new Handler();

    /**
     * Object for native functions handling
     */
    NativeWrapper native_lib;


    /**
     * Called when the activity is first created.
     * This is where developer should do all of normal static set up: create views, bind data to
     * lists, etc.
     * This method also provides developer with a Bundle containing the activity's previously
     * frozen state, if there was one.
     * Always followed by onStart().
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // keep screen always on
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        setContentView(R.layout.activity_main);
        Toolbar toolbar = findViewById(R.id.toolbar);
        TextView toolbar_title = findViewById(R.id.toolbar_title);

        Log.i(SYNA_TAG, "onCreate() + " );

        /* get the APP version */
        try {
            PackageInfo pInfo = this.getPackageManager().getPackageInfo(getPackageName(), 0);
            versionAPK = pInfo.versionName;
        }
        catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }
        String title = "SynaBspTest v" + versionAPK;
        toolbar_title.setText(title);
        setSupportActionBar(toolbar);

        /* UI components declaration */
        BtnStart = findViewById(R.id.button_start);
        SpinnerRT = findViewById(R.id.spinner_rt);
        EditThreshold = findViewById(R.id.edit_threshold);
        EditRecordTime = findViewById(R.id.edit_recordtime);
        EditPathFile = findViewById(R.id.edit_pathlog);
        Progress = findViewById(R.id.progress_bar);
        TextAsic = findViewById(R.id.text_asic);
        TextFwID = findViewById(R.id.text_fw_id);
        TextFrames_testing = findViewById(R.id.text_frames);
        TextGears_testing = findViewById(R.id.text_current_gear);
        TextResult = findViewById(R.id.text_result);
        TextErrCnt = findViewById(R.id.text_fail_count);
        TextResult_detail = findViewById(R.id.text_result_detail);
        TextFrames_total = findViewById(R.id.text_total_frames);
        TextFrames_gear = findViewById(R.id.text_frame_gear);
        TextFile = findViewById(R.id.text_file_name);

        BtnStart.setEnabled(false);
        SpinnerRT.setEnabled(false);
        EditThreshold.setEnabled(false);
        EditRecordTime.setEnabled(false);
        EditPathFile.setEnabled(false);

        native_lib = new NativeWrapper();

        /* load the preferences */
        loadPreferences();

        if (!native_lib.onCreateVIVOFolder()) {
            showMessage("Could not create the folder \n\n" + VIVO_FOLDER);
            is_initialized = false;
            return;
        }

        /* check the syna touch device */
        boolean ret = native_lib.onCheckDev();
        if (!ret) {
            showMessage("Could not find the proper /dev/rmi or /dev/tcm node in the file system.\n\n" +
                    "Please use ADB interface to confirm the followings: \n" +
                    "1. The synaptics touch driver is installed.\n" +
                    "2. The permission of /dev/rmi or /dev/tcm file is proper.\n");
            is_initialized = false;
            return;
        }

        /* to get the asic and build information */
        asic_type = native_lib.getDevID();
        firmware_id = native_lib.getFirmwareID();
        is_tddi_rmi = ((asic_type.charAt(0) == '4') && native_lib.isRMIDev() );

        /* to indicate the /dev/rmi device exists */
        is_initialized = true;

        /* UI listener initialization */
        BtnStart.setOnClickListener(_button_listener_start);
        BtnStart.setEnabled(true);

        List all_type = new ArrayList();
        all_type.add(REORT_TYPE_LIST[0]);
        if (is_tddi_rmi) {  /* add RT94 only if it is tddi product on rmi interface */
            all_type.add(REORT_TYPE_LIST[1]);
        }
        ArrayAdapter<String> typeList = new ArrayAdapter<>(this, R.layout.spinner_item, all_type);
        typeList.setDropDownViewResource(android.R.layout.select_dialog_singlechoice);
        SpinnerRT.setAdapter(typeList);
        SpinnerRT.setOnItemSelectedListener(_spinner_listener);
        SpinnerRT.setEnabled(true);

        EditThreshold.setOnEditorActionListener(_edit_listener_threshold);
        EditThreshold.setEnabled(true);
        EditRecordTime.setOnEditorActionListener(_edit_listener_recordtime);
        EditRecordTime.setEnabled(true);
        EditPathFile.setOnEditorActionListener(_edit_listener_path);
        EditPathFile.setEnabled(true);


        /* Ability to start from command line */
        String idata;
        Bundle extras = getIntent().getExtras();
        if(extras != null) {
            Log.i(SYNA_TAG, "onCreate() getIntent data");

            idata = extras.getString("FRAMES_SEC");
            if (idata != null) {
                FRAMES_PER_SECOND = Integer.parseInt(idata);

                Log.i(SYNA_TAG, "getIntent FRAMES_SEC, value = " + FRAMES_PER_SECOND);
            }

            idata = extras.getString("LOG_PATH");
            if (idata != null) {
                path_file = idata;

                Log.i(SYNA_TAG, "getIntent LOG_PATH, path = " + path_file);
            }

            int temp_time = extras.getInt("time", -1);
            if (temp_time != -1) {
                record_time = temp_time;
                Log.i(SYNA_TAG, "getIntent time, value = " + record_time);
            }

            int temp_numerator = extras.getInt("threshold_numerator", -1);
            if (temp_numerator != -1) {
                delta_threshold_numerator = temp_numerator;
                Log.i(SYNA_TAG, "getIntent threshold_numerator, value = " + delta_threshold_numerator);
            }

            int temp_denominator = extras.getInt("threshold_denominator", -1);
            if (temp_denominator != -1) {
                delta_threshold_denominator = temp_denominator;
                Log.i(SYNA_TAG, "getIntent threshold_denominator, value = " + delta_threshold_denominator);
            }

        }


    }
    /* end onCreate() */

    /**
     * Called after onCreate(Bundle)
     * (1) Check whether the rmi character device node exists or not
     * (2) Initialize the RMI device and collect the DUT information
     */
    @Override
    public void onStart() {
        Log.i(SYNA_TAG, "onStart() + " );
        int finger_delta;

        /* update the initial UI content  */
        if (is_initialized) {
            TextAsic.setText(asic_type);
            TextFwID.setText(firmware_id);

            if(!(path_file.substring(path_file.length()-1).equals("/"))) {
                path_file = path_file + "/";
            }
            EditPathFile.setText(path_file);
            EditPathFile.clearFocus();

            finger_delta = native_lib.getThreshold();
            Log.i(SYNA_TAG, "onStart() : finger_delta = " + finger_delta);

            finger_delta = finger_delta * delta_threshold_numerator / delta_threshold_denominator;
            if (delta_threshold != finger_delta) {
                delta_threshold = finger_delta;
            }
            Log.i(SYNA_TAG, "onStart() : delta_threshold = " + delta_threshold +
                    "  (numerator, denominator) = (" + delta_threshold_numerator + " , " +
                    delta_threshold_denominator +")");


            EditThreshold.setText(String.valueOf(delta_threshold));
            EditThreshold.clearFocus();
            native_lib.setThreshold(delta_threshold); /* save to the native method as default */

            EditRecordTime.setText(String.valueOf(record_time));
            EditRecordTime.clearFocus();

            Log.i(SYNA_TAG, "onStart() : report_type = " + report_type);
            if (report_type == 94) {
                if (is_tddi_rmi) {
                    SpinnerRT.setSelection(1);
                }
                else {
                    report_type = 2;
                    SpinnerRT.setSelection(0);
                }

            }
            else if (report_type == 2) {
                SpinnerRT.setSelection(0);
            }

            /* get available gear information */
            parseGearInfo(native_lib.getGearInfo());
            /* calculate the estimated total frames */
            /* there are around 10 report images can be captured in 1 second */
            /* therefore, the number of total frames = recording time x 10 x total available gears */
            test_frames_total = record_time * FRAMES_PER_SECOND * gears.size();
            TextFrames_total.setText(String.valueOf(test_frames_total));
            TextFrames_gear.setText(String.valueOf(test_frames_total/gears.size()));

            handler.postDelayed(new Runnable(){
                @Override
                public void run() {
                    BtnStart.performClick();
                }}, 1000);
        }
        else {
            Log.e(SYNA_TAG, "UI is not initialized yet " );
        }

        super.onStart();
    }
    /* end onStart() */

    /**
     * Called when this application is no longer visible to the user.
     */
    @Override
    public void onStop() {
        Log.i(SYNA_TAG, "onStop() + " );

        if (is_testing) {
            is_testing = false;

            handler.postDelayed(new Runnable(){
                @Override
                public void run() {
                    deleteLogFile();
                }}, 5000);

            this.finish();
        }

        savePreferences(); /* save preferences before closing the application*/

        super.onStop();
    }
    /* end onStop() */

    /**
     * The final call you receive before your activity is destroyed.
     * This can happen either because the activity is finishing on it
     */
    @Override
    public void onDestroy() {
        Log.i(SYNA_TAG, "onDestroy() + " );
        super.onDestroy();
    }
    /* end onDestroy() */

    /**
     * function to listen the KeyEvent
     */
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {

        switch(keyCode) {

            case KeyEvent.KEYCODE_VOLUME_UP:
                if  (event.getRepeatCount() ==  0 ) {
                    event.startTracking();

                    Log.i(SYNA_TAG, "onKeyDown() + KEYCODE_VOLUME_UP" );
                    /* stop the testing */
                    is_testing = false;

                    handler.postDelayed(new Runnable(){
                        @Override
                        public void run() {
                            deleteLogFile();
                            SynaBspTest.this.finish();
                        }}, 5000);
                }
                return true ;

            case KeyEvent.KEYCODE_VOLUME_DOWN:
                if  (event.getRepeatCount() ==  0 ) {
                    event.startTracking();

                    Log.i(SYNA_TAG, "onKeyDown() + KEYCODE_VOLUME_DOWN" );
                    /* start the testing */
                    if (is_testing) {
                        return true;
                    }
                    /* disable the button here */
                    BtnStart.setEnabled(false);
                    BtnStart.setText(R.string.btn_str_running);
                    /* disable all UI input */
                    SpinnerRT.setEnabled(false);
                    EditThreshold.setEnabled(false);
                    EditRecordTime.setEnabled(false);
                    EditPathFile.setEnabled(false);
                    /* generate the name of log file with timestamp and */
                    /* save the name to the native method */
                    setLogFile();
                    /* reset UI to start the testing */
                    is_testing = true;
                    test_frames_current = 0;
                    test_failure_cnt = 0;
                    TextFrames_testing.setText(String.valueOf(0));
                    TextResult.setText(R.string.testing);
                    TextResult.setTextColor(Color.WHITE);
                    TextErrCnt.setText(String.valueOf(0));
                    TextResult_detail.setText("");
                    /* doTesting on another thread */

                    handler.postDelayed(new Runnable(){
                        @Override
                        public void run() {
                            doTesting();
                        }}, 2000);
                }
                return true ;

        }
        return super.onKeyDown(keyCode, event);
    }
    /* end onKeyDown() */

    /**
     * auto-created by wizard
     * onCreateOptionsMenu method is responsible for creating a menu.
     * Menu object is passed to this method as a parameter.
     */
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }  /* end onCreateOptionsMenu() */

    /**
     * onOptionsItemSelected method is an activity listener.
     * It receives menu item which has been clicked as a parameter - MenuItem.
     */
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();

        if (id == R.id.action_about) {
            showMessage("APK Version : " + versionAPK +
                    "\n\nThis is a special version to perform noise testing on the phone level.");
            return true;
        }

        return super.onOptionsItemSelected(item);
    }  /* end onOptionsItemSelected() */

    /**
     * helper function to output the Toast message
     */
    private void showMessage(String msg) {
        AlertDialog.Builder dlg  = new AlertDialog.Builder(this);
        dlg.setMessage(msg);
        dlg.setPositiveButton("OK", null);
        dlg.create().show();
    }

    /**
     * implementation if START button is pressed
     *
     * (1) set the log file name
     * (2) calculate the estimated total frames
     * (3) start the testing
     */
    private View.OnClickListener _button_listener_start =
            new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    Log.i(SYNA_TAG, "onClick() + BtnStart " + is_testing );
                    if (is_testing) {
                        return;
                    }

                    /* disable the button here */
                    BtnStart.setEnabled(false);
                    BtnStart.setText(R.string.btn_str_running);
                    /* disable all UI input */
                    SpinnerRT.setEnabled(false);
                    EditThreshold.setEnabled(false);
                    EditRecordTime.setEnabled(false);
                    EditPathFile.setEnabled(false);
                    /* generate the name of log file with timestamp and */
                    /* save the name to the native method */
                    setLogFile();
                    /* reset UI to start the testing */
                    is_testing = true;
                    test_frames_current = 0;
                    test_failure_cnt = 0;
                    TextFrames_testing.setText(String.valueOf(0));
                    TextResult.setText(R.string.testing);
                    TextResult.setTextColor(Color.WHITE);
                    TextErrCnt.setText(String.valueOf(0));
                    TextResult_detail.setText("");
                    /* doTesting on another thread */
                    doTesting();
                }
            }; /* end _button_listener_start */

    /**
     * implementation if the specified item in spinner is selected
     *
     * (1) save the report type, and update the setting into jni method
     */
    private AdapterView.OnItemSelectedListener _spinner_listener =
            new AdapterView.OnItemSelectedListener() {
                @Override
                public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                    switch(position) {
                        case 0:
                            report_type = 2;
                            break;
                        case 1:
                            report_type = 94;
                            break;
                    }
                    /* call native function to save the report type */
                    native_lib.setReportType(report_type);
                }

                @Override
                public void onNothingSelected(AdapterView<?> parent) {}
            }; /* end _spinner_listener */

    /**
     * implementation if the content of threshold is changed
     *
     * (1) save the threshold, and update the setting into jni method
     */
    private TextView.OnEditorActionListener _edit_listener_threshold =
            new TextView.OnEditorActionListener() {
                @Override
                public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                    if (actionId == EditorInfo.IME_ACTION_DONE) {
                        EditThreshold.clearFocus();
                        InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
                        assert imm != null;
                        imm.hideSoftInputFromWindow(EditThreshold.getWindowToken(), 0);

                        String temp = EditThreshold.getText().toString();
                        delta_threshold = Integer.parseInt(temp);

                        /* call native function to save the setting of delta threshold */
                        native_lib.setThreshold(delta_threshold);

                        return true;
                    }
                    return false;
                }
            }; /* end _edit_listener_threshold */
    /**
     * implementation if the content of record time is changed
     *
     * (1) calculate the number of frame to be captured
     *     around 11 frames in 1 seconds
     */
    private TextView.OnEditorActionListener _edit_listener_recordtime =
            new TextView.OnEditorActionListener() {
                @Override
                public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                    if (actionId == EditorInfo.IME_ACTION_DONE) {
                        EditRecordTime.clearFocus();
                        InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
                        assert imm != null;
                        imm.hideSoftInputFromWindow(EditRecordTime.getWindowToken(), 0);

                        String temp = EditRecordTime.getText().toString();
                        record_time = Integer.parseInt(temp);
                        /* update the estimated total frames */
                        /* there are around 10 report images can be captured in 1 second */
                        /* therefore, the number of total frames = recording time x 10 x total available gears */
                        test_frames_total = record_time * FRAMES_PER_SECOND * gears.size();

                        TextFrames_total.setText(String.valueOf(test_frames_total));
                        TextFrames_gear.setText(String.valueOf(test_frames_total/gears.size()));
                        return true;
                    }
                    return false;
                }
            }; /* end _edit_listener_recordtime */
    /**
     * implementation if the path of log file is changed
     *
     * (1) save the path of log file
     */
    private TextView.OnEditorActionListener _edit_listener_path =
            new TextView.OnEditorActionListener() {
                @Override
                public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                    if (actionId == EditorInfo.IME_ACTION_DONE) {
                        EditPathFile.clearFocus();
                        InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
                        assert imm != null;
                        imm.hideSoftInputFromWindow(EditPathFile.getWindowToken(), 0);

                        /* normalize the format, the latest character should be '/'  */
                        path_file = EditPathFile.getText().toString();
                        path_file = removeTheBlankOfString(path_file);
                        if(!(path_file.substring(path_file.length()-1).equals("/"))){
                            path_file = path_file + "/";
                            EditPathFile.setText(path_file, TextView.BufferType.EDITABLE);
                        }
                        return true;
                    }
                    return false;
                }
            }; /* end _edit_listener_path */
    /**
     * helper function remove all blank in the string
     */
    private String removeTheBlankOfString(String str) {
        if(str == null||str.equals(""))
            return null;

        int endPosition = str.length()-1;
        for (int i = str.length()-1; i >= 0; i--) {
            if(str.charAt(i)==' ')endPosition--;
            else break;
        }
        return str.substring(0,endPosition+1);
    }
    /**
     * to parse all available gear and make them structure
     */
    private void parseGearInfo(String info) {
        TextView TextAllGears = findViewById(R.id.text_all_gears);
        StringBuilder ui_info = new StringBuilder();
        int idx_end;
        int idx_start = 0;

        if (!gears.isEmpty()) {
            gears.clear();
        }

        for (int i = 0; i < info.length(); i++) {
            if (info.charAt(i) == ' ')
                idx_start = i;

            if (info.charAt(i) == '\n') {
                idx_end = i ;
                gears.add(Integer.parseInt(info.substring(idx_start+1, idx_end)));
            }
        }
        Log.i(SYNA_TAG, "parseGearInfo() + number of gear enabled = " + gears.size());
        for (int i = 0; i < gears.size(); i++) {
                ui_info.append((i+1)).append(".  Gear-").append(gears.get(i));
            if (i+1 != gears.size())
            	ui_info.append("\n");
        }
        /* update information to UI */
        TextAllGears.setText(ui_info.toString());
    }
    /**
     * helper function generate the name of log file
     * and set it to the native method
     */
    private void setLogFile() {
        Long ts = System.currentTimeMillis()/1000;
        String file_name = "noisetest_" + ts.toString();
        /* set the file name to the native method */
        native_lib.setOutputFile(path_file + file_name);
        /* show the file name on UI */
        TextFile.setText(file_name);
    }
    /**
     * helper function to delete the specified file
     */
    private void deleteLogFile() {
        String filename = TextFile.getText().toString();
        Log.i(SYNA_TAG, "deleteLogFile()) + file name = " + filename);

        File log_file = new File(path_file + filename + "_all.csv");
        if (log_file.exists()) {
            if(log_file.delete()){
                Log.i(SYNA_TAG, "deleteLogFile()) + file " + path_file + filename + "_all.csv is deleted.");
            }
        }

        File fail_log_file = new File(path_file + filename + "_fail.csv");
        if (fail_log_file.exists()) {
            if(fail_log_file.delete()){
                Log.i(SYNA_TAG, "deleteLogFile()) + file " + path_file + filename + "_fail.csv is deleted.");
            }
        }
    }

    /**
     * function to perform the noise testing
     *
     * (1) create a thread to perform noise testing
     * (1) enable one specified gear
     * (2) get the amount of delta images. if any Tixel > Threshold, it means testing failure
     * (3) loop (2) and (3) until all available gears finished the test
     */
    public void doTesting() {
        Log.i(SYNA_TAG, "doTesting() + " );
        /*  create a thread to perform the testing */
        new Thread(new Runnable() {
            public void run() {
                boolean ret;
                int gear;
                int frames;
                int max_frames_per_gear = test_frames_total/gears.size();
                int err_per_gear;

                Progress.setMax(test_frames_total);
                Progress.setProgress(0);

                test_result_detail = new StringBuilder(); /* reset the string */

                /* open the syna /dev and do preparation  */
                ret = native_lib.onOpenTest(test_frames_total, gears.size());
                if (!ret) {
                    native_lib.onCloseTest(0); /* close the syna /dev  */
                    runOnUiThread(new Runnable() {
                        public void run(){
                            BtnStart.setEnabled(true);
                            BtnStart.setText(R.string.btn_str_running);
                            SpinnerRT.setEnabled(true);
                            EditThreshold.setEnabled(true);
                            EditRecordTime.setEnabled(true);
                            EditPathFile.setEnabled(true);
                            TextResult.setText(" ");
                            TextFile.setText(" ");

                            showMessage("Unable to start the testing because the log file can't be created.\n" +
                                    "Please check the followings\n" +
                                    "1. the input path is accessible.\n" +
                                    "2. the access permission is correct.");
                        }
                    });
                    is_testing = false;
                    return;
                }

                /* walk through all available gears, and enable each one by calling the native method */
                for (int i = 0; i < gears.size(); i++) {
                    gear = gears.get(i);
                    test_gear_current = "Gear-" + gear;

                    Log.i(SYNA_TAG, "frequency selection: gear- " + gear);

                    if ( i != 0)
                        test_result_detail.append("\n");

                    test_result_detail.append((i+1)).append(".  Gear-").append(gear).append(" : ");

                    /* update UI, show current gear selection */
                    runOnUiThread(new Runnable() {
                        public void run(){
                            TextGears_testing.setText(test_gear_current);
                        }
                    });

                    /* change to specified gear */
                    ret = native_lib.setGear(gear);
                    if (!ret) {
                        test_result_detail.append("Fail to change gear\n") ;
                        test_failure_cnt += test_frames_total/gears.size();
                        /* update UI, show error information */
                        /* skip all testing in that gear */
                        runOnUiThread(new Runnable() {
                            public void run(){
                                Progress.incrementProgressBy(test_frames_total/gears.size());
                                TextErrCnt.setText(String.valueOf(test_failure_cnt));
                                TextResult_detail.setText(test_result_detail);
                            }
                        });
                        continue;
                    }

                    /* do noise testing via the native function */
                    frames = 0;
                    err_per_gear = 0;
                    while((is_testing) && (frames < max_frames_per_gear)) {
                        /* do noise testing */
                        test_frames_current += 1;
                        ret = native_lib.onNoiseTest(test_frames_current, gear);
                        if (!ret) {
                            test_failure_cnt += 1;
                            err_per_gear += 1;
                        }
                        /* update UI, show the testing progress and status */
                        runOnUiThread(new Runnable() {
                            public void run(){
                                TextFrames_testing.setText(String.valueOf(test_frames_current));
                                Progress.incrementProgressBy(1); /* update progress */
                                TextErrCnt.setText(String.valueOf(test_failure_cnt));
                            }
                        });

                        frames += 1;
                    }

                    if (err_per_gear != 0) {
                        test_result_detail.append("Fail (error: ").append(err_per_gear).append(")");
                    }
                    else if ((!is_testing) && (frames < max_frames_per_gear)) {
                        test_result_detail.append("Terminated");
                    }
                    else {
                        test_result_detail.append("Pass");
                    }
                    /* update test detail for each gear */
                    runOnUiThread(new Runnable() {
                        public void run(){
                            TextResult_detail.setText(test_result_detail.toString());
                        }
                    });
                }


                /* close the syna /dev */
                native_lib.onCloseTest(test_failure_cnt);

                /* recover UI, enable the start button */
                runOnUiThread(new Runnable() {
                    public void run(){
                        BtnStart.setEnabled(true);
                        BtnStart.setText(R.string.btn_str_default);
                        SpinnerRT.setEnabled(true);
                        EditThreshold.setEnabled(true);
                        EditRecordTime.setEnabled(true);
                        EditPathFile.setEnabled(true);

                        /* update final testing result */
                        if(test_failure_cnt != 0) {
                            TextResult.setText(R.string.testing_fail);
                            TextResult.setTextColor(Color.RED);
                        }
                        else if (test_frames_current != test_frames_total) {
                            TextResult.setText(R.string.testing_fail);
                            TextResult.setTextColor(Color.RED);
                            showMessage("Test is terminated.");
                        }
                        else {
                            TextResult.setText(R.string.testing_pass);
                            TextResult.setTextColor(Color.GREEN);
                        }
                    }
                });
                /* set flag to false */
                is_testing = false;

                broadcastResult();

            }
        }).start(); /* start the Thread */
    }

    /**
     * function to save preferences
     */
    public void savePreferences() {
        Log.i(SYNA_TAG, "savePreferences() + " );

        SharedPreferences perferences = getSharedPreferences("perferences", Context.MODE_PRIVATE);
        perferences.edit()
                .putInt("report_type", report_type)
                .putString("path_file", path_file)
                .putInt("record_time", record_time)
                .putInt("threshold_numerator", delta_threshold_numerator)
                .putInt("threshold_denominator", delta_threshold_denominator)
                .putInt("FRAMES_PER_SECOND", FRAMES_PER_SECOND)
                .apply();
    }
    /**
     * function to load preferences and update the UI settings as well
     */
    public void loadPreferences() {
        Log.i(SYNA_TAG, "loadPreferences() + " );

        SharedPreferences perferences = getSharedPreferences("perferences", Context.MODE_PRIVATE);

        /* update the default parameter, report type */
        report_type = perferences.getInt("report_type", 2);

        /* update the default parameter, path of log file */
        path_file = perferences.getString("path_file", VIVO_FOLDER);

        /* update the default parameter, record time */
        record_time = perferences.getInt("record_time", 600);

        /* update the default parameter, report type */
        delta_threshold_numerator = perferences.getInt("threshold_numerator", 3);
        delta_threshold_denominator = perferences.getInt("threshold_denominator", 4);

        /* update the default parameter, number of frames per second */
        FRAMES_PER_SECOND = perferences.getInt("FRAMES_PER_SECOND", 15);
    }
    /********************************************************
     * function to broadcast testing result
     ********************************************************/
    private void broadcastResult() {
        Log.i(SYNA_TAG, "broadcastResult() + " );

        String result;
        Intent intent = new Intent();
        intent.setAction("vivotouchscreen.SynaBspTest");
        if (test_failure_cnt == 0) {
            result = "0";  // pass
            Log.i(SYNA_TAG, "broadcastResult() pass " );
        }
        else {
            result = "1";  // failure
            Log.i(SYNA_TAG, "broadcastResult() failure (" + test_failure_cnt + ")" );
        }
        intent.putExtra("vivotouchscreen.SynaBspTest", result);
        sendBroadcast(intent);

        handler.postDelayed(new Runnable(){
            @Override
            public void run() {
                SynaBspTest.this.finish();
            }}, 1000);
    }
}
