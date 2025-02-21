#!/bin/bash

# Set path to your NS-3 directory
NS3_DIR=~/ns-allinone-3.32/ns-3.32

# Function to check if a file has been modified since the last build
check_modified() {
    SRC_FILE=$1
    BUILD_DIR=$NS3_DIR/build

    if [ ! -f "$BUILD_DIR/$SRC_FILE.o" ]; then
        echo "$SRC_FILE needs to be compiled."
        return 0
    fi

    if [ "$NS3_DIR/scratch/$SRC_FILE.cc" -nt "$BUILD_DIR/$SRC_FILE.o" ]; then
        echo "$SRC_FILE was modified. Recompiling..."
        return 0
    fi

    return 1
}

# Function to extract and print performance metrics
print_performance_metrics() {
    echo "Performance Metrics for $1:"
    grep -E "Packet Loss|Throughput|Energy|Overhead Efficiency" "$2"
    echo "------------------------"
}

# Move to NS-3 directory
cd $NS3_DIR || { echo "NS-3 directory not found. Exiting..."; exit 1; }

# Compile only if the files have been modified
echo "Checking for modified source files..."

MODIFIED=0

if check_modified "backoff-modifed"; then
    MODIFIED=1
fi

if check_modified "olsr-hello"; then
    MODIFIED=1
fi

if check_modified "tcp-server"; then
    MODIFIED=1
fi

if [ $MODIFIED -eq 1 ]; then
    echo "Changes detected. Compiling..."
    ./waf build
    if [ $? -ne 0 ]; then
        echo "Build failed. Exiting..."
        exit 1
    fi
else
    echo "No changes detected. Skipping compilation."
fi

# Run the simulations and capture output
echo "Running Backoff Modified Simulation..."
./waf --run "scratch/backoff-modifed" > backoff_output.txt 2>&1
print_performance_metrics "Backoff Modified" backoff_output.txt

echo "Running OLSR Hello Simulation..."
./waf --run "scratch/olsr-hello" > olsr_output.txt 2>&1
print_performance_metrics "OLSR Hello" olsr_output.txt

echo "Running TCP Server Simulation..."
./waf --run "scratch/tcp-server" > tcp_output.txt 2>&1
print_performance_metrics "TCP Server" tcp_output.txt

echo "All simulations executed successfully."
