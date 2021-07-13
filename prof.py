perf record - g - o ${outputfile} ${input_executable}
perf script | c++filt | gprof2dot - f perf | dot - Tpng - o output.png
#valgrind, cachegrind

for k in [k.split("::")[1] for k in mdc.keys()]:
    print("".join([m.capitalize() for m in k.split("_")]))
