@echo off
@rem Copyright (c) 2026-2027 Huawei Device Co., Ltd.
@rem Licensed under the Apache License, Version 2.0 (the "License");
@rem you may not use this file except in compliance with the License.
@rem You may obtain a copy of the License at
@rem
@rem     http://www.apache.org/licenses/LICENSE-2.0
@rem
@rem Unless required by applicable law or agreed to in writing, software
@rem distributed under the License is distributed on an "AS IS" BASIS,
@rem WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
@rem See the License for the specific language governing permissions and
@rem limitations under the License.
@rem Layer 3: JNI integration test automation script for Windows
@rem Build and run JNI integration tests for specified module (Android Instrumentation Test)
@rem
@rem Usage: integration-test.bat <module>    [-test=<filter>] [-build-only]
@rem        integration-test.bat all        [-test=<filter>] [-build-only]
@rem        integration-test.bat -h|--help
@rem
@rem Prerequisites:
@rem   1. ANDROID_HOME (or ANDROID_SDK_ROOT) environment variable set
@rem   2. JAVA_HOME environment variable set (Java 11+)
@rem   3. Android device connected or emulator running (adb devices visible)
@rem      (Not required when using -build-only flag)

setlocal enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
cd /d "%SCRIPT_DIR%"

rem ===== Parse module list from modules.json =====
set "MOD_COUNT=0"
for /f "tokens=1,2 delims=|" %%a in ('powershell -NoProfile -Command "& {$json = Get-Content '%SCRIPT_DIR%modules.json' -Raw | ConvertFrom-Json; $json.PSObject.Properties | ForEach-Object { Write-Output \"$($_.Name)|$($_.Value)\" }}"') do (
    set /a MOD_COUNT+=1
    set "MOD_!MOD_COUNT!=%%a"
    set "MODPATH_!MOD_COUNT!=%%b"
)

rem ===== Parse arguments =====
set "MODULE="
set "TEST_FILTER="
set "BUILD_ONLY=false"

:parse_args
if "%~1"=="" goto :args_done
if "%~1"=="-h" goto :show_usage
if "%~1"=="--help" goto :show_usage
if "%~1"=="all" (
    set "MODULE=all"
    shift
    goto :parse_args
)
if "%~1"=="-build-only" (
    set "BUILD_ONLY=true"
    shift
    goto :parse_args
)
set "ARG_PREFIX=%~1"
set "ARG_PREFIX=%ARG_PREFIX:~0,6%"
if "%ARG_PREFIX%"=="-test=" (
    set "TEST_FILTER=%~1"
    set "TEST_FILTER=!TEST_FILTER:~6!"
    shift
    goto :parse_args
)
set "ARG_FIRST=%~1"
set "ARG_FIRST_CHAR=%ARG_FIRST:~0,1%"
if "%ARG_FIRST_CHAR%"=="-" (
    echo [ERROR] Unknown option: %~1
    goto :show_usage
)
if "%MODULE%"=="" set "MODULE=%~1"
shift
goto :parse_args

:args_done
if "%MODULE%"=="" goto :show_usage

rem ===== Show usage =====
goto :skip_usage
:show_usage
echo Usage: integration-test.bat ^<module^>    [-test=^<filter^>] [-build-only]
echo        integration-test.bat all        [-test=^<filter^>] [-build-only]
echo        integration-test.bat -h^|--help
echo.
echo Options:
echo   -build-only    Build test APK without running tests (no device required)
echo.
echo Available modules:
for /l %%i in (1,1,%MOD_COUNT%) do (
    echo   - !MOD_%%i!
)
echo.
echo Examples:
echo   integration-test.bat <module>
echo   integration-test.bat all -test=<test_class_name>
echo   integration-test.bat <module> -test=<test_method_name>
echo   integration-test.bat <module> -build-only
endlocal
exit /b 1
:skip_usage

rem ===== Validate single module =====
set "MODULE_VALID=0"
for /l %%i in (1,1,%MOD_COUNT%) do (
    if "!MOD_%%i!"=="%MODULE%" set "MODULE_VALID=1"
)
if "%MODULE_VALID%"=="0" (
    echo [ERROR] Unknown module: %MODULE%
    echo.
    goto :show_usage
)

rem ===== Environment checks =====

