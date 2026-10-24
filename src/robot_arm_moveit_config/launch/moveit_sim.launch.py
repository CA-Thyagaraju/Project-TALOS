from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
from moveit_configs_utils import MoveItConfigsBuilder


def generate_launch_description():
    vision_pkg = get_package_share_directory("robot_arm_urdf")
    moveit_config = (
        MoveItConfigsBuilder("robot_arm_urdf", package_name="robot_arm_moveit_config")
        .robot_description(file_path="config/robot_arm_urdf.gz.urdf.xacro")
        .to_moveit_configs()
    )

    return LaunchDescription([
        Node(
            package="moveit_ros_move_group",
            executable="move_group",
            output="screen",
            parameters=[
                moveit_config.to_dict(),
                {
                    "use_sim_time": True,
                    "publish_robot_description": True,
                    "publish_robot_description_semantic": True,
                },
            ],
        ),
        Node(
            package="rviz2",
            executable="rviz2",
            name="rviz",
            output="log",
            arguments=[
                "-d",
                str(vision_pkg + "/config/vision_control_base_config.rviz"),
            ],
            parameters=[
                moveit_config.robot_description,
                moveit_config.robot_description_semantic,
                moveit_config.robot_description_kinematics,
                moveit_config.planning_pipelines,
                moveit_config.joint_limits,
                {"use_sim_time": True},
            ],
        ),
    ])
