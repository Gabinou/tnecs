    #!/bin/bash
# Written by Gabriel Taillon on January 3rd 2025
# Analyzing cache usage with perf

# --- callgraph ---
perf record -F 99 -g ./test
perf script | gprof2dot -f perf | dot -Tpng -o output.png

# --- cache misses ---
# Check with accessible fields:
sudo perf list cache
# Disable nim watchdog:  echo 0 > /proc/sys/kernel/nmi_watchdog
# Re-enable nim watchdog:  echo 1 > /proc/sys/kernel/nmi_watchdog

perf stat -B -r 50 -e cache-references,cache-misses,cycles,instructions,branches,faults,migrations,L1-dcache-loads,L1-dcache-load-misses test
valgrind --tool=cachegrind ./test