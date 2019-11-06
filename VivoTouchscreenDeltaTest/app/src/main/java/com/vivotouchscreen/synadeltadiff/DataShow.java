package com.vivotouchscreen.synadeltadiff;

import android.graphics.Color;
import android.graphics.Point;
import android.os.Bundle;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.Display;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.LinearLayout;
import android.widget.TableLayout;
import android.widget.TableRow;
import android.widget.TextView;

import java.util.Locale;
import java.util.Vector;

public class DataShow extends AppCompatActivity {

    /**
     * Global variables
     */
    private final String SYNA_TAG = "syna-apk";

    /**
     * UI Components
     */
    private TableLayout table_layout;
    private Button btn_img_delta;
    private Button btn_img_raw;
    private TextView text_statistics;
    private TextView text_msg_to_stop;
    private CheckBox cbtn_show_statistics;
    private LinearLayout layout_buttons;
    private LinearLayout layout_statistics;
    private CheckBox cbtn_show_rotated;

    /**
     * Object for native functions handling
     */
    NativeWrapper native_lib;

    private int display_width;
    private int display_height;

    private Vector<short[]> vector_image_frames = new Vector<>();
    private boolean b_running;
    private boolean b_show_statistics;
    private int finger_threshold;
    private boolean b_show_rotated;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        /* keep screen always on */
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
                WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        /* to hide the navigation bar */
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);

        setContentView(R.layout.content_datashow);

        Log.i(SYNA_TAG, "onCreate() + " );

        /* get display resolution */
        Display display = getWindowManager().getDefaultDisplay();
        Point size = new Point();
        display.getSize(size);
        display_width = size.x;
        display_height = size.y;
        Log.i(SYNA_TAG, "ActivityImageLogger: display (width,height) = " +
                "(" + display_width + " , " + display_height + ")" );

        b_show_statistics = true;
        b_show_rotated = false;

        /* create an object to handle native methods calling */
        native_lib = new NativeWrapper();

        /* UI components - linear layout */
        layout_statistics = findViewById(R.id.layout_report_info);
        layout_statistics.setVisibility(View.INVISIBLE);
        layout_buttons = findViewById(R.id.layout_report_buttons);
        /* UI components - table layout */
        table_layout = findViewById(R.id.table_img_image_data);
        /* UI components - texts od image logger  */
        text_statistics = findViewById(R.id.text_img_statistics);
        text_msg_to_stop = findViewById(R.id.text_img_msg_to_stop);
        text_msg_to_stop.setVisibility(View.INVISIBLE);
        /* UI components - buttons */
        btn_img_delta = findViewById(R.id.btn_img_delta);
        btn_img_delta.setOnClickListener(_button_listener_delta);
        btn_img_delta.setEnabled(false);
        btn_img_raw = findViewById(R.id.btn_img_raw);
        btn_img_raw.setOnClickListener(_button_listener_raw);
        btn_img_raw.setEnabled(false);
        cbtn_show_statistics = findViewById(R.id.cbtn_img_statistics);
        cbtn_show_statistics.setOnCheckedChangeListener(_checkbox_listener_show);
        cbtn_show_statistics.setChecked(b_show_statistics);
        cbtn_show_statistics.setEnabled(false);
        cbtn_show_rotated = findViewById(R.id.cbtn_img_rotation);
        cbtn_show_rotated.setOnCheckedChangeListener(_checkbox_listener_rotate);
        cbtn_show_rotated.setChecked(b_show_rotated);
        cbtn_show_rotated.setEnabled(false);
    }
    /* end onCreate() */

    @Override
    public void onStart() {
        Log.i(SYNA_TAG, "onStart() + " );

        /* check the syna touch device */
        boolean ret = native_lib.onCheckDev();
        if (!ret) {
            showMessage("Could not find the proper /dev/rmi or /dev/tcm node in the file system.\n\n" +
                    "Please use ADB interface to confirm the followings: \n" +
                    "1. The synaptics touch driver is installed.\n" +
                    "2. The permission of /dev/rmi or /dev/tcm file is proper.\n");
        }
        else {
            btn_img_delta.setEnabled(true);
            btn_img_raw.setEnabled(true);
            cbtn_show_statistics.setEnabled(true);
            cbtn_show_rotated.setEnabled(true);
            text_msg_to_stop.setVisibility(View.INVISIBLE);

            finger_threshold = native_lib.getThreshold();
            Log.i(SYNA_TAG, "onStart() : finger_threshold = " + finger_threshold);

        }

        super.onStart();
    }
    /* end onStart() */

    @Override
    public void onStop() {
        Log.i(SYNA_TAG, "onStop() + " );
        b_running = false;
        super.onStop();
    }
    /* end onStop() */

    @Override
    public void onDestroy() {
        Log.i(SYNA_TAG, "onDestroy() + " );
        b_running = false;
        super.onDestroy();
    }
    /* end onDestroy() */

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {

        switch(keyCode) {

            case KeyEvent.KEYCODE_VOLUME_UP:
                if  (event.getRepeatCount() ==  0 ) {
                    event.startTracking();

                    if (b_running) {
                        b_running = false;
                        Log.i(SYNA_TAG, "onKeyDown() : get VOL_UP, stop image acquisition");

                        btn_img_delta.setEnabled(true);
                        btn_img_raw.setEnabled(true);
                        cbtn_show_statistics.setEnabled(true);
                        text_msg_to_stop.setVisibility(View.INVISIBLE);
                    }

                }
                return true ;

        }
        return super.onKeyDown(keyCode, event);
    }
    /* end onKeyDown() */

    private CheckBox.OnCheckedChangeListener _checkbox_listener_show = new CheckBox.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            b_show_statistics = isChecked;
        }
    }; /* end Button.OnClickListener() */

    private CheckBox.OnCheckedChangeListener _checkbox_listener_rotate = new CheckBox.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            b_show_rotated = isChecked;
        }
    }; /* end Button.OnClickListener() */

    private View.OnClickListener _button_listener_delta = new View.OnClickListener() {
        @Override
        public void onClick(View view) {

            layout_statistics.setVisibility(View.VISIBLE);
            layout_buttons.setVisibility(View.INVISIBLE);

            btn_img_delta.setEnabled(false);
            btn_img_raw.setEnabled(false);
            cbtn_show_statistics.setEnabled(false);

            text_statistics.setText("");

            /* read delta image */
            n_type = native_lib.IMG_DELTA;
            doImageAcquisition();

        }
    }; /* end Button.OnClickListener() */

    private View.OnClickListener _button_listener_raw = new View.OnClickListener() {
        @Override
        public void onClick(View view) {

            layout_statistics.setVisibility(View.VISIBLE);
            layout_buttons.setVisibility(View.INVISIBLE);

            btn_img_delta.setEnabled(false);
            btn_img_raw.setEnabled(false);
            cbtn_show_statistics.setEnabled(false);

            text_statistics.setText("");

            /* read raw image */
            n_type = native_lib.IMG_RAW;
            doImageAcquisition();

        }
    }; /* end Button.OnClickListener() */

    private final Object ui_sync = new Object();  /* for synchronization between UI and process thread  */

    private int n_value_max;  /* value to keep the max. in the frame */
    private int n_value_overthreshold;  /* value to keep the num which is over threshold */
    private int n_num_in_row;
    private int n_num_in_column;
    private int n_type;
    private boolean flag_err;

    private int n_frame_show_idx;

    private void doImageAcquisition() {
        Log.i(SYNA_TAG, "doImageAcquisition() + " );

        text_msg_to_stop.setVisibility(View.VISIBLE);

        /* clear the vector */
        vector_image_frames.clear();
        n_value_max = 0;
        n_value_overthreshold = 0;

        /* retrieve the layout info from identification */
        boolean ret = native_lib.retrieveImageLayout();
        if (!ret) {
            Log.e(SYNA_TAG, "doImageAcquisition() : fail to retrieve the image info" );
            return;
        }

        n_num_in_row = native_lib.getNumCellinRow();
        n_num_in_column = native_lib.getNumCellinColumn();
        Log.i(SYNA_TAG, "doImageAcquisition() : num_in_row = " + n_num_in_row +
                ", num_in_column = " + n_num_in_column);

        /* create a thread to run report image logger */
        new Thread(new Runnable() {
            public void run() {
                boolean ret;
                short[] data_buf;

                /* to disable the report stream */
                ret = native_lib.prepareReportFrame(n_type);
                if (!ret) {
                    Log.e(SYNA_TAG, "AdoImageAcquisition() : fail to enable the report" );
                }

                /* set flag to true to indicate the process is ongoing */
                b_running = true;
                flag_err = false;

                n_frame_show_idx = 0;

                /* collect the report image until the flag is_running changed */
                while (b_running && !flag_err) {

                    data_buf = new short[n_num_in_row * n_num_in_column];

                    /* call native method to get a report from syna device */
                    ret = native_lib.requestReportFrame(n_type, data_buf);
                    if (!ret) {
                        b_running = false;
                        flag_err = true;
                    }
                    else {
                        /* put one image data into queue */
                        vector_image_frames.add(data_buf);
                    }

                    /* show report data on the UI */
                    runOnUiThread(new Runnable() {
                        public void run(){
                            if (b_running && (n_frame_show_idx < vector_image_frames.size())) {
                                /* remove the previous display */
                                table_layout.removeAllViews();
                                table_layout.setStretchAllColumns(true);

                                /* get one image data from the queue and show it on the UI */
                                showImageData(vector_image_frames.get(n_frame_show_idx));

                                n_frame_show_idx += 1;
                            }
                            /* try to synchronize with UI */
                            synchronized(ui_sync) {
                                ui_sync.notify();
                            }
                        }
                    });
                    /* wait for the UI update completion */
                    try{
                        synchronized(ui_sync) {
                            ui_sync.wait();
                        }
                    } catch(InterruptedException ignored){}

                } /* end while */

                /* to disable the report stream */
                ret = native_lib.completeReportFrame(n_type);
                if (!ret) {
                    Log.e(SYNA_TAG, "AdoImageAcquisition() : fail to disable the report" );
                }

                /* show result */
                runOnUiThread(new Runnable() {
                    public void run(){
                        if (flag_err) {
                            showMessage("Error:\n\nFail to get the syna report image");
                        }

                        /* enable all buttons */
                        btn_img_delta.setEnabled(true);
                        btn_img_raw.setEnabled(true);
                        cbtn_show_statistics.setEnabled(true);
                        text_msg_to_stop.setVisibility(View.INVISIBLE);
                        layout_statistics.setVisibility(View.INVISIBLE);
                        layout_buttons.setVisibility(View.VISIBLE);
                    }
                });

                /* set flag to false to indicate the process is terminated */
                b_running = false;

            } /* end of run() */

        }).start(); /* start the Thread */


    }/* end doDeltaAcquisition() */

    private void showImageData(short[] image) {
        table_layout.setShrinkAllColumns(true);
        table_layout.setStretchAllColumns(true);

        String str = " Max. = " + n_value_max ;
        if (n_type == native_lib.IMG_DELTA) {
            str += " , Over Threshold = " + n_value_overthreshold;
        }

        if (b_show_statistics)
            text_statistics.setText(str);

        int offset;
        String str_value;

        int img_row = Math.max(n_num_in_row, n_num_in_column);
        int img_col = Math.min(n_num_in_row, n_num_in_column);

        if (b_show_rotated) {

            for(int i = img_col - 1; i >= 0; i--) {
                TableRow row_data = new TableRow(DataShow.this);
                for(int j = 0; j < img_row; j++) {

                    offset = i * img_row + j;

                    int data = (n_type == native_lib.IMG_RAW)?image[offset]&0xFFFF:image[offset];
                    int color = Color.LTGRAY;
                    if (n_type == native_lib.IMG_DELTA) {
                        if (image[offset] > finger_threshold) {
                            n_value_overthreshold += 1;
                            color = Color.RED;
                        }
                    }

                    str_value = String.format(Locale.getDefault(),"% 6d", data);
                    addTextToTableRow(row_data,
                            str_value,
                            color);
                    n_value_max = Math.max(n_value_max, image[offset]);
                    if (n_type == native_lib.IMG_DELTA) {
                        if (image[offset] > finger_threshold)
                            n_value_overthreshold += 1;
                    }
                }
                table_layout.addView(row_data);
            }
        }
        else {

            for(int i = 0; i < img_col; i++) {
                TableRow row_data = new TableRow(DataShow.this);
                for(int j = 0; j < img_row; j++) {

                    offset = i * img_row + j;

                    int data = (n_type == native_lib.IMG_RAW)?image[offset]&0xFFFF:image[offset];
                    int color = Color.LTGRAY;
                    if (n_type == native_lib.IMG_DELTA) {
                        if (image[offset] > finger_threshold) {
                            n_value_overthreshold += 1;
                            color = Color.RED;
                        }
                    }

                    str_value = String.format(Locale.getDefault(),"% 6d", data);
                    addTextToTableRow(row_data,
                            str_value,
                            color);
                    n_value_max = Math.max(n_value_max, image[offset]);
                    if (n_type == native_lib.IMG_DELTA) {
                        if (image[offset] > finger_threshold)
                            n_value_overthreshold += 1;
                    }
                }
                table_layout.addView(row_data);
            }
        }


    } /* end showReportData() */

    private void addTextToTableRow(TableRow row, String str, int color) {
        int FONT_SIZE = 6;

        TextView text = new TextView(DataShow.this);
        text.setText(str);
        text.setGravity(Gravity.CENTER);
        text.setTextColor(color);
        text.setBackgroundResource(R.drawable.text_view_border);
        text.setWidth(display_width/n_num_in_column);
        text.setHeight(display_height/(n_num_in_row + 2));
        text.setTextSize(FONT_SIZE);
        /* add text to table row */
        row.addView(text);
    } /* end addTextToTableRow() */


    private void showMessage(String msg) {
        AlertDialog.Builder dlg  = new AlertDialog.Builder(this);
        dlg.setMessage(msg);
        dlg.setPositiveButton("OK", null);
        dlg.create().show();
    } /* end showMessage() */

}