rem Check ANDROID_HOME
if not defined ANDROID_HOME (
    if not defined ANDROID_SDK_ROOT (
        echo [ERROR] ANDROID_HOME not set.
        echo   Please set ANDROID_HOME to your Android SDK path.
        echo   Example: set ANDROID_HOME=%%LOCALAPPDATA%%\Android\sdk
        endlocal
        exit /b 1
    )
)
if defined ANDROID_HOME (
    set "ANDROID_HOME_VAL=%ANDROID_HOME%"
) else (
    set "ANDROID_HOME_VAL=%ANDROID_SDK_ROOT%"
)

rem Check JAVA_HOME
if not defined JAVA_HOME (
    echo [ERROR] JAVA_HOME not set.
    echo   Please set JAVA_HOME to your Java 11+ installation path.
    echo   Example: set JAVA_HOME=C:\Program Files\Java\jdk-11
    endlocal
    exit /b 1
)

rem Check adb (skip in build-only mode)
if not "%BUILD_ONLY%"=="true" (
    where adb >nul 2>&1
    if errorlevel 1 (
        echo [ERROR] adb command not found. Ensure ANDROID_HOME\platform-tools is in PATH.
        endlocal
        exit /b 1
    )
)

rem Check device (skip in build-only mode)
if "%BUILD_ONLY%"=="true" (
    echo [INFO] Build-only mode: skipping device check
) else (
    adb devices | findstr /r "device$" >nul 2>&1
    if errorlevel 1 (
        echo [ERROR] No Android device/emulator connected.
        echo Please connect a device or start an emulator, or use -build-only to skip test execution.
        endlocal
        exit /b 1
    )

    for /f "tokens=*" %%d in ('adb shell getprop ro.product.model 2^>nul') do set "DEVICE_INFO=%%d"
    for /f "tokens=*" %%a in ('adb shell getprop ro.build.version.sdk 2^>nul') do set "API_LEVEL=%%a"
    echo [INFO] Device: %DEVICE_INFO% (API %API_LEVEL%)
)

rem Create local.properties
echo sdk.dir=%ANDROID_HOME_VAL%> local.properties

rem Create log directory
if not exist "log" mkdir log

rem ===== Build Gradle arguments =====
set "GRADLE_EXTRA_ARGS="
if not "%TEST_FILTER%"=="" (
    set "GRADLE_EXTRA_ARGS=-Pandroid.testInstrumentationRunnerArguments.class=%TEST_FILTER%"
)

rem ===== Main execution =====
if "%MODULE%"=="all" (
    call :run_all_modules
) else (
    call :run_module "%MODULE%"
)
goto :script_end

rem ===== Run single module =====
:run_module
set "mod=%~1"
set "timestamp=%date:~0,4%%date:~5,2%%date:~8,2%_%time:~0,2%%time:~3,2%%time:~6,2%"
set "timestamp=%timestamp: =0%"
set "log_file=log\integration-test_%mod%_%timestamp%.log"

echo =========================================
echo   Layer 3: JNI integration test (module: %mod%)
echo   Log: %log_file%
echo =========================================

rem Build test APK
echo [1/2] Building test APK (module: %mod%)...
call gradlew.bat -Pmodule="%mod%" :app:assembleDebugAndroidTest > "%log_file%" 2>&1
set "BUILD_EXIT_CODE=%ERRORLEVEL%"

if not "%BUILD_EXIT_CODE%"=="0" (
    type "%log_file%"
    echo.
    echo =========================================
    echo   Layer 3 build failed (%mod%, exit %BUILD_EXIT_CODE%)
    echo =========================================
    exit /b %BUILD_EXIT_CODE%
)

if "%BUILD_ONLY%"=="true" (
    echo.
    echo =========================================
    echo   Layer 3 Build Result: APK built (%mod%)
    echo =========================================
    exit /b 0
)

rem Install and run tests
echo [2/2] Installing and running tests...
echo -----------------------------------------
call gradlew.bat -Pmodule="%mod%" :app:connectedDebugAndroidTest %GRADLE_EXTRA_ARGS% >> "%log_file%" 2>&1
set "EXIT_CODE=%ERRORLEVEL%"
echo -----------------------------------------

rem Print test summary
call :print_test_summary "%mod%"

if "%EXIT_CODE%"=="0" (
    echo.
    echo =========================================
    echo   Layer 3 test result: ALL PASSED (%mod%)
    echo =========================================
) else (
    echo.
    echo =========================================
    echo   Layer 3 test result: FAILED (%mod%, exit %EXIT_CODE%)
    echo =========================================
)
exit /b %EXIT_CODE%

rem ===== Print test summary =====
:print_test_summary
set "mod=%~1"

