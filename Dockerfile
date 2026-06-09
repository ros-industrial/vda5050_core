# syntax=docker/dockerfile:1
#-----------------------
# Stage 1 - Dependencies
#-----------------------
FROM ros:humble-ros-base-jammy

RUN apt-get update \
  && apt-get install -y \
    cmake \
    curl \
    git \
    python3-colcon-common-extensions \
    python3-vcstool \
    wget \
    python3-pip \
    clang lldb lld  openssh-client\
    libmosquitto-dev mosquitto-clients \
  && pip3 install requests paho-mqtt pydantic rich\
  && rm -rf /var/lib/apt/lists/* 

# setup keys
RUN apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys D2486D2DD83DB69272AFE98867170598AF249743

# setup sources.list
RUN . /etc/os-release \
    && echo "deb http://packages.osrfoundation.org/gazebo/$ID-stable `lsb_release -sc` main" > /etc/apt/sources.list.d/gazebo-latest.list

RUN mkdir /colcon_ws
WORKDIR /colcon_ws
RUN mkdir src
RUN rosdep update --rosdistro $ROS_DISTRO

# Temporary way to mount folders
ADD vda5050_fiware /colcon_ws/src/vda5050_fiware

ENV DEBIAN_FRONTEND=noninteractive
# COPY vda5050.repos vda5050.repos
# RUN vcs import src < vda5050.repos \
#     && apt-get update \
#     && apt-get upgrade -y \
#     && rosdep update \
#     && rosdep install --from-paths src --ignore-src --rosdistro $ROS_DISTRO -yr \
#     && rm -rf /var/lib/apt/lists/*

#-----------------
# Stage 2 - build
#-----------------

# compile rmf_demo_panel gui
# use wget
# RUN npm install --prefix src/demonstrations/rmf_demos/rmf_demos_panel/rmf_demos_panel/static/ \
  # && npm run build --prefix src/demonstrations/rmf_demos/rmf_demos_panel/rmf_demos_panel/static/

# # Gitlab Authentication
# RUN --mount=type=ssh,id=github_ssh_key pip wheel \
#     --no-cache \
# #     --requirement requirements.txt \
# # --wheel-dir=/app/wheels

# # Clone dependencies
# RUN cd /colcon_ws/src && git clone git@gitlab.com:ROSI-AP/rmf2/fiware_api.git && cd /colcon_ws


ADD fiware_api /colcon_ws/src/fiware_api
ADD rmf2_agv   /colcon_ws/src/rmf2_agv
ADD rmf2_building /colcon_ws/src/rmf2_building
ADD rmf2_tasks /colcon_ws/src/rmf2_tasks


# colcon compilation
RUN . /opt/ros/$ROS_DISTRO/setup.sh \
  && cd /colcon_ws && colcon build --cmake-args -DCMAKE_BUILD_TYPE=Release


#----------
# Stage 3
#----------

CMD ["bash"]
