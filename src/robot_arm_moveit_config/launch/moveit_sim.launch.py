from launch import LaunchDescription
from launch_ros.actions import Node
from moveit_configs_utils import MoveItConfigsBuilder

def generate_launch_description():

    moveit_config = (
        MoveItConfigsBuilder(
            "robot_arm_urdf",
            package_name="robot_arm_moveit_config",
        )
        .to_moveit_configs()
    )

    ld = LaunchDescription()

    ld.add_action(
        Node(
            package="moveit_ros_move_group",
            executable="move_group",
            output="screen",
            parameters=[
                moveit_config.to_dict(),
                {"use_sim_time": True},
            ],
        )
    )

    rviz_config = (
        moveit_config.package_path / "config" / "moveit.rviz"
    )

    ld.add_action(
        Node(
            package="rviz2",
            executable="rviz2",
            name="rviz",
            output="log",
            arguments=["-d", str(rviz_config)],
            parameters=[
                moveit_config.robot_description,
                moveit_config.robot_description_semantic,
                moveit_config.robot_description_kinematics,
                moveit_config.planning_pipelines,
                moveit_config.joint_limits,
                {"use_sim_time": True},
            ],
        )
    )

    return ld