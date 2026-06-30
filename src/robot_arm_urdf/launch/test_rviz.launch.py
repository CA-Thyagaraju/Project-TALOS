import os
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    # Direct path to your URDF file in the source directory
    urdf_path = os.path.expanduser('~/ros2_ws/src/robot_arm_urdf/urdf/robot_arm_urdf.urdf')
    with open(urdf_path, 'r') as f:
        robot_desc = f.read()

    return LaunchDescription([
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            parameters=[{'robot_description': robot_desc}]
        ),
        Node(
            package='joint_state_publisher_gui',
            executable='joint_state_publisher_gui'
        ),
        Node(
            package='rviz2',
            executable='rviz2'
        )
    ])