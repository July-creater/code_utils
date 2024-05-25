# Use the NVIDIA CUDA 12.1.0 base image with Ubuntu 20.04
FROM nvidia/cuda:12.1.0-base-ubuntu20.04

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive

# Install essential packages
RUN apt-get update \
    && apt-get upgrade -y \ 
    && apt-get install -y \
    build-essential \
    wget \
    curl \
    git \
    cmake \
    vim \
    libeigen3-dev \
    libyaml-cpp-dev \
    pkg-config \
    libvtk7-dev \
    libusb-1.0-0-dev \
    libmetis-dev \
    libspdlog-dev \
    libsuitesparse-dev \
    qtdeclarative5-dev \
    qt5-qmake \
    libqglviewer-dev-qt5 \
    libgoogle-glog-dev \
    libgflags-dev \
    libgtest-dev \
    libatlas-base-dev \
# Install OpenCV
    libopencv-dev \
# Install PCL
    libpcl-dev \
    && rm -rf /var/lib/apt/lists/*

# Install fmt v0.11.0 (required by Sophus)
RUN git clone https://github.com/fmtlib/fmt.git \
    && cd fmt \
    && mkdir build && cd build \
    && cmake .. \
    && make -j$(nproc) \
    && make install \
    && cd ../.. \
    && rm -rf fmt

# Install latest Sophus 1.22.10
RUN git clone https://github.com/strasdat/Sophus.git \
    && cd Sophus \
    && mkdir build && cd build \
    && cmake .. \
    && make -j$(nproc) \
    && make install \
    && cd ../.. \
    && rm -rf Sophus

# Install robin-map v1.3.0 
RUN git clone https://github.com/Tessil/robin-map.git \
    && cd robin-map \
    && mkdir build && cd build \
    && cmake .. \
    && make -j$(nproc) \
    && make install \
    && cd ../.. \
    && rm -rf robin-map

# Install g2o 20230806_git
RUN git clone https://github.com/RainerKuemmerle/g2o.git \
    && cd g2o \
    && mkdir build && cd build \
    && cmake .. -DBUILD_WITH_MARCH_NATIVE=OFF -DCMAKE_BUILD_TYPE=Release \
    && make -j$(nproc) \
    && make install \
    && cd ../.. \
    && rm -rf g2o

# Install Ceres 2.2.0
RUN git clone https://github.com/ceres-solver/ceres-solver.git \
    && cd ceres-solver \
    && mkdir build && cd build \
    && cmake .. \
    && make -j$(nproc) \
    && make install \
    && cd ../.. \
    && rm -rf ceres-solver

# Clean up
RUN apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Default command
CMD ["bash"]