rem Find module app dir from modules.json
set "mod_app_dir="
for /l %%i in (1,1,%MOD_COUNT%) do (
    if "!MOD_%%i!"=="%mod%" set "mod_app_dir=!MODPATH_%%i!"
)

if "%mod_app_dir%"=="" (
    echo   [WARN] Module path not found for %mod%
    exit /b 1
)

set "result_dir=%SCRIPT_DIR%%mod_app_dir%\build\outputs\androidTest-results\connected"

rem Find TEST-*.xml file
set "xml_file="
for /f "tokens=*" %%f in ('dir /b "%result_dir%\TEST-*.xml" 2^>nul') do (
    set "xml_file=%result_dir%\%%f"
    goto :found_xml
)
:found_xml

if "%xml_file%"=="" (
    echo   [WARN] Test result file not found
    exit /b 1
)

rem Parse XML test results using PowerShell
for /f "tokens=1,2,3,4,5 delims=," %%a in ('powershell -NoProfile -Command "& {$xml = [xml](Get-Content '%xml_file%'); $tests=[int]$xml.testsuite.tests; $fail=[int]$xml.testsuite.failures; $errs=[int]$xml.testsuite.errors; $skip=[int]$xml.testsuite.skipped; $pass=$tests-$fail-$errs-$skip; Write-Output \"$tests,$pass,$fail,$errs,$skip\"}"') do (
    set "TOTAL=%%a"
    set "PASSED=%%b"
    set "FAILED=%%c"
    set "ERRORS=%%d"
    set "SKIPPED=%%e"
)

echo.
echo   ----- Test results (%mod%) -----
echo   Total:   %TOTAL%
echo   Passed:  %PASSED%
echo   Failed:  %FAILED%
echo   Errors:  %ERRORS%
echo   Skipped: %SKIPPED%
echo   ---------------------------
exit /b 0

rem ===== Run all modules =====
:run_all_modules
set "FAILED_COUNT=0"
set "FAILED_LIST="

for /l %%i in (1,1,%MOD_COUNT%) do (
    call :run_module "!MOD_%%i!"
    if errorlevel 1 (
        set /a FAILED_COUNT+=1
        set "FAILED_LIST=!FAILED_LIST! !MOD_%%i!"
    )
)

echo.
echo =========================================
echo   Layer 3 test summary
echo =========================================

rem Show per-module statistics
set "TOTAL_ALL=0"
set "PASSED_ALL=0"
set "FAILED_ALL=0"
set "ERRORS_ALL=0"
set "SKIPPED_ALL=0"

for /l %%i in (1,1,%MOD_COUNT%) do (
    set "mod_name=!MOD_%%i!"
    set "mod_path=!MODPATH_%%i!"
    set "result_dir=%SCRIPT_DIR%!mod_path!\build\outputs\androidTest-results\connected"

    set "xml_file="
    for /f "tokens=*" %%f in ('dir /b "!result_dir!\TEST-*.xml" 2^>nul') do (
        set "xml_file=!result_dir!\%%f"
    )

    if defined xml_file (
        for /f "tokens=1,2,3,4,5 delims=," %%a in ('powershell -NoProfile -Command "& {$xml = [xml](Get-Content '!xml_file!'); $tests=[int]$xml.testsuite.tests; $fail=[int]$xml.testsuite.failures; $errs=[int]$xml.testsuite.errors; $skip=[int]$xml.testsuite.skipped; $pass=$tests-$fail-$errs-$skip; Write-Output \"$tests,$pass,$fail,$errs,$skip\"}"') do (
            echo   !mod_name!: %%b/%%a passed (failed:%%c errors:%%d skipped:%%e)
            set /a TOTAL_ALL+=%%a
            set /a PASSED_ALL+=%%b
            set /a FAILED_ALL+=%%c
            set /a ERRORS_ALL+=%%d
            set /a SKIPPED_ALL+=%%e
        )
    ) else (
        echo   !mod_name!: no test results
    )
)
echo   ---------------------------
echo   Total: %PASSED_ALL%/%TOTAL_ALL% passed (failed:%FAILED_ALL% errors:%ERRORS_ALL% skipped:%SKIPPED_ALL%)

if "%FAILED_COUNT%"=="0" (
    echo   All passed (%MOD_COUNT% modules)
) else (
    echo   Failed modules (%FAILED_COUNT%/%MOD_COUNT%):
    for %%m in (%FAILED_LIST%) do echo     - %%m
    endlocal
    exit /b 1
)
exit /b 0

:script_end
endlocal
