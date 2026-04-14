# **Mapping Package**

This package is designed to bring up Robots in Gazebo only (e.g., Turtlebot3 , Sjtu Drone) and start mapping using SLAM.

---

## **Installation**

1. **Install depenedencies**:
    ```bash
    cd ~/robotisim_ws/
    rosdep install --from-paths src --ignore-src -y
    ```

2. **Build the Package**:
   ```bash
   colcon build --packages-select slam
   source install/setup.bash
   ```
---

## **TURTLEBOT3 : Launch Files**
- **2D Mapping using SLAM Toolbox : To Change environment , go to launch file and edit**:
  ```bash
    ros2 launch  slam 2d_mapping_tb3.launch.py
    ros2 run teleop_twist_keyboard teleop_twist_keyboard #(next terminal)
  ```
   - **Load 2D Map : Loads a map into mapserver**:
  ```bash
    ros2 launch slam 2d_map_load_rviz.launch.py
  ```

- **3D mapping**:
  - Run mapping Using RTAB Mapping
  ```bash
    ros2 launch slam 3d_mapping_tb3.launch.py
    ros2 run teleop_twist_keyboard teleop_twist_keyboard #(next terminal)
  ```


-------------------------------
## **DRONE Launch Files : To Change environment , go to launch file and edit**
- **3D mapping**:
  - Run mapping Using RTAB Mapping
  ```bash
    ros2 launch slam 3d_mapping_drone.launch.py
    ros2 run teleop_twist_keyboard teleop_twist_keyboard --ros-args -r cmd_vel:=/simple_drone/cmd_vel #(next terminal)
  ```
