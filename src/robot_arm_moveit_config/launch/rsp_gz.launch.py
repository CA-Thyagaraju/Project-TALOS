from moveit_configs_utils import MoveItConfigsBuilder
from moveit_configs_utils.launches import generate_rsp_launch


def generate_launch_description():
    moveit_config = (
        MoveItConfigsBuilder("robot_arm_urdf", package_name="robot_arm_moveit_config")
        .robot_description(file_path="config/robot_arm_urdf.gz.urdf.xacro")
        .to_moveit_configs()
    )
    return generate_rsp_launch(moveit_config)
