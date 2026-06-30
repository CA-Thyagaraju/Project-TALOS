import os
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    # Package names based on your environment
    pkg_moveit = get_package_share_directory('robot_arm_moveit_config')
    pkg_current = get_package_share_directory('robot_arm_urdf')
    rviz_config_path = os.path.join(pkg_current, 'config', 'vision_control_base_config.rviz')
    
    return LaunchDescription([
        # 1. Start MoveIt Demo (includes RViz, move_group, and Fake ros2_control hardware)
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(os.path.join(pkg_moveit, 'launch', 'demo.launch.py')),
            launch_arguments={'rviz_config': rviz_config_path}.items()
        ),

        # 2. Camera Transform: 0.5m in front, 0.5m up, facing forward parallel with base_link
        Node(
            package='tf2_ros',
            executable='static_transform_publisher',
            name='base_to_camera',
            arguments=['0.5', '-0.5', '0.5', '1.5708', '0.0', '0.0', 'base_link', 'camera_link']
        ),

        # 3. Your Vision Node (Processes camera feed and publishes /target_arm_pose)
        Node(
            package='ros2_vision_cpp',
            executable='cam_control_cpp',  # Must match entry point in CMakeLists
            name='marker_follower',
            output='screen',
            parameters=[{'pixel_scale': 600.0}]
        ),       

        # # 4. NEW: Your MoveIt Tracking Node (Subscribes to /target_arm_pose and moves the joints)
        Node(
            package='ros2_vision_cpp',
            executable='goal_pose_publisher', # Must match entry point in CMakeLists
            name='goal_pose_publisher',
            output='screen'
        )
    ])