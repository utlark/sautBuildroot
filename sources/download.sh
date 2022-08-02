HiKern="https://github.com/utlark/hisilicon-kernel.git"

function hi_kernel() {
    if [ -z ./kernel ]; then
        mkdir ./kernel
    fi

    git clone "$HiKern" ./kernel/hisilicon-kernel
}

function usage() {
    echo "This script will help you load the source code of the external saut tree."
    echo "Options:"
    echo "hi_kernel  - download HiSilicon kernel"
    echo "imx_kernel - download I.MX kernel"
    echo ""
    echo "fast_boot  - download HiSilicon fast-boot"
    echo "imx_boot   - download I.MX u-boot"
    echo ""
    echo "hi_all     - download HiSilicon kernel and fast-boot"
    echo "imx_all    - download I.MX kernel and u-boot"
}

if declare -f "$1" > /dev/null
then
  # call arguments verbatim
  "$@"
else
  # show a helpful error
  echo "'$1' is not a known function name" >&2
  usage
  exit 1
fi

