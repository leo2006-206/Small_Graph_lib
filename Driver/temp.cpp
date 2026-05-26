#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

void my_heavy_function() {
    // Simulating the workload you actually care about
    // for (volatile int i = 0; i < 500000000; i++) {
    //     // do work
    // }
}

int main() {
    printf("Program starting. Waiting for perf to attach...\n");

    // 1. Open the control FIFO.
    // This will block until the `perf` tool opens the other end,
    // which safely synchronizes your program with perf!
    int ctl_fd = open("./perf_control.fifo", O_WRONLY);
    if (ctl_fd == -1) {
        perror("Failed to open control FIFO. Did you create it?");
        exit(EXIT_FAILURE);
    }

    // --- START TARGETED PROFILING ---
    // 2. Tell the external perf command to start recording
    // write(ctl_fd, "enable\n", 7);
    
    my_heavy_function(); // <--- Only this is profiled

    // 3. Tell the external perf command to stop recording
    // write(ctl_fd, "disable\n", 8);
    // --- END TARGETED PROFILING ---

    close(ctl_fd);
    printf("Work done. Exiting.\n");
    return 0;
}