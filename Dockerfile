FROM ubuntu:20.04

SHELL ["/bin/bash", "-c"]

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    curl gnupg2 lsb-release software-properties-common \
    build-essential git cmake \
    python3-pip \
    nlohmann-json3-dev \
    libusb-1.0-0-dev \
    && rm -rf /var/lib/apt/lists/*

RUN curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key \
    -o /usr/share/keyrings/ros-archive-keyring.gpg
RUN echo "deb [signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros/ubuntu $(lsb_release -cs) main" \
    > /etc/apt/sources.list.d/ros1.list

RUN apt-get update && apt-get install -y --no-install-recommends \
    ros-noetic-desktop-full \
    && rm -rf /var/lib/apt/lists/*

    WORKDIR /opt

RUN git clone https://github.com/Livox-SDK/Livox-SDK.git && \
    cd Livox-SDK && \
    rm -rf build && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make -j$(nproc) && \
    make install

WORKDIR /ws_livox

RUN mkdir -p src

WORKDIR /ws_livox/src

RUN git clone https://github.com/Livox-SDK/livox_ros_driver.git

WORKDIR /ws_livox

RUN source /opt/ros/noetic/setup.bash && \
    catkin_make

WORKDIR /ros_ws

RUN mkdir -p src

COPY . /ros_ws/src/livox_bag_aggregate

RUN source /opt/ros/noetic/setup.bash && \
    source /ws_livox/devel/setup.bash && \
    catkin_make
    
ARG UID=1000
ARG GID=1000
RUN groupadd -g $GID ros && \
    useradd -m -u $UID -g $GID -s /bin/bash ros

RUN echo "source /opt/ros/noetic/setup.bash" >> ~/.bashrc && \
    echo "source /ws_livox/devel/setup.bash" && >> ~/.bashrc \
    echo "source /ros_ws/devel/setup.bash" >> ~/.bashrc

CMD ["bash"]