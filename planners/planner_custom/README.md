### **Custom A* Planner for ROS 2 Navigation (Nav2)**
A custom **A* (Astar) planner** for ROS 2 Navigation stack, implemented to demonstrate how to integrate custom planning algorithms into Nav2.

---

## **How to Run**

### **Build the Repository**
Clone the `dev_robotisim` repository (**dev_engineering branch**) and build it:
```bash
cd ~/robotisim_ws
colcon build --symlink-install --packages-select planner_custom slam
source install/setup.bash
```

### **Launch the Planner**
This command launches the planner, loads the appropriate **RViz configuration** and **map**:
```bash
ros2 launch planner_custom custom_planner.launch.py
```

### **Manually Set Lifecycle States**
After launching, manually configure and activate required components:
```bash
ros2 lifecycle set /map_server configure
ros2 lifecycle set /map_server activate

ros2 lifecycle set /costmap/costmap configure
ros2 lifecycle set /costmap/costmap activate

ros2 lifecycle set /planner_server configure
ros2 lifecycle set /planner_server activate
```

### **Send a Goal Command**
Once the planner is active, send a **goal pose**:
```bash
ros2 action send_goal /compute_path_to_pose nav2_msgs/action/ComputePathToPose "{start: {header: {frame_id: 'map'}, pose: {position: {x: -5.17, y: -4.190, z: 0.0}, orientation: {w: 1.0}}}, goal: {header: {frame_id: 'map'}, pose: {position: {x: -4.86, y: -0.930, z: 0.0}, orientation: {w: 1.0}}}, use_start: true}"
```

---

## **Nav2 Working Structure and Plugin Interaction**
Nav2 follows a modular design with different components interacting through topics, actions, and services.

```
+---------------------+    +----------------------+    +---------------------+    +---------------------+    +---------------------+    +------------------+
|  Custom Planner    | -> |  Planner Plugin (C++) | -> |  Plugin Registration | -> |  Planner Server    | -> |  Costmap Server     | -> |  Map Server      |
|  (A* Implementation) |    |  (nav2_planner_interface) |    |  (pluginlib - XML) |    |  (planner_server) |    |  (costmap_server)  |    |  (map_server)   |
+---------------------+    +----------------------+    +---------------------+    +---------------------+    +---------------------+    +------------------+
```

### **Process Breakdown**
1. **Custom Planner** is implemented in C++ (`nav2_planner_interface.cpp`).
2. **Planner Plugin** is created by implementing `nav2_core::GlobalPlanner`.
3. **Plugin Registration** is done in `global_planner_plugin.xml` using `pluginlib`.
4. **Planner Server (`planner_server`)** loads the plugin as defined in `planner_params.yaml`.
5. **Costmap Server (`costmap_server`)** provides the required `global_costmap`.
6. **Map Server (`map_server`)** loads the static map, required for global planning.

### **How Custom Plugins are Integrated**
1. The `planner_server` loads plugins via `pluginlib`.
2. The `nav2_params.yaml` file specifies which planner to use.
3. The plugin is registered in `global_planner_plugin.xml`.
4. The `planner_server` calls `createPlan()` from the registered planner.

---

## **Important Files**

### **Configurations**
- **Planner Plugin Registration (`config/global_planner_plugin.xml`)**
  Registers the A* planner with the Nav2 planner server.
  ```xml
  <library path="nav2_astar_planner">
    <class type="nav2_astar_planner::AstarPlanner" base_class_type="nav2_core::GlobalPlanner">
      <description>A* Path Planner</description>
    </class>
  </library>
  ```
- **Planner Parameters (`config/planner_params.yaml`)**
  Configures the planner for `planner_server`.
  ```yaml
  planner_server:
    ros__parameters:
      use_sim_time: false
      expected_planner_frequency: 20.0
      planner_plugins: ["AstarPlanner"]
      AstarPlanner:
        plugin: "nav2_astar_planner::AstarPlanner"
  ```

### **Core Planner Code**
- **Planner Plugin Interface (`src/nav2_planner_interface.cpp`)**
  - Implements **Nav2’s Global Planner interface**.
  - Converts the **Nav2 costmap** into an **Occupancy Grid**.
  - Calls the **A* search algorithm**.
  - Returns the **computed path**.

  ```cpp
  nav_msgs::msg::Path AstarPlanner::createPlan(geometry_msgs::msg::PoseStamped const& start,
                                               geometry_msgs::msg::PoseStamped const& goal) {
      nav_msgs::msg::OccupancyGrid grid = costmapToOccupancyGrid();
      auto astar_path = astar_search(grid, mx_start, my_start, mx_goal, my_goal);
      if (!astar_path.empty()) {
          nav_msgs::msg::Path path;
          path.header.frame_id = global_frame_;
          path.poses = astar_path;
          return path;
      }
      return nav_msgs::msg::Path();
  }
  ```

- **A* Algorithm (`src/algo_astar.cpp`)**
  - Implements the **A* algorithm** for path planning.
  - Searches for the **optimal path** in the occupancy grid.

- **Planner Node (`src/path_planning.cpp`)**
  - Subscribes to **/occupancy_grid**.
  - Publishes planned path to **/path**.
  - Can switch between **A* and other planners (RRT, Grid Sweep)**.

  ```cpp
  class PathPlanning : public rclcpp::Node {
  public:
    PathPlanning() : Node("path_planning_node"), use_astar_(true) { }

    void occupancyGridCallback(const nav_msgs::msg::OccupancyGrid &grid) {
        nav_msgs::msg::Path path_msg;
        if (use_astar_) {
            path_msg.poses = astar_search(grid);
        }
        path_publisher_->publish(path_msg);
    }
  };
  ```

### **Build System (CMake)**
- **CMakeLists.txt**
  - Registers **A* planner** as a **plugin for Nav2**.
  - Uses **Eigen** for mathematical computations.
  - Builds necessary libraries and nodes.

  ```cmake
  add_library(nav2_astar_planner SHARED src/nav2_planner_interface.cpp src/algo_astar_plugin.cpp)
  ament_target_dependencies(nav2_astar_planner geometry_msgs nav2_core nav2_costmap_2d nav2_util nav_msgs rclcpp)
  pluginlib_export_plugin_description_file(nav2_core config/global_planner_plugin.xml)
  ```
