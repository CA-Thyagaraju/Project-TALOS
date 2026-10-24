# robot_arm_moveit_config

This package contains the MoveIt planning configuration, controller settings, and RViz configurations for the TALOS arm.

The default robot description uses fake ros2_control hardware for standalone RViz. The Gazebo robot description uses `gz_ros2_control` and is started by `talos_gazebo`.

Useful launches:

```bash
ros2 launch robot_arm_moveit_config demo.launch.py
ros2 launch robot_arm_moveit_config moveit_sim.launch.py
```

`demo.launch.py` is for standalone fake hardware. `moveit_sim.launch.py` is used by the Gazebo launch and should normally not be started separately.
