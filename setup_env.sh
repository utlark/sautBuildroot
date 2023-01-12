BR2_EXTERNAL="sautExternal"
BR2_DL_DIR="$BR2_EXTERNAL/downloads"
TARGET=""

function set_target() {
    if [ $# -ne 1 ]; then
        echo "Usage: set_target TARGET"
        return
    fi

    TARGET=$1
    echo "Set target: $TARGET"

    BULDROOT_OUTPUT="$BR2_EXTERNAL/output/$TARGET"
    m "${TARGET}_defconfig"
}

function get_target() {
    echo "Target: $TARGET"
}

function m() {
    if [ -z $TARGET ]; then
        echo "Error: need to set target"
        set_target
        return
    fi

    make \
        BR2_EXTERNAL="$PWD/$BR2_EXTERNAL" \
        BR2_DL_DIR="$PWD/$BR2_DL_DIR" \
        O="$PWD/$BULDROOT_OUTPUT" \
        -C "$PWD/buildroot-2022.02.4" \
        -j$(nproc) \
        "$@"
}
