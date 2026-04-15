#!/bin/bash
# Copyright (c) 2026-2027 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# Layer 3: JNI integration test automation script (module-aware)
# Build and run JNI integration tests for specified modules (Android Instrumentation Test)
#
# Usage: ./integration-test.sh <module>    [-test=<filter>] [-build-only]
#        ./integration-test.sh all        [-test=<filter>] [-build-only]
#        ./integration-test.sh -h|--help
#
# Prerequisites:
#   1. ANDROID_HOME (or ANDROID_SDK_ROOT) environment variable is set
#   2. JAVA_HOME environment variable is set (Java 11+)
#   3. Android device connected or emulator started (visible via adb devices)
#      (Not required when using -build-only flag)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "${SCRIPT_DIR}"

# Parse module list from modules.json dynamically
parse_module_list() {
    sed -n 's/.*"\([a-zA-Z_][a-zA-Z0-9_]*\)".*/\1/p' "${SCRIPT_DIR}/modules.json"
}
AVAILABLE_MODULES=()
while IFS= read -r line; do
    AVAILABLE_MODULES+=("$line")
done < <(parse_module_list)

# Find app directory path from modules.json by module name
get_module_app_dir() {
    local module="$1"
    sed -n "s/.*\"${module}\"[[:space:]]*:[[:space:]]*\"\([^\"]*\)\".*/\1/p" "${SCRIPT_DIR}/modules.json" | \
        head -1
}

# ===== Argument parsing =====
MODULE=""
TEST_FILTER=""
BUILD_ONLY=false

for arg in "$@"; do
    case "${arg}" in
        -h|--help)
            MODULE=""
            break
            ;;
        -test=*)
            TEST_FILTER="${arg#-test=}"
            ;;
        -build-only)
            BUILD_ONLY=true
            ;;
        all)
            MODULE="all"
            ;;
        -*)
            echo "[ERROR] Unknown option: ${arg}"
            MODULE=""
            break
            ;;
        *)
            if [ -z "${MODULE}" ]; then
                MODULE="${arg}"
            fi
            ;;
    esac
done

# ===== Show usage =====
show_usage() {
    echo "Usage: ./integration-test.sh <module>    [-test=<filter>] [-build-only]"
    echo "       ./integration-test.sh all        [-test=<filter>] [-build-only]"
    echo "       ./integration-test.sh -h|--help"
    echo ""
    echo "Options:"
    echo "  -build-only    Build test APK without running tests (no device required)"
    echo ""
    echo "Available modules:"
    for m in "${AVAILABLE_MODULES[@]}"; do
        echo "  - ${m}"
    done
    echo ""
    echo "Examples:"
    echo "  ./integration-test.sh <module>"
    echo "  ./integration-test.sh all -test=<test_class_name>"
    echo "  ./integration-test.sh <module> -test=<test_method_name>"
    echo "  ./integration-test.sh <module> -build-only"
}

if [ -z "${MODULE}" ]; then
    show_usage
    exit 1
fi

# Validate single module name
validate_module() {
    local mod="$1"
    for m in "${AVAILABLE_MODULES[@]}"; do
        if [ "${m}" = "${mod}" ]; then
            return 0
        fi
    done
    echo "[ERROR] Unknown module: ${mod}"
    echo ""
    show_usage
    return 1
}

# ===== Environment check =====

# Check ANDROID_HOME
if [ -z "${ANDROID_HOME:-}" ] && [ -z "${ANDROID_SDK_ROOT:-}" ]; then
    echo "[ERROR] ANDROID_HOME is not set."
    echo "  Please set ANDROID_HOME to your Android SDK path."
    echo "  Example: export ANDROID_HOME=~/Library/Android/sdk  (macOS)"
    echo "           export ANDROID_HOME=/opt/android-sdk       (Linux)"
    exit 1
fi
export ANDROID_HOME="${ANDROID_HOME:-${ANDROID_SDK_ROOT}}"

