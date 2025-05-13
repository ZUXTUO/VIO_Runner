#!/usr/bin/env bash
set -euo pipefail

# 并行编译使用的线程数
JOBS=$(nproc)

# 要安装的软件版本
OPENCV_VER="3.4"
COLMAP_VER="3.6"
CMAKE_VER="3.22.0"

# 工作目录（home）
WORKDIR="$HOME"
CMAKE_PREFIX="$WORKDIR/cmake-${CMAKE_VER}"

# -------------------------------------------------------------------
# 1) 临时安装 CMake ${CMAKE_VER}
# -------------------------------------------------------------------
if [ ! -x "${CMAKE_PREFIX}/bin/cmake" ]; then
  echo "=== 安装临时 CMake ${CMAKE_VER} ==="
  cd "${WORKDIR}"
  wget -qO cmake-${CMAKE_VER}-linux-x86_64.tar.gz \
     https://github.com/Kitware/CMake/releases/download/v${CMAKE_VER}/cmake-${CMAKE_VER}-linux-x86_64.tar.gz
  mkdir -p "${CMAKE_PREFIX}"
  tar --strip-components=1 -zxf cmake-${CMAKE_VER}-linux-x86_64.tar.gz -C "${CMAKE_PREFIX}"
  rm cmake-${CMAKE_VER}-linux-x86_64.tar.gz
else
  echo "-> 已存在临时 CMake ${CMAKE_VER}"
fi
export PATH="${CMAKE_PREFIX}/bin:${PATH}"

# -------------------------------------------------------------------
# 2) 安装基础依赖
# -------------------------------------------------------------------
echo "=== 更新软件源并安装基础依赖 ==="
sudo apt update
sudo apt upgrade -y
sudo apt install -y \
  build-essential git pkg-config wget unzip tar dos2unix \
  gcc-8 g++-8 \
  libboost-dev libboost-all-dev \
  libeigen3-dev \
  libceres-dev \
  libgflags-dev \
  libsqlite3-dev \
  libglew-dev libglfw3-dev \
  libcgal-dev libmetis-dev \
  libflann-dev libopenmpi-dev \
  libgtk-3-dev \
  libavcodec-dev libavformat-dev libswscale-dev \
  libavutil-dev libraw1394-dev \
  libv4l-dev libxvidcore-dev libx264-dev \
  libjpeg-dev libpng-dev libtiff-dev \
  gfortran openexr libopenexr-dev libatlas-base-dev \
  python3-dev python3-numpy \
  libtbb2 libtbb-dev \
  libdc1394-22-dev \
  qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools \
  ffmpeg \
  openjdk-8-jdk \
  nvidia-cuda-toolkit \
  fonts-arphic-ukai fonts-arphic-uming fonts-wqy-zenhei fonts-wqy-microhei

sudo apt install -y python3-pip
pip3 install --upgrade wheel
sudo apt -y autoremove

# -------------------------------------------------------------------
# 3) 编译安装 OpenCV
# -------------------------------------------------------------------
check_opencv_installed() {
  pkg-config --modversion opencv 2>/dev/null | grep -q "^${OPENCV_VER}"
}
if check_opencv_installed; then
  echo "-> 已检测到 OpenCV ${OPENCV_VER}，跳过"
else
  echo "=== 编译安装 OpenCV ${OPENCV_VER} + contrib ==="
  cd "${WORKDIR}"
  [ ! -d opencv ] && git clone --depth 1 --branch "${OPENCV_VER}" https://github.com/opencv/opencv.git
  [ ! -d opencv_contrib ] && git clone --depth 1 --branch "${OPENCV_VER}" https://github.com/opencv/opencv_contrib.git
  cd opencv && mkdir -p build && cd build

  cmake .. \
    -D CMAKE_BUILD_TYPE=Release \
    -D CMAKE_INSTALL_PREFIX=/usr/local \
    -D OPENCV_EXTRA_MODULES_PATH="${WORKDIR}/opencv_contrib/modules" \
    -D BUILD_opencv_python3=ON \
    -D BUILD_EXAMPLES=ON \
    -D WITH_CUDA=OFF \
    -D WITH_OPENGL=ON \
    -D WITH_TBB=ON \
    -D WITH_V4L=ON \
    -D WITH_QT=OFF \
    -D WITH_GTK=ON \
    -D BUILD_TESTS=OFF \
    -D BUILD_opencv_hdf=OFF

  make -j"${JOBS}"
  sudo make install
  sudo ldconfig
  echo "-> OpenCV ${OPENCV_VER} 安装完成"
fi

# -------------------------------------------------------------------
# 4) 编译安装 Pangolin
# -------------------------------------------------------------------
check_pangolin_installed() {
  ldconfig -p | grep -q libpangolin
}
if check_pangolin_installed; then
  echo "-> 已检测到 Pangolin，跳过"
else
  echo "=== 编译安装 Pangolin ==="
  cd "${WORKDIR}"
  [ ! -d Pangolin ] && git clone https://github.com/stevenlovegrove/Pangolin.git
  cd Pangolin && mkdir -p build && cd build

  cmake .. \
    -DBUILD_PANGOLIN_PYTHON=OFF \
    -DCMAKE_CXX_FLAGS="-Wno-deprecated-copy -Wno-error" \
    -DPangolin_DIR=/usr/local/lib/cmake/Pangolin

  make -j"${JOBS}"
  sudo make install
  echo "-> Pangolin 安装完成"
