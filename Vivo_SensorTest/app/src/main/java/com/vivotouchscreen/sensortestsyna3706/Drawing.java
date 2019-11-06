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

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PaintFlagsDrawFilter;
import android.view.View;

public class Drawing extends View {

    private Paint paint;
    private Canvas canvas;
    private Bitmap bitmap;

    int width;
    int height;

    /********************************************************
     * Default Constructor
     ********************************************************/
    public Drawing(Context context) {
        super(context);
    }
    /********************************************************
     * Constructor to initialize all essential object
     ********************************************************/
    public Drawing(Context context, int display_width, int display_height, int color) {
        super(context);

        width = display_width;
        height = display_height;

        canvas = new Canvas();
        canvas.drawColor(color);
        canvas.setDrawFilter(new PaintFlagsDrawFilter(0,
                            Paint.ANTI_ALIAS_FLAG|Paint.FILTER_BITMAP_FLAG));

        paint = new Paint(Paint.DITHER_FLAG);
        paint.setAntiAlias(true);
        paint.setStyle(Paint.Style.STROKE);
        paint.setStrokeJoin(Paint.Join.ROUND);

        bitmap = Bitmap.createBitmap(display_width, display_height, Bitmap.Config.ARGB_8888);
        canvas.setBitmap(bitmap);

    }
    /********************************************************
     * Drawing on the canvas
     ********************************************************/
    @Override
    protected void onDraw(Canvas canvas) {
        //super.onDraw(canvas);
        canvas.drawBitmap(bitmap,0,0,null);
    }
    /********************************************************
     * Function to draw a circle with the appointed center
     ********************************************************/
    public void drawCircle(int x, int y, int radius, int color) {

        paint.setColor(color);
        paint.setStrokeWidth(6f);
        paint.setStyle(Paint.Style.STROKE);

        /* draw circle */
        canvas.drawCircle(x, y, radius, paint);

        /* draw text touch position */
        paint.setStrokeWidth(1f);
        paint.setTextSize(18);
        String msg = "( " + x + " , " + y + " )";
        float yf = ((y - 1.2 * radius) < 0)? 0 : (y - 2 * radius);
        canvas.drawText(msg, x, yf, paint);

        /* re-paint */
        invalidate();
    }
    /********************************************************
     * Function to draw a rectangle with the appointed position
     ********************************************************/
    public void drawRectangle(int l, int t, int r, int b, int color) {

        paint.setColor(color);
        paint.setStrokeWidth(6f);
        paint.setStyle(Paint.Style.FILL_AND_STROKE);

        /* draw circle */
        canvas.drawRect(l, t, r, b, paint);

        /* re-paint */
        invalidate();
    }
    /********************************************************
     * Function to draw the line
     ********************************************************/
    public void drawLine(int idx, int x, int y, int color, boolean start) {

        paint.setColor(color);
        paint.setStrokeWidth(6f);
        paint.setStyle(Paint.Style.STROKE);

        /* draw path */
        if (start) {
            canvas.drawPoint(x, y, paint);
            canvas.drawCircle(x, y, 120, paint);

            /* draw text touch position */
            paint.setStrokeWidth(1f);
            paint.setTextSize(18);
            String msg = "( " + x + " , " + y + " )";
            float yf = ((y - 150) < 0)? 0 : (y - 150);
            canvas.drawText(msg, x, yf, paint);

            pos_x[idx] = x;
            pos_y[idx] = y;
        }
        else {
            canvas.drawLine(pos_x[idx], pos_y[idx], x, y, paint);

            pos_x[idx] = x;
            pos_y[idx] = y;
        }

        /* re-paint */
        invalidate();
    }

    int[] pos_x = new int[10];
    int[] pos_y = new int[10];

    /********************************************************
     * Function to clear the existed circle
     ********************************************************/
    public void clearDraw() {
        canvas.drawColor(Color.BLACK);
        invalidate();
    }
}