# Check JAVA_HOME
if [ -z "${JAVA_HOME:-}" ]; then
    echo "[ERROR] JAVA_HOME is not set."
    echo "  Please set JAVA_HOME to your Java 11+ installation path."
    echo "  Example: export JAVA_HOME=/usr/lib/jvm/java-11-openjdk  (Linux)"
    echo "           export JAVA_HOME=\$(/usr/libexec/java_home -v 11)  (macOS)"
    exit 1
fi

# Check adb (skip in build-only mode)
if [ "${BUILD_ONLY}" = false ]; then
    if ! command -v adb &>/dev/null; then
        echo "[ERROR] adb command not found. Please ensure ANDROID_HOME/platform-tools is in PATH."
        exit 1
    fi
fi

# Check device (skip in build-only mode)
if [ "${BUILD_ONLY}" = false ]; then
    if ! adb devices | grep -q "device$"; then
        echo "[ERROR] No Android device/emulator connected"
        echo "Please connect a device or start an emulator, or use -build-only to skip test execution"
        exit 1
    fi

    DEVICE_INFO=$(adb shell getprop ro.product.model 2>/dev/null | tr -d '\r')
    API_LEVEL=$(adb shell getprop ro.build.version.sdk 2>/dev/null | tr -d '\r')
    echo "[INFO] Device: ${DEVICE_INFO} (API ${API_LEVEL})"
else
    echo "[INFO] Build-only mode: skipping device check"
fi

# Create local.properties
echo "sdk.dir=${ANDROID_HOME}" > local.properties

# Create log directory
mkdir -p log

# ===== Build Gradle arguments =====
GRADLE_EXTRA_ARGS=""
if [ -n "${TEST_FILTER}" ]; then
    GRADLE_EXTRA_ARGS="-Pandroid.testInstrumentationRunnerArguments.class=${TEST_FILTER}"
fi

# ===== Test result summary =====
print_test_summary() {
    local mod="$1"
    local mod_app_dir
    mod_app_dir=$(get_module_app_dir "${mod}")
    local result_dir="${SCRIPT_DIR}/${mod_app_dir}/build/outputs/androidTest-results/connected"
    local xml_file
    xml_file=$(find "${result_dir}" -maxdepth 1 -name 'TEST-*.xml' 2>/dev/null | head -1)

    if [ -z "${xml_file}" ]; then
        echo "  [WARN] Test result file not found"
        return 1
    fi

    local tests failures errors skipped
    tests=$(grep -o 'tests="[0-9]*"' "${xml_file}" | head -1 | grep -o '[0-9]*')
    failures=$(grep -o 'failures="[0-9]*"' "${xml_file}" | head -1 | grep -o '[0-9]*')
    errors=$(grep -o 'errors="[0-9]*"' "${xml_file}" | head -1 | grep -o '[0-9]*')
    skipped=$(grep -o 'skipped="[0-9]*"' "${xml_file}" | head -1 | grep -o '[0-9]*')
    local passed=$((tests - failures - errors - skipped))

    echo ""
    echo "  ----- Test Results (${mod}) -----"
    echo "  Total:    ${tests}"
    echo "  Passed:   ${passed}"
    echo "  Failed:   ${failures}"
    echo "  Errors:   ${errors}"
    echo "  Skipped:  ${skipped}"
    echo "  ---------------------------"
}

