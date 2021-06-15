perf record -g -o ${outputfile} ${input_executable}
perf script | c++filt | gprof2dot -f perf | dot -Tpng -o output.png
#valgrind, cachegrind