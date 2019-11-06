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
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.TextView;

public class ActivityIdentifier extends Activity {

    private final String SYNA_TAG = "syna-apk";

    /********************************************************
     * variables for device nodes
     ********************************************************/
    private String str_dev_node;
    private String str_dev_rmi_en;
    private String str_dev_tcm_en;
    
    /********************************************************
     * components on this page
     ********************************************************/
    TextView text_identify_info;
    /********************************************************
     * object for native function access
     ********************************************************/
    private NativeWrapper native_lib;

    /********************************************************
     * called from MainActivity if button 'IDENTIFY' is pressed
     * to initial all UI objects
     * then, run doIdentification() in the end
     ********************************************************/
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        /* to hide the navigation bar */
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        /* set view */
        setContentView(R.layout.page_identifier);

        Log.i(SYNA_TAG, "ActivityIdentifier onCreate() + " );

        Intent intent = this.getIntent();
        str_dev_node = intent.getStringExtra(Common.STR_DEV_NODE);
        str_dev_rmi_en = intent.getStringExtra(Common.STR_DEV_RMI_EN);
        str_dev_tcm_en = intent.getStringExtra(Common.STR_DEV_TCM_EN);
        Log.i(SYNA_TAG, "ActivitySetting onCreate() dev_node: " +
                str_dev_node + " (rmi: " + str_dev_rmi_en + ") (tcm: " + str_dev_tcm_en + ")");

        /* create an object to handle native methods calling */
        native_lib = new NativeWrapper();

        /* UI components - button exit */
        Button btn_exit = findViewById(R.id.btn_identify_exit);
        btn_exit.setOnClickListener(_button_listener_exit);
        /* UI components - TextViews */
        text_identify_info = findViewById(R.id.text_identify_info);

        /* call function to perform device identification */
        doIdentification();

    } /* end onCreate() */

    /********************************************************
     * called when this application is closed
     ********************************************************/
    @Override
    public void onStop() {
        Log.i(SYNA_TAG, "ActivityIdentifier onStop() + " );
        super.onStop();
    } /* end onStop() */

    /********************************************************
     * function to listen the KeyEvent
     * VOLUME_UP - click the button exit
     ********************************************************/
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        switch(keyCode) {
            case KeyEvent.KEYCODE_VOLUME_UP:
                if  (event.getRepeatCount() ==  0 ) {
                    event.startTracking();
                }
                Button btn_exit = findViewById(R.id.btn_identify_exit);
                btn_exit.performClick();
                break;
        }
        return false;
    } /* end onKeyDown() */

    /********************************************************
     * implementation if 'EXIT' button is pressed
     * go back to the MAIN page
     ********************************************************/
    private View.OnClickListener _button_listener_exit = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            Intent intent = new Intent();
            intent.setClass(ActivityIdentifier.this, MainActivity.class);
            startActivity(intent);
            ActivityIdentifier.this.finish();
        }
    }; /* end Button.OnClickListener() */

    /********************************************************
     * function to do identification
     * will get identify report and application report first
     * show all information on the display
     ********************************************************/
    private void doIdentification() {
        Log.i(SYNA_TAG, "ActivityIdentifier doIdentification() + " );

        StringBuilder sb = new StringBuilder();

        boolean ret = native_lib.onSetupDev(str_dev_node, str_dev_rmi_en, str_dev_tcm_en);
        if (!ret) {
            Log.e(SYNA_TAG, "ActivityIdentifier doIdentification() fail to find valid device" );
            sb.append("\n").append("Error : fail to find valid device node");
            text_identify_info.setText(sb.toString());
            text_identify_info.setTextColor(Color.RED);
            return;
        }

        ret = native_lib.onIdentifyDev(sb);
        if (!ret) {
            Log.e(SYNA_TAG, "ActivityIdentifier doIdentification() fail to do identify" );
            sb.append("\n").append("Error : fail to do identify, device information is not available");
            text_identify_info.setText(sb.toString());
            text_identify_info.setTextColor(Color.RED);
            return;
        }

        text_identify_info.setText(sb.toString());
        text_identify_info.setTextColor(Color.WHITE);

    }/* end doIdentification() */

}
