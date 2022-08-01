BR2_EXTERNAL="sautExternal"
BR2_DL_DIR="$BR2_EXTERNAL/downloads"
TARGET=""
BR_V=""

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

function set_br_version() {
    if [ $# -ne 1 ]; then
        echo "Usage: set_br_version version"
        return
    fi

    if [ -z $TARGET ]; then
        echo "Error: need to set target"
        set_target
        return
    fi

    if [ -z $PWD/$BR2_EXTERNAL/board/$TARGET/ ]; then
        mkdir "$PWD/$BR2_EXTERNAL/board/$TARGET/"
    fi

    echo $1 > "$PWD/$BR2_EXTERNAL/board/$TARGET/br-version"
    echo "Set version: $1"
}

function find_br_version() {
    if [ -z $TARGET ]; then
        echo "Error: need to set target"
        set_target
        return
    fi

    if [ -z $PWD/$BR2_EXTERNAL/board/$TARGET/br-version ]; then
        echo "Error: need to set version"
        set_br_version
        return
    fi

    BR_V="buildroot-$(cat $PWD/$BR2_EXTERNAL/board/$TARGET/br-version)"
    echo "Find version: $BR_V"
    return

    echo "Error: could not find buildroot version"
    return 1
}

function m() {
    if [ -z $TARGET ]; then
        echo "Error: need to set target"
        set_target
        return
    fi

    find_br_version

    make \
        BR2_EXTERNAL="$PWD/$BR2_EXTERNAL" \
        BR2_DL_DIR="$PWD/$BR2_DL_DIR" \
        O="$PWD/$BULDROOT_OUTPUT" \
        -C "$PWD/$BR_V" \
        -j$(nproc) \
        "$@"
}
