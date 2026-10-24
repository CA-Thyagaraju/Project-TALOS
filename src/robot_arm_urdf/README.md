# robot_arm_urdf

This package contains the robot URDF, meshes, RViz setup, and the shared webcam-tracking launch files.

`vision_tracking.launch.py` starts the base-to-camera transform, the pouch tracker, and the MoveIt goal-pose publisher. It is used by both standalone RViz and Gazebo simulation.

Run standalone tracking with fake ros2_control hardware:

```bash
ros2 launch robot_arm_urdf vision_control.launch.py
```

It connects to `robot_arm_moveit_config` for MoveIt and `ros2_vision_cpp` for webcam tracking.
