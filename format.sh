
set -e

clang-format -i --style=file ashura/std/*.h
clang-format -i --style=file ashura/std/*.cc
clang-format -i --style=file ashura/gpu/*.h
clang-format -i --style=file ashura/gpu/*.cc
clang-format -i --style=file ashura/engine/*.h
clang-format -i --style=file ashura/engine/*.cc