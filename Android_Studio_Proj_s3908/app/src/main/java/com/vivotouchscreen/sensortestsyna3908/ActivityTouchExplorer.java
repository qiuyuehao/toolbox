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
import android.content.Intent;
import android.graphics.Color;
import android.graphics.Point;
import android.os.Bundle;
import android.util.Log;
import android.view.Display;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.TableLayout;
import java.util.Queue;
import  java.util.LinkedList;

public class ActivityTouchExplorer extends Activity {

    private final String SYNA_TAG = "syna-apk";

    private final int MAX_TOUCH_OBJ = 10;

    /********************************************************
     * components for Image Data Logger Page
     ********************************************************/
    private Button btn_exit;

    /********************************************************
     * flag to save the status of logger process
     ********************************************************/
    private boolean b_running;

    /********************************************************
     * variables of drawing
     ********************************************************/
    private Drawing drawing_obj;

    private int[] colors = { 0xFFFA5858, 0xFFFAAC58, 0xFFF4FA58, 0xFF3ADF00,
                             0xFF58FAF4, 0xFF5882FA, 0xFF0404B4, 0xFFDA81F5,
                             0xFFFF0040, 0xFFD8D8D8, 0xFFFFFFFF};

    private Queue<TouchPoints>[] q_points = new Queue[MAX_TOUCH_OBJ];

    /********************************************************
     * called from MainActivity if button 'TOUCH EXPLORER' is pressed
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
        setContentView(R.layout.page_touch_explorer);

        Log.i(SYNA_TAG, "ActivityTouchExplorer onCreate() + " );

        /* get display resolution */
        Display display = getWindowManager().getDefaultDisplay();
        Point size = new Point();
        display.getSize(size);
        int display_width = size.x;
        int display_height = size.y;
        Log.i(SYNA_TAG, "ActivityTouchExplorer onCreate() display (width,height) = ("
                + display_width + " , " + display_height + ")" );

        /* UI components - table layout */
        drawing_obj = new Drawing(this, display_width, display_height, Color.BLACK);
        TableLayout container = findViewById(R.id.table_touch_explorer_background);
        container.addView(drawing_obj);
        /* UI components - button exit */
        btn_exit = findViewById(R.id.btn_touch_explorer_exit);
        btn_exit.setOnClickListener(_button_listener_exit);

        /* UI components - button clear */
        Button btn_clear = findViewById(R.id.btn_touch_explorer_clear);
        btn_clear.setOnClickListener(_button_listener_clear);

        /* initialize the array of queue */
        for (int i = 0; i < MAX_TOUCH_OBJ; i++) {
            q_points[i] = new LinkedList<>();
        }

        /* start the thread to handle ui output */
        Thread t = new Thread(ThreadPointsOutput);
        t.start();

