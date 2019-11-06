@echo off
setlocal enableextensions enabledelayedexpansion

set FOLDER=/sdcard/Android/data/com.touchscreen.tptest
set INI=syna_test_cfg_3908.ini

adb root
adb shell setenforce 0
adb shell "chmod 777 /sdcard"
adb shell "mkdir %FOLDER%"
adb push %~dp0%INI% %FOLDER%

:_FOUND_RMI_DEV
set rmi_is_found=
adb shell ls /dev/rmi* > check_rmi_dev
for /F "tokens=1-2 delims=: " %%a in (check_rmi_dev) do (
	if not "%%b"=="No" (
		set rmi_is_found=1
	)
)
del check_rmi_dev

if "%rmi_is_found%"=="" GOTO _FOUND_TCM_DEV	

adb shell "chmod 666 /dev/rmi0"

:_FOUND_TCM_DEV
set tcm_is_found=
adb shell ls /dev/tcm* > check_tcm_dev
for /F "tokens=1-2 delims=: " %%a in (check_tcm_dev) do (
	if not "%%b"=="No" (
		set tcm_is_found=1
	)
)
del check_tcm_dev

if "%tcm_is_found%"=="" GOTO :_LAUNCH_APP

adb shell "chmod 666 /dev/tcm0"

:_LAUNCH_APP

:: ---------------------------------------------------------- ::
:: Parameters                                                 ::
:: ---------------------------------------------------------- ::
:: CFG_LOG_SAVE_STR  - false: disable the log saving          ::
::                   - true : enable the log saving           ::
:: CFG_LOG_PATH      - path of log file placed                ::
:: CFG_TEST_INI      - false: no test config file             ::
::                   - true : input an external test config   ::
:: CFG_TEST_INI_FILE - test config file                       ::
:: ----------------------------------------------------- ::
adb shell am force-stop com.vivotouchscreen.sensortest
adb shell am start -a android.intent.action.MAIN -n com.vivotouchscreen.sensortestsyna3908/.MainActivity --activity-clear-task ^
 -e "CFG_LOG_SAVE_STR" "true" ^
 -e "CFG_LOG_PATH" "%FOLDER%"/ ^
 -e "CFG_TEST_INI" "true" ^
 -e "CFG_TEST_INI_FILE" "%FOLDER%/%INI%" ^
endlocal
