import os
from launch import LaunchDescription
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

        # 2. Webcam pouch tracking, marker publishing, and MoveIt goal publishing.
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                os.path.join(pkg_current, 'launch', 'vision_tracking.launch.py')
            )
        ),
    ])
