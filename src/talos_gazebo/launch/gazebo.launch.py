from launch import LaunchDescription
from launch.actions import ExecuteProcess, IncludeLaunchDescription, TimerAction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory
from launch_ros.actions import Node
import os

def generate_launch_description():

    pkg_share = get_package_share_directory("talos_gazebo")
    world = "/usr/share/gz/gz-sim8/worlds/empty.sdf"

    gazebo = ExecuteProcess(
        cmd=[
            "gz",
            "sim",
            "-r",
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
                    "-z",
                    "0.3",
                ],
                output="screen",
            )
        ],
    )

    load_controllers = TimerAction(
    period=4.0,
    actions=[
        Node(
            package="controller_manager",
            executable="spawner",
            arguments=["joint_state_broadcaster"],
            output="screen",
        ),
        Node(
            package="controller_manager",
            executable="spawner",
            arguments=["arm_controller"],
            output="screen",
        ),
        Node(
            package="controller_manager",
            executable="spawner",
            arguments=["gripper_controller"],
            output="screen",
        ),
    ],
)


    clock_bridge = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        arguments=[
            "/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock"
        ],
        output="screen",
    )

    return LaunchDescription([
        gazebo,
        rsp,
        clock_bridge,
        spawn_robot,
        load_controllers,
    ])