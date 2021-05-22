export ASAN_OPTIONS=fast_unwind_on_malloc=0 CXXFLAGS="-fsanitize=address,undefined"

#-fno-sanitize=vptr