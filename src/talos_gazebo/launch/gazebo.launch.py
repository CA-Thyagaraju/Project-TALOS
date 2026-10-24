from launch import LaunchDescription
from launch.actions import ExecuteProcess, IncludeLaunchDescription, TimerAction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory
from launch_ros.actions import Node
import os

def generate_launch_description():

    pkg_share = get_package_share_directory("talos_gazebo")
    world = "/usr/share/gz/gz-sim8/worlds/empty.sdf"
    camera_model = os.path.join(
        pkg_share, "models", "overhead_camera", "model.sdf"
    )

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
    vision_pkg = get_package_share_directory("robot_arm_urdf")

    rsp = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                moveit_pkg,
                "launch",
                "rsp_gz.launch.py"
            )
        )
    )

    moveit = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                moveit_pkg,
                "launch",
                "moveit_sim.launch.py",
            )
        )
    )

    vision_tracking = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                vision_pkg,
                "launch",
                "vision_tracking.launch.py",
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

    # Match the base_link -> camera_link transform used by vision_tracking.
    # The robot base is spawned 0.3 m above the world origin, so the camera
    # is placed at z = 0.3 + 0.5 m in Gazebo world coordinates.
    spawn_camera = TimerAction(
        period=3.0,
        actions=[
            Node(
                package="ros_gz_sim",
                executable="create",
                arguments=[
                    "-world", "empty",
                    "-file", camera_model,
                    "-name", "overhead_camera",
                    "-x", "0.5",
                    "-y", "-0.5",
                    "-z", "0.8",
                    "-Y", "1.5708",
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
        spawn_camera,
        load_controllers,
        moveit,
        vision_tracking,
    ])
