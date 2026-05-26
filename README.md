# VDA5050 Library and Support Tools

This repository provides libraries and tools to
- Enable VDA5050 compatibility for AGV/AMRs
- Build custom VDA5050 Master.

# Build and install 
```bash 
colcon build 
echo "source ${PWD}/install/setup.bash" >> ~/.bashrc
```

# Run 
```bash
ros2 run vda5050_core adapter_example tcp://localhost:1883 my_agv```

# Troubleshoot 
if fail try adding `Add: [-std=c++17]` to the .clangd compileFlag

  