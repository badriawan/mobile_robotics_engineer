#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>
#include <visualization_msgs/msg/marker.hpp>
#include <geometry_msgs/msg/point.hpp>
#include <algorithm>
#include <cmath>
#include <limits>

class ScanSplitter : public rclcpp::Node
{
public:
  ScanSplitter() : Node("scan_splitter")
  {
    using sensor_qos = rclcpp::SensorDataQoS;

    scan_sub_ = create_subscription<sensor_msgs::msg::LaserScan>(
        "scan", sensor_qos(),
        std::bind(&ScanSplitter::scanCallback, this, std::placeholders::_1));

    front_pub_  = create_publisher<sensor_msgs::msg::LaserScan>("scan/front", 10);
    left_pub_   = create_publisher<sensor_msgs::msg::LaserScan>("scan/left",  10);
    back_pub_   = create_publisher<sensor_msgs::msg::LaserScan>("scan/back",  10);
    right_pub_  = create_publisher<sensor_msgs::msg::LaserScan>("scan/right", 10);

    front_marker_pub_ = create_publisher<visualization_msgs::msg::Marker>("scan/front/marker", 10);
    left_marker_pub_  = create_publisher<visualization_msgs::msg::Marker>("scan/left/marker",  10);
    back_marker_pub_  = create_publisher<visualization_msgs::msg::Marker>("scan/back/marker",  10);
    right_marker_pub_ = create_publisher<visualization_msgs::msg::Marker>("scan/right/marker", 10);
  }

private:

  void logMinDistance(const std::string & label,
                      const sensor_msgs::msg::LaserScan & slice)
  {
    double min_val = std::numeric_limits<double>::infinity();
    for (double r : slice.ranges)
      if (std::isfinite(r))
        min_val = std::min(min_val, r);

    if (std::isfinite(min_val))
      RCLCPP_INFO(get_logger(), "Min distance in %s: %.3f m", label.c_str(), min_val);
    else
      RCLCPP_WARN(get_logger(), "No finite readings in %s segment\n", label.c_str());
  }
  // -------------------------------------------------------------------------

  void scanCallback(const sensor_msgs::msg::LaserScan::SharedPtr msg)
  {
    const auto total = msg->ranges.size();
    if (total < 4) return;

    const auto q = total / 4U;

    publishSlice(*msg, 0,          q,           "front", front_pub_,  front_marker_pub_,  1,0,0);
    publishSlice(*msg, q,          q,           "left",  left_pub_,   left_marker_pub_,   0,1,0);
    publishSlice(*msg, 2*q,        q,           "back",  back_pub_,   back_marker_pub_,   0,0,1);
    publishSlice(*msg, 3*q, total-3*q, "right", right_pub_, right_marker_pub_, 1,1,0);
  }

  void publishSlice(const sensor_msgs::msg::LaserScan & src,
                    std::size_t start, std::size_t len,
                    const std::string & label,
                    const rclcpp::Publisher<sensor_msgs::msg::LaserScan>::SharedPtr & scan_pub,
                    const rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr & marker_pub,
                    float r, float g, float b)
  {
    // ---- build LaserScan slice ----
    sensor_msgs::msg::LaserScan scan = src;
    scan.ranges.assign(src.ranges.begin()+start, src.ranges.begin()+start+len);
    scan.intensities.assign(scan.ranges.size(), 0.0f);
    scan.angle_min = src.angle_min + start*src.angle_increment;
    scan.angle_max = scan.angle_min + (len-1)*src.angle_increment;

    scan_pub->publish(scan);

    // ---- log the closest hit in this segment ----
    logMinDistance(label, scan);

    // ---- RViz marker ----
    visualization_msgs::msg::Marker mk;
    mk.header = src.header;
    mk.ns     = "scan_quadrant";
    mk.id     = static_cast<int>(start);
    mk.type   = visualization_msgs::msg::Marker::POINTS;
    mk.action = visualization_msgs::msg::Marker::ADD;
    mk.scale.x = mk.scale.y = 0.05;
    mk.color.a = 1.0; mk.color.r=r; mk.color.g=g; mk.color.b=b;

    for (std::size_t i=0;i<scan.ranges.size();++i)
    {
      double rng = scan.ranges[i];
      if (!std::isfinite(rng)) continue;
      double ang = scan.angle_min + i*src.angle_increment;
      geometry_msgs::msg::Point p;
      p.x = rng*std::cos(ang);
      p.y = rng*std::sin(ang);
      mk.points.push_back(p);
    }
    marker_pub->publish(mk);
  }

  // -------------- members ---------------
  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr scan_sub_;
  rclcpp::Publisher<sensor_msgs::msg::LaserScan>::SharedPtr
      front_pub_, left_pub_, back_pub_, right_pub_;
  rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr
      front_marker_pub_, left_marker_pub_, back_marker_pub_, right_marker_pub_;
};

int main(int argc,char**argv)
{
  rclcpp::init(argc,argv);
  rclcpp::spin(std::make_shared<ScanSplitter>());
  rclcpp::shutdown();
  return 0;
}