        b_running = true;

    } /* end onCreate() */

    /********************************************************
     * called when this application is closed
     ********************************************************/
    @Override
    public void onStop() {
        Log.i(SYNA_TAG, "ActivityTouchExplorer onStop() + " );

        super.onStop();
    } /* end onStop() */


    /********************************************************
     * function to listen the KeyEvent
     * VOLUME_UP - stop the explorer
     ********************************************************/
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        switch(keyCode) {
            case KeyEvent.KEYCODE_VOLUME_UP:
                if  (event.getRepeatCount() ==  0 ) {
                    event.startTracking();
                }

                btn_exit.performClick();

                break;

        }
        return false;
    } /* end onKeyDown() */

    /********************************************************
     * function to listen the TouchEvent
     ********************************************************/
    @Override
    public boolean onTouchEvent(MotionEvent event) {

        int pointer_index = event.getActionIndex();
        int masked_action = event.getActionMasked();

        int x = (int)event.getX(pointer_index);
        int y = (int)event.getY(pointer_index);

        switch( masked_action ) {
            case MotionEvent.ACTION_DOWN:
            case MotionEvent.ACTION_POINTER_DOWN:

                onSaveTouchPoints(pointer_index, x, y, true);

                break;

            case MotionEvent.ACTION_MOVE:
                for (int i = 0; i < event.getPointerCount(); i++) {
                    pointer_index = event.findPointerIndex(i);

                    if (pointer_index >= 0) {
                        x = (int)event.getX(pointer_index);
                        y = (int)event.getY(pointer_index);

                        onSaveTouchPoints(pointer_index, x, y, false);
                    }
                }

                break;

            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_POINTER_UP:
            case MotionEvent.ACTION_CANCEL:

                break;
        }

        return true;
    }

    /********************************************************
     * implementation if 'EXIT' button is pressed
     * go back to the MAIN page
     ********************************************************/
    private View.OnClickListener _button_listener_exit = new View.OnClickListener() {
        @Override
        public void onClick(View view) {

            b_running = false;

            Intent intent = new Intent();
            intent.setClass(ActivityTouchExplorer.this, MainActivity.class);
            startActivity(intent);
            ActivityTouchExplorer.this.finish();
        }
    }; /* end Button.OnClickListener() */

    /********************************************************
     * implementation if 'CLEAR' button is pressed
     * clear the paint
     ********************************************************/
    private View.OnClickListener _button_listener_clear = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            onDrawClear();
        }
    }; /* end Button.OnClickListener() */

    /********************************************************
     * create a object to store the touch information
     ********************************************************/
    private void onSaveTouchPoints(int index, int x, int y, boolean is_action_down) {

        TouchPoints p = new TouchPoints(x, y, index, is_action_down);
        for (int i = 0; i < MAX_TOUCH_OBJ; i++) {
            if (i == index) {
                q_points[i].offer(p);
            }
        }
    }

    /********************************************************
     * thread to handle hte line drawing
     ********************************************************/
    private boolean is_done;
    private Runnable ThreadPointsOutput = new Runnable() {
        @Override
        public void run() {

            is_done = true;

            while (b_running) {
                for (int i = 0; i < MAX_TOUCH_OBJ; i++) {
                    while (q_points[i].size() > 0 && is_done) {
                        TouchPoints p = q_points[i].poll();
                        if (p != null) {

                            is_done = false;
                            if (p.is_action_down)
                                onDrawLine(p.index, p.x, p.y, colors[p.index], true);
                            else
                                onDrawLine(p.index, p.x, p.y, colors[p.index], false);
                        }

                    }
                }
            } /*  while b_running  */
        } /* end of run() */
    };


    private int draw_x;
    private int draw_y;
    private int draw_color;
    private int draw_idx;

    /********************************************************
     * function to update the line drawing
     ********************************************************/
    private void onDrawLine(int idx, int pos_x, int pos_y, int color, boolean start) {

        draw_x = pos_x;
        draw_y = pos_y;
        draw_color = color;
        draw_idx = idx;

        Log.i(SYNA_TAG, "ActivityTouchExplorer onDrawLine() update_line_drawing idx = " +
                idx + " (x,y) = " + draw_x + " , " + draw_y );

        if (start) {
            runOnUiThread(new Runnable() {
                public void run(){
                    drawing_obj.drawLine(draw_idx, draw_x, draw_y, draw_color, true);

                    is_done = true;
                }
            });
        }
        else {
            runOnUiThread(new Runnable() {
                public void run(){
                    drawing_obj.drawLine(draw_idx, draw_x, draw_y, draw_color, false);

                    is_done = true;
                }
            });
        }

    }
    /********************************************************
     * function to clear the drawing view
     ********************************************************/
    private void onDrawClear() {
        for (int i = 0; i < MAX_TOUCH_OBJ; i++) {
            q_points[i].clear();
        }
        /* call drawing_obj to clear */
        runOnUiThread(new Runnable() {
            public void run(){
                drawing_obj.clearDraw();
            }
        });
    }

}
