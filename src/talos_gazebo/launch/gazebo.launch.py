from launch import LaunchDescription
from launch.actions import ExecuteProcess, IncludeLaunchDescription, TimerAction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory
from launch_ros.actions import Node
import os

def generate_launch_description():

    pkg_share = get_package_share_directory("talos_gazebo")

    world = os.path.join(
        pkg_share,
        "worlds",
        "empty.sdf"
    )

    gazebo = ExecuteProcess(
        cmd=[
            "gz",
            "sim",
            world
        ],
        output="screen"
    )

    moveit_pkg = get_package_share_directory(
        "robot_arm_moveit_config"
    )

    rsp = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                moveit_pkg,
                "launch",
                "rsp.launch.py"
            )
        )
    )

    spawn_robot = TimerAction(
        period=2.0,
        actions=[
            Node(
                package="ros_gz_sim",
                executable="create",
                arguments=[
                    "-topic",
                    "/robot_description",
                    "-name",
                    "talos",
                ],
                output="screen",
            )
        ],
    )

    return LaunchDescription([
        gazebo,
        rsp,
        spawn_robot
    ])