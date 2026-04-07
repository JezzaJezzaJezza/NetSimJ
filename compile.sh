if [ ! -f src/SimConfig.hpp ] || [ "$1" = "--configure" ]; then
    python3 configure.py || exit 1
fi
rm -rf build
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
