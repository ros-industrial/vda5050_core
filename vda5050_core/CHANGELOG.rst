^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package vda5050_core
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

0.1.0 (2026-10-28)
-----------
* Client Library

  * feat: provide initial adapter interface along with rmf migration guide (`#71 <https://github.com/ros-industrial/vda5050_core/issues/71>`_, `#80 <https://github.com/ros-industrial/vda5050_core/issues/80>`_, `#86 <https://github.com/ros-industrial/vda5050_core/issues/86>`_)

    * docs: readme and how-to guides for client adapter (`#87 <https://github.com/ros-industrial/vda5050_core/issues/87>`_)
    * feat: add order validator and acceptance strategy (`#31 <https://github.com/ros-industrial/vda5050_core/issues/31>`_, `#63 <https://github.com/ros-industrial/vda5050_core/issues/63>`_)
    * feat: add AGVContext and execution tracking resource (`#62 <https://github.com/ros-industrial/vda5050_core/issues/62>`_, `#66 <https://github.com/ros-industrial/vda5050_core/issues/66>`_)
    * feat: add concrete execution data types (`#53 <https://github.com/ros-industrial/vda5050_core/issues/53>`_)
    * feat: add StateManager (`#14 <https://github.com/ros-industrial/vda5050_core/issues/14>`_)

* Master Library

  * feat: add initial implementation of VDA5050 Master (`#20 <https://github.com/ros-industrial/vda5050_core/issues/20>`_, `#45 <https://github.com/ros-industrial/vda5050_core/issues/45>`_, `#59 <https://github.com/ros-industrial/vda5050_core/issues/59>`_, `#78 <https://github.com/ros-industrial/vda5050_core/issues/78>`_)

    * feat: add order lifecycle, stitcher, instant actions, mode handling, and pose_view (`#69 <https://github.com/ros-industrial/vda5050_core/issues/69>`_, `#82 <https://github.com/ros-industrial/vda5050_core/issues/82>`_)
    * feat: master-side event detectors and heartbeat hardening (`#58 <https://github.com/ros-industrial/vda5050_core/issues/58>`_)
    * feat: add traversability, action-conflict, factsheet, and mode validators (`#56 <https://github.com/ros-industrial/vda5050_core/issues/56>`_)
    * feat: add schema and pre-send validators (`#55 <https://github.com/ros-industrial/vda5050_core/issues/55>`_)

* Core Library

  * feat(transport): add ProtocolAdapter `#48 <https://github.com/ros-industrial/vda5050_core/issues/48>`_, `#74 <https://github.com/ros-industrial/vda5050_core/issues/74>`_)
  * feat(transport): add MQTT client interface with Paho as an default implementation (`#1 <https://github.com/ros-industrial/vda5050_core/issues/1>`_, `#17 <https://github.com/ros-industrial/vda5050_core/issues/17>`_, `#19 <https://github.com/ros-industrial/vda5050_core/issues/19>`_)
  * feat(types): create C++ structs for VDA5050 2.0.0 spec (`#6 <https://github.com/ros-industrial/vda5050_core/issues/6>`_)
  * feat(serialization): add common (de)serializers for C++ structs and ROS 2 msgs (`#10 <https://github.com/ros-industrial/vda5050_core/issues/10>`_, `#24 <https://github.com/ros-industrial/vda5050_core/issues/24>`_)
  * feat(layout): add LIF topology graph and JSON loader (`#54 <https://github.com/ros-industrial/vda5050_core/issues/54>`_)
  * feat(execution): add execution utilities (`#28 <https://github.com/ros-industrial/vda5050_core/issues/28>`_, `#36 <https://github.com/ros-industrial/vda5050_core/issues/36>`_, `#50 <https://github.com/ros-industrial/vda5050_core/issues/50>`_)
  * feat(pybind): refactor pybind code for better compatibility with modern pyproject (`#88 <https://github.com/ros-industrial/vda5050_core/issues/88>`_, `#89 <https://github.com/ros-industrial/vda5050_core/issues/89>`_, `#90 <https://github.com/ros-industrial/vda5050_core/issues/90>`_)
  * fix(client): fix execution lifecycle handling for navigation and actions (`#91 <https://github.com/ros-industrial/vda5050_core/issues/91>`_)
  * fix(transport): register message handler before subscribing (`#93 <https://github.com/ros-industrial/vda5050_core/issues/93>`_)

* Others

  * C++ packaging related (`#27 <https://github.com/ros-industrial/vda5050_core/issues/27>`_, `#43 <https://github.com/ros-industrial/vda5050_core/issues/43>`_, `#44 <https://github.com/ros-industrial/vda5050_core/issues/44>`_)
  * CI and Github Actions (`#35 <https://github.com/ros-industrial/vda5050_core/issues/35>`_, `#25 <https://github.com/ros-industrial/vda5050_core/issues/25>`_, `#6 <https://github.com/ros-industrial/vda5050_core/issues/6>`_)

* Contributors: Chen Bainian, Eileen Teoh Yan Zhen, John Arman Abogado, Leah, Saurabh Kamat, Lim Yew Hao, Glenn Tan, Shawn Chan, Tinapat Game Limsila