fi

# -------------------------------------------------------------------
# 5) 编译安装 FreeImage (danoli3)
# -------------------------------------------------------------------
check_freeimage_installed() {
  pkg-config --exists freeimage
}
if check_freeimage_installed; then
  echo "-> 系统 FreeImage 检测到，跳过"
else
  echo "=== 编译安装 FreeImage (danoli3) ==="
  cd "${WORKDIR}"
  [ ! -d FreeImage ] && git clone https://github.com/danoli3/FreeImage.git
  cd FreeImage
  cmake . -B cmake-build
  cmake --build cmake-build
  cd cmake-build
  cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
  make -j"${JOBS}"
  sudo make install
  echo "-> FreeImage (danoli3) 安装完成"
fi

# -------------------------------------------------------------------
# 6) 编译安装 COLMAP
# -------------------------------------------------------------------
check_colmap_installed() {
  colmap --version 2>/dev/null | grep -q "^${COLMAP_VER}"
}
if check_colmap_installed; then
  echo "-> 已检测到 COLMAP ${COLMAP_VER}，跳过"
else
  echo "-> !先卸载glog"
  sudo rm -rf /usr/local/include/glog
  sudo rm -f /usr/local/lib/libglog*
  sudo ldconfig

  echo "=== 编译安装 COLMAP ${COLMAP_VER} ==="
  sudo apt install -y libfreeimage-dev
  
  # ls /usr/local/include/FreeImage.h
  # ls ls /usr/lib/x86_64-linux-gnu/libfreeimage.so*

  cd "${WORKDIR}"
  wget -qO colmap-${COLMAP_VER}.zip \
     https://github.com/colmap/colmap/archive/refs/tags/${COLMAP_VER}.zip
  unzip -q -o colmap-${COLMAP_VER}.zip
  rm -rf colmap-${COLMAP_VER}/build
  cd colmap-${COLMAP_VER} && mkdir build && cd build

  CC=gcc-8 CXX=g++-8 cmake .. \
    -DFreeImage_DIR=/usr/local/lib/cmake/FreeImage \
    -DCMAKE_PREFIX_PATH="/usr/lib/x86_64-linux-gnu" \
    -DCMAKE_INCLUDE_PATH="/usr/include" \
    -DCUDA_ENABLED=ON

  make -j"${JOBS}"
  sudo make install
  echo "-> COLMAP ${COLMAP_VER} 安装完成"

  # sudo apt remove -y libfreeimage-dev libfreeimage3
  # sudo apt autoremove -y
  # sudo rm -rf /usr/include/FreeImage.h /usr/lib/x86_64-linux-gnu/libfreeimage*
  # sudo ldconfig
fi

# -------------------------------------------------------------------
# 7) 编译安装 Google glog
# -------------------------------------------------------------------
# echo "=== 编译安装 glog ==="
# sudo apt remove -y libglog-dev || true

# cd "${WORKDIR}"
# sudo rm -rf glog
# echo "-> 克隆 glog 源码"
# git clone https://github.com/google/glog.git
# cd glog
# git checkout v0.3.0

# ./configure --prefix=/usr/local --enable-shared
# make -j$(nproc)
# sudo make install

# sudo ldconfig
# echo "-> glog 安装完成"

sudo apt install libgoogle-glog-dev

# -------------------------------------------------------------------
# 8) 编译 vio 工程
# -------------------------------------------------------------------
echo "=== 编译 vio ==="
cd ~
# unzip -q -o vio.zip

VIO_DIR="${WORKDIR}/VIO_Runner/vio_project"
if [ -d "${VIO_DIR}" ]; then
  cd "${VIO_DIR}"
  rm -rf build && mkdir build && cd build

  cmake .. \
    -DGlog_DIR=/usr/local/lib/cmake/glog \
    -DFreeImage_DIR=/usr/local/lib/cmake/FreeImage \
    -DCMAKE_PREFIX_PATH=/usr/local

  make -j"${JOBS}"
  echo "-> vio 编译完成"
else
  echo "Error: 未找到 ${VIO_DIR}"
  exit 1
fi

# -------------------------------------------------------------------
# 9) 下载 vocab_tree 文件
# -------------------------------------------------------------------
echo "=== 下载 vocab_tree 文件到 WorkSpace/Vocab ==="

VOCAB_DIR="${WORKDIR}/WorkSpace/Vocab"
mkdir -p "${VOCAB_DIR}"
cd "${VOCAB_DIR}"

VOCAB_URL_BASE="https://github.com/colmap/colmap/releases/download/3.11.1"
VOCAB_FILES=(
  "vocab_tree_flickr100K_words32K.bin"
  "vocab_tree_flickr100K_words256K.bin"
  "vocab_tree_flickr100K_words1M.bin"
)

for FILE in "${VOCAB_FILES[@]}"; do
  if [ -f "${FILE}" ]; then
    echo "-> 已存在 ${FILE}，跳过下载"
  else
    echo "-> 正在下载 ${FILE}..."
    wget -q "${VOCAB_URL_BASE}/${FILE}"
  fi
done

echo "-> 所有 vocab_tree 文件已准备完毕"


echo "=== 全部步骤执行完毕 ==="
