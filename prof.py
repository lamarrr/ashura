perf record
perf script | c++filt | gprof2dot -f perf | dot -Tpng -o output.png