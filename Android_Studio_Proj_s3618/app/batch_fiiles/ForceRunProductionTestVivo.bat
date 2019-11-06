@echo off
setlocal enableextensions enabledelayedexpansion

set FOLDER=/sdcard/Android/data/com.touchscreen.tptest/
set INI=syna_test_cfg_3618f.ini

adb root
adb shell setenforce 0

adb shell am start -a android.intent.action.MAIN -n com.vivotouchscreen.sensortestsyna3618f/.ActivityVIVOProduction --activity-clear-task ^
 
endlocal
