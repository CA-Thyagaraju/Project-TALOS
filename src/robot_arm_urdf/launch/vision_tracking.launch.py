from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    """Launch the shared webcam-to-MoveIt tracking pipeline."""
    return LaunchDescription([
        Node(
            package="tf2_ros",
            executable="static_transform_publisher",
            name="base_to_camera",
            arguments=[
                "0.5", "-0.5", "0.5", "1.5708", "0.0", "0.0",
                "base_link", "camera_link",
            ],
        ),
        Node(
            package="ros2_vision_cpp",
            executable="cam_control_cpp",
            name="marker_follower",
            output="screen",
            parameters=[{"pixel_scale": 600.0}],
        ),
        Node(
            package="ros2_vision_cpp",
            executable="goal_pose_publisher",
            name="goal_pose_publisher",
            output="screen",
        ),
    ])
