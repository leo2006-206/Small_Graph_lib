#!/bin/bash

# 1. Safety check: Did you actually provide an executable name?
if [ -z "$1" ]; then
    echo "Error: Please provide an executable name."
    echo "Usage: ./run.sh <executable_name> [clang|gcc|both] [stat|record|asm]"
    exit 1
fi

EXECUTABLE="$1"
# Change the default fallback to 'both' instead of 'clang'
COMPILER="${2:-both}" 
MODE="$3"

# Parse arguments to handle different modes
# Cases: 
#   ./run.sh <exec> asm               -> COMPILER=both, MODE=asm
#   ./run.sh <exec> stat              -> COMPILER=both, MODE=stat
#   ./run.sh <exec> record            -> COMPILER=both, MODE=record
#   ./run.sh <exec> <compiler> asm    -> COMPILER=compiler, MODE=asm

if [[ "$COMPILER" == "stat" || "$COMPILER" == "record" || "$COMPILER" == "asm" ]]; then
    MODE="$COMPILER"
    COMPILER="both"
fi

# 2. Define a reusable function to build and run
run_target() {
    local comp=$1
    local build_path="./build_${comp}/"
    
    echo "=================================================="
    echo "  Building and processing with: ${comp^^} (C++)"
    echo "=================================================="
    
    # Build the specific target
    make "${comp}_make" || exit 1
    
    printf "\n"
    
    # Run normally, with perf stat, with perf record, or dump asm
    if [[ "$MODE" == "stat" ]]; then
        ctl_fifo="perf_control.fifo"
        rm -f $ctl_fifo
        mkfifo $ctl_fifo
        exec 3<>$ctl_fifo
        perf stat -d -D -1 --control fifo:perf_control.fifo taskset -c 8 "${build_path}${EXECUTABLE}" 2>&1 | grep -v "cpu_atom"
        exec 3>&-
        rm -f $ctl_fifo
        
    elif [[ "$MODE" == "record" ]]; then
        ctl_fifo="perf_control.fifo"
        rm -f $ctl_fifo
        mkfifo $ctl_fifo
        exec 3<>$ctl_fifo
        echo "Recording performance data..."
        perf record -D -1 --control fifo:perf_control.fifo -o "perf_${comp}.data" taskset -c 8 "${build_path}${EXECUTABLE}"
        printf "\n"
        echo "Generating performance report..."
        perf report -i "perf_${comp}.data"
        rm ./*.data
        rm -f $ctl_fifo
        
    elif [[ "$MODE" == "asm" ]]; then
        echo "Generating assembly dump..."
        local out_file="asm_${comp}.txt"
        
        # -d: disassemble, -S: interleave C++ source, -C: demangle C++ names, -M intel: use Intel syntax (like Godbolt)
        objdump -d -S -C -M intel "${build_path}${EXECUTABLE}" > "$out_file"
        
        echo "Assembly saved to ./$out_file"
        
    else
        taskset -c 8 "${build_path}${EXECUTABLE}"
    fi
    
    printf "\n\n"
}

# 3. Execute based on the user's choice
if [[ "$COMPILER" == "both" ]]; then
    run_target "gcc"
    run_target "clang"
elif [[ "$COMPILER" == "gcc" || "$COMPILER" == "clang" ]]; then
    run_target "$COMPILER"
else
    echo "Error: Unknown compiler '$COMPILER'. Use 'clang', 'gcc', or 'both'."
    exit 1
fi

exit 0