# Project TALOS

> **Project TALOS** is an ongoing open-source robotics project inspired by **Tony Stark's workshop robots** from the **MCU**. The long-term goal is to build a Vision-Language-Action (VLA) robotic manipulator that can understand natural language, perceive its surroundings, and interact with the real world. Built on ROS 2, the project brings together computer vision, motion planning, and AI—because someone has to build **DUM-E** before they can build **JARVIS**.

Project TALOS currently tracks a red pouch with a webcam and moves a six-axis robot arm towards the detected target. The same vision pipeline works in two modes:

- **Standalone RViz:** MoveIt uses fake ros2_control hardware for quick vision and motion-planning tests.
- **Gazebo simulation:** Gazebo controls the arm through `gz_ros2_control`; MoveIt, RViz, the webcam tracker, and the target marker start together.

## Structure

```text
Project-TALOS/
├── src/
│   ├── robot_arm_urdf/          # Robot model and shared vision launches
│   ├── robot_arm_moveit_config/ # MoveIt and controller configuration
│   ├── ros2_vision_cpp/         # Webcam pouch tracker and goal publisher
│   └── talos_gazebo/            # Gazebo world and simulation launch
├── build/
├── install/
└── log/
```

## Getting Started

```bash
colcon build
source install/setup.bash
```

## Run

Standalone RViz tracking:

```bash
ros2 launch robot_arm_urdf vision_control.launch.py
```

Gazebo tracking:

```bash
ros2 launch talos_gazebo gazebo.launch.py
```

The webcam tracker publishes the target pose on `/target_arm_pose` and the RViz visual marker on `/visualization_marker`.
