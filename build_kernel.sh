
#!/bin/bash

export ARCH=arm64
export PATH=/home/kali/Android/Toolchains/aarch64-linux-android-4.9/bin:$PATH

mkdir out

make -C $(pwd) O=$(pwd)/out CROSS_COMPILE=aarch64-linux-android- KCFLAGS=-mno-android gts4lwifi_eur_open_defconfig
make -j8 -C $(pwd) O=$(pwd)/out CROSS_COMPILE=aarch64-linux-android- KCFLAGS=-mno-android
 
cp out/arch/arm64/boot/Image $(pwd)/arch/arm64/boot/Image



