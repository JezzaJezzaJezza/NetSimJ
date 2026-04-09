CUDA_FLAG=""
CONFIGURE=false

for arg in "$@"; do
    case "$arg" in
        --configure) CONFIGURE=true ;;
        --cuda)      CUDA_FLAG="-DENABLE_CUDA=ON" ;;
    esac
done

if $CONFIGURE; then
    if [ -n "$CUDA_FLAG" ]; then
        python3 configure.py --cuda || exit 1
    else
        python3 configure.py || exit 1
    fi
elif [ ! -f src/SimConfig.hpp ]; then
    if [ -n "$CUDA_FLAG" ]; then
        python3 configure.py --cuda || exit 1
    else
        python3 configure.py || exit 1
    fi
fi

rm -rf build
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON $CUDA_FLAG
cmake --build build