# ===== Single module test =====
run_module() {
    local mod="$1"
    local timestamp
    timestamp=$(date +%Y%m%d_%H%M%S)
    local log_file="log/integration-test_${mod}_${timestamp}.log"

    echo "========================================="
    echo "  Layer 3: JNI Integration Test (module: ${mod})"
    echo "  Log: ${log_file}"
    echo "========================================="

    # Build test APK
    echo "[1/2] Building test APK (module: ${mod})..."
    local build_exit_code=0
    ./gradlew -Pmodule="${mod}" :app:assembleDebugAndroidTest 2>&1 | tee "${log_file}" || build_exit_code=$?
    if [ ${build_exit_code} -ne 0 ]; then
        echo ""
        echo "========================================="
        echo "  Layer 3 build failed (${mod}, exit ${build_exit_code})"
        echo "========================================="
        return ${build_exit_code}
    fi

    if [ "${BUILD_ONLY}" = true ]; then
        echo ""
        echo "========================================="
        echo "  Layer 3 Build Result: APK built (${mod})"
        echo "========================================="
        return 0
    fi

    # Install and run tests
    echo "[2/2] Installing and running tests..."
    echo "-----------------------------------------"
    local exit_code=0
    ./gradlew -Pmodule="${mod}" :app:connectedDebugAndroidTest ${GRADLE_EXTRA_ARGS} 2>&1 | tee -a "${log_file}" || exit_code=$?
    echo "-----------------------------------------"

    if [ ${exit_code} -eq 0 ]; then
        print_test_summary "${mod}"
        echo ""
        echo "========================================="
        echo "  Layer 3 Test Result: ALL PASSED (${mod})"
        echo "========================================="
    else
        print_test_summary "${mod}" 2>/dev/null
        echo ""
        echo "========================================="
        echo "  Layer 3 Test Result: FAILED (${mod}, exit ${exit_code})"
        echo "========================================="
    fi

    return ${exit_code}
}

# ===== Execute =====
if [ "${MODULE}" = "all" ]; then
    FAILED_MODULES=()
    for mod in "${AVAILABLE_MODULES[@]}"; do
        if ! run_module "${mod}"; then
            FAILED_MODULES+=("${mod}")
        fi
    done

    echo ""
    echo "========================================="
    echo "  Layer 3 Test Summary"
    echo "========================================="

    # Show test statistics per module
    total_tests=0
    total_passed=0
    total_failed=0
    total_errors=0
    total_skipped=0
    for mod in "${AVAILABLE_MODULES[@]}"; do
        mod_app_dir=$(get_module_app_dir "${mod}")
        result_dir="${SCRIPT_DIR}/${mod_app_dir}/build/outputs/androidTest-results/connected"
        xml_file=$(find "${result_dir}" -maxdepth 1 -name 'TEST-*.xml' 2>/dev/null | head -1)
        if [ -n "${xml_file}" ]; then
            t=$(grep -o 'tests="[0-9]*"' "${xml_file}" | head -1 | grep -o '[0-9]*')
            f=$(grep -o 'failures="[0-9]*"' "${xml_file}" | head -1 | grep -o '[0-9]*')
            e=$(grep -o 'errors="[0-9]*"' "${xml_file}" | head -1 | grep -o '[0-9]*')
            s=$(grep -o 'skipped="[0-9]*"' "${xml_file}" | head -1 | grep -o '[0-9]*')
            p=$((t - f - e - s))
            echo "  ${mod}: ${p}/${t} passed (failed:${f} errors:${e} skipped:${s})"
            total_tests=$((total_tests + t))
            total_passed=$((total_passed + p))
            total_failed=$((total_failed + f))
            total_errors=$((total_errors + e))
            total_skipped=$((total_skipped + s))
        else
            echo "  ${mod}: no test results"
        fi
    done
    echo "  ---------------------------"
    echo "  Total: ${total_passed}/${total_tests} passed (failed:${total_failed} errors:${total_errors} skipped:${total_skipped})"

    if [ ${#FAILED_MODULES[@]} -eq 0 ]; then
        echo "  All passed (${#AVAILABLE_MODULES[@]} modules)"
    else
        echo "  Failed modules (${#FAILED_MODULES[@]}/${#AVAILABLE_MODULES[@]}):"
        for m in "${FAILED_MODULES[@]}"; do
            echo "    - ${m}"
        done
        exit 1
    fi
else
    validate_module "${MODULE}"
    run_module "${MODULE}"
fi
