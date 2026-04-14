from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory
from launch_ros.actions import Node
import os

def generate_launch_description():
    use_sim_time_arg = DeclareLaunchArgument(
        'use_sim_time',
        default_value='true',
        description='Use sim time'
    )
    ######## Environment Selection ##########
    environments_pkg_dir = get_package_share_directory('environments')

    # Declare a launch argument for the environment
    # Available options: forest_gz, office_gz, maze_gz
    selected_environment = DeclareLaunchArgument(
        'environment_name', default_value='maze_gz',
        description="Select the environment: forest_gz, office_gz, maze_gz"
    )
    bring_environment_launch = os.path.join(
        environments_pkg_dir, 'launch', 'bringup_{}.launch.py'.format('office_gz')
    )
    bring_environment = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(bring_environment_launch)
    )

    ######## Robot Selection ##########
    robots_pkg_dir = get_package_share_directory('robots')

    # Declare a launch argument for the robot
    # Available options: turtlebot3, drone
    selected_robot = DeclareLaunchArgument(
        'robot_name', default_value='turtlebot3',
        description="Select the robot: turtlebot3, drone"
    )
    spawn_robot_launch = os.path.join(
        robots_pkg_dir, 'launch', 'spawn_{}.launch.py'.format('turtlebot3')
    )
    spawn_robot = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(spawn_robot_launch)
    )


    rviz_bringup = Node(

        package='rviz2',
        executable='rviz2',
        name='rviz2',
        output='screen',
        arguments=['-d', os.path.join(get_package_share_directory('slam'), 'config', 'view_map.rviz')]
    )

    mapping = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(get_package_share_directory('slam_toolbox'), 'launch', 'online_async_launch.py')
        ),
        launch_arguments={'use_sim_time': 'true'}.items()
    )



    ########## Launch Description ##########
    ld = LaunchDescription()

    # Add launch arguments for dynamic selection
    ld.add_action(selected_environment)
    ld.add_action(selected_robot)

    # Add the selected environment and robot to the launch description
    ld.add_action(bring_environment)
    ld.add_action(rviz_bringup)
    ld.add_action(spawn_robot)
    ld.add_action(mapping)

    return ld
