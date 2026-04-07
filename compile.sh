CUDA_FLAG=""

for arg in "$@"; do
    case "$arg" in
        --configure)
            python3 configure.py || exit 1
            ;;
        --cuda)
            CUDA_FLAG="-DENABLE_CUDA=ON"
            ;;
    esac
done

if [ ! -f src/SimConfig.hpp ]; then
    python3 configure.py || exit 1
fi

rm -rf build
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON $CUDA_FLAG
cmake --build build
