# talos_gazebo

This package starts the TALOS arm in Gazebo with `gz_ros2_control`, the required controllers, Gazebo MoveIt, RViz, and the shared webcam pouch-tracking pipeline.

It also spawns a fixed overhead camera model at the same pose used by the RViz camera transform. The webcam tracker still uses the physical default webcam; the Gazebo camera is currently a visual simulation object. The pouch target is shown as an RViz marker, not yet as a Gazebo-world object.

Run the full Gazebo tracking setup:

```bash
ros2 launch talos_gazebo gazebo.launch.py
```

It connects to `robot_arm_moveit_config` for Gazebo MoveIt and `robot_arm_urdf` for the shared vision, marker, and camera transform setup.
