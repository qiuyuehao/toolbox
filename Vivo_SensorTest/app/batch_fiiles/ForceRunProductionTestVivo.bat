@echo off
setlocal enableextensions enabledelayedexpansion

set FOLDER=/sdcard/Android/data/com.touchscreen.tptest
set INI=syna_test_cfg_s3706.ini

adb root
adb shell setenforce 0
adb shell "chmod 777 /sdcard"
adb shell "mkdir %FOLDER%"
adb push %~dp0%INI% %FOLDER%


adb shell am start -a android.intent.action.MAIN -n com.vivotouchscreen.sensortestsyna3706/.ActivityVIVOProduction --activity-clear-task ^

endlocal
