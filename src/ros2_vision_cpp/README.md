# ros2_vision_cpp

This package detects the red pouch from the default webcam and turns the detected position into a MoveIt target.

`cam_control_cpp` publishes the camera image, the target pose on `/target_arm_pose`, and RViz markers on `/visualization_marker`. `goal_pose_publisher` listens to the target pose and asks MoveIt to move the arm.

These nodes are started by `robot_arm_urdf` through `vision_tracking.launch.py`. Run the standalone launch instead of starting them manually:

```bash
ros2 launch robot_arm_urdf vision_control.launch.py
```
