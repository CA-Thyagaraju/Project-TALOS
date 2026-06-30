#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include <numeric>
#include <algorithm>
#include <deque>
#include <sstream>
#include <iomanip>
#include <cmath> // For atan2

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/point_stamped.hpp"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp" // Added for MoveIt tracking targets
#include "visualization_msgs/msg/marker.hpp"
#include "sensor_msgs/msg/image.hpp"

#include <cv_bridge/cv_bridge.hpp>

#include "tf2_ros/buffer.h"
#include "tf2_ros/transform_listener.h"
#include "tf2_ros/static_transform_broadcaster.h"
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <tf2/LinearMath/Quaternion.h> // Added for quaternion conversion

#include <opencv2/opencv.hpp>

using namespace std::chrono_literals;

class MarkerFollower : public rclcpp::Node {
public:
    MarkerFollower() : Node("marker_follower"), tracking_confidence_(0) {
        // TF & CV Bridge initialization
        tf_buffer_ = std::make_unique<tf2_ros::Buffer>(this->get_clock());
        tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);
        tf_static_broadcaster_ = std::make_shared<tf2_ros::StaticTransformBroadcaster>(this);

        // Publishers
        viz_pub_ = this->create_publisher<visualization_msgs::msg::Marker>("/visualization_marker", 10);
        image_pub_ = this->create_publisher<sensor_msgs::msg::Image>("/camera/image_raw", 10);
        
        // CRITICAL: Publisher targeted by our tracking control scripts
        goal_pub_ = this->create_publisher<geometry_msgs::msg::PoseStamped>("/target_arm_pose", 10);

        // Open Video Capture (Default camera)
        cap_.open(0);
        if (!cap_.isOpened()) {
            RCLCPP_ERROR(this->get_logger(), "Could not open video stream!");
        }

        // --- WORKSPACE CONFIGURATION ---
        // Aligned with launch settings: Zero tilt, 90-degree yaw parallel setup
        CAM_X_ = 0.5; CAM_Y_ = -0.5; CAM_Z_ = 0.5;
        POUCH_WIDTH_M_ = 0.20;
        WORKSPACE_W_ = 0.6; WORKSPACE_H_ = 0.4;

        broadcast_camera_setup();

        // Main Loop Timer (approx 30 FPS / 33ms)
        timer_ = this->create_wall_timer(33ms, std::bind(&MarkerFollower::main_loop, this));
        RCLCPP_INFO(this->get_logger(), "Goal Publisher Online. Tracking output broadcasted to /target_arm_pose.");
    }

    ~MarkerFollower() {
        cap_.release();
        cv::destroyAllWindows();
    }

private:
    void main_loop() {
        publish_camera_body_marker();
        
        cv::Mat frame;
        if (!cap_.read(frame)) return;

        cv::Mat blurred, hsv;
        cv::GaussianBlur(frame, blurred, cv::Size(11, 11), 0);
        cv::cvtColor(blurred, hsv, cv::COLOR_BGR2HSV);

        cv::Mat mask1, mask2, mask;
        cv::inRange(hsv, cv::Scalar(0, 160, 100), cv::Scalar(10, 255, 255), mask1);
        cv::inRange(hsv, cv::Scalar(170, 160, 100), cv::Scalar(180, 255, 255), mask2);
        mask = mask1 + mask2;

        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7));
        cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);
        cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        bool found_this_frame = false;

        for (const auto& cnt : contours) {
            double area = cv::contourArea(cnt);
            if (area > 3000) {
                cv::RotatedRect rect = cv::minAreaRect(cnt);
                cv::Point2f center = rect.center;
                float w = rect.size.width;
                float h = rect.size.height;

                float aspect_ratio = std::max(w, h) / (std::min(w, h) + 0.001f);

                if (aspect_ratio > 1.8f && aspect_ratio < 4.0f) {
                    found_this_frame = true;
                    tracking_confidence_++;

                    float pixel_width = std::max(w, h);
                    float dist_to_cam = (POUCH_WIDTH_M_ * 600.0f) / pixel_width;

                    std::string display_text;
                    if (tracking_confidence_ > 2) {
                        calculate_scaled_position(center.x, center.y, dist_to_cam);
                        
                        std::stringstream ss;
                        ss << "POUCH: " << std::fixed << std::setprecision(2) << dist_to_cam << "m";
                        display_text = ss.str();
                    } else {
                        display_text = "CALIBRATING...";
                    }

                    cv::Point2f vertices[4];
                    rect.points(vertices);
                    std::vector<cv::Point> box_points;
                    for (int i = 0; i < 4; i++) {
                        box_points.push_back(vertices[i]);
                    }
                    std::vector<std::vector<cv::Point>> contours_poly{box_points};
                    cv::drawContours(frame, contours_poly, 0, cv::Scalar(0, 0, 255), 3);
                    
                    cv::putText(frame, display_text, cv::Point(center.x - 60, center.y - 30),
                                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);
                    break; 
                }
            }
        }

        if (!found_this_frame) {
            tracking_confidence_ = 0;
        }

        auto img_msg = cv_bridge::CvImage(std_msgs::msg::Header(), "bgr8", frame).toImageMsg();
        image_pub_->publish(*img_msg);

        cv::imshow("Marker_Follower", frame);
        cv::waitKey(1);
    }

    void broadcast_camera_setup() {
        geometry_msgs::msg::TransformStamped t;
        t.header.stamp = this->get_clock()->now();
        t.header.frame_id = "base_link";
        t.child_frame_id = "camera_link";
        
        t.transform.translation.x = CAM_X_;
        t.transform.translation.y = CAM_Y_;
        t.transform.translation.z = CAM_Z_;

        // Quaternions matching 90-degree parallel setup (0.7071z, 0.7071w)
        t.transform.rotation.x = 0.0;
        t.transform.rotation.y = 0.0;
        t.transform.rotation.z = 0.7071;
        t.transform.rotation.w = 0.7071;

        tf_static_broadcaster_->sendTransform(t);
    }

    void calculate_scaled_position(float x, float y, float depth) {
        geometry_msgs::msg::PointStamped pt;
        pt.header.frame_id = "camera_link";
        pt.header.stamp = this->get_clock()->now();

        float depth_factor = depth / 0.5f;

        pt.point.x = ((x - 320.0f) / 640.0f) * WORKSPACE_W_ * depth_factor;
        pt.point.z = -((y - 240.0f) / 480.0f) * WORKSPACE_H_ * depth_factor;
        pt.point.y = depth;

        try {
            geometry_msgs::msg::PointStamped p_base;
            auto trans = tf_buffer_->lookupTransform("base_link", "camera_link", tf2::TimePointZero);
            tf2::doTransform(pt, p_base, trans);

            push_to_history(history_x_, p_base.point.x);
            push_to_history(history_y_, p_base.point.y);
            push_to_history(history_z_, p_base.point.z);

            double avg_x = get_average(history_x_);
            double avg_y = get_average(history_y_);
            double avg_z = get_average(history_z_);

            publish_marker_ball(avg_x, avg_y, avg_z);
            
            // --- NEW: GENERATE AND PUBLISH THE TARGET POSE ---
            publish_calculated_goal(avg_x, avg_y, avg_z);

        } catch (const tf2::TransformException &ex) {
            // Drop execution silently on transformation faults
        }
    }

    void publish_calculated_goal(double target_x, double target_y, double target_z) {
        geometry_msgs::msg::PoseStamped goal_msg;
        goal_msg.header.stamp = this->get_clock()->now();
        goal_msg.header.frame_id = "base_link";

        // Assign filtered 3D point tracking data 
        goal_msg.pose.position.x = target_x;
        goal_msg.pose.position.y = target_y;
        goal_msg.pose.position.z = target_z;

        // Calculate horizontal alignment angles facing the pouch target
        double target_yaw = std::atan2(target_y, target_x);

        // Convert Euler angles to modern ROS 2 Quaternion structures
        tf2::Quaternion q;
        q.setRPY(0.0, 0.0, target_yaw);

        goal_msg.pose.orientation.x = q.x();
        goal_msg.pose.orientation.y = q.y();
        goal_msg.pose.orientation.z = q.z();
        goal_msg.pose.orientation.w = q.w();

        goal_pub_->publish(goal_msg);
    }

    void publish_camera_body_marker() {
        visualization_msgs::msg::Marker m;
        m.header.frame_id = "base_link";
        m.header.stamp = this->get_clock()->now();
        m.ns = "camera"; m.id = 0;
        m.type = visualization_msgs::msg::Marker::CUBE;
        
        m.pose.position.x = CAM_X_; m.pose.position.y = CAM_Y_; m.pose.position.z = CAM_Z_;
        m.scale.x = 0.08; m.scale.y = 0.04; m.scale.z = 0.04;
        m.color.a = 0.8; m.color.r = 0.4; m.color.g = 0.4; m.color.b = 0.4;
        viz_pub_->publish(m);
    }

    void publish_marker_ball(double x, double y, double z) {
        visualization_msgs::msg::Marker m;
        m.header.frame_id = "base_link";
        m.header.stamp = this->get_clock()->now();
        m.ns = "target"; m.id = 1;
        m.type = visualization_msgs::msg::Marker::SPHERE;
        
        m.pose.position.x = x; m.pose.position.y = y; m.pose.position.z = z;
        m.scale.x = m.scale.y = m.scale.z = 0.07;
        m.color.a = 1.0; m.color.r = 1.0;
        viz_pub_->publish(m);
    }

    void push_to_history(std::deque<double>& queue, double val) {
        queue.push_back(val);
        if (queue.size() > 10) {
            queue.pop_front();
        }
    }

    double get_average(const std::deque<double>& queue) {
        if (queue.empty()) return 0.0;
        return std::accumulate(queue.begin(), queue.end(), 0.0) / queue.size();
    }

    std::unique_ptr<tf2_ros::Buffer> tf_buffer_;
    std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
    std::shared_ptr<tf2_ros::StaticTransformBroadcaster> tf_static_broadcaster_;
    
    rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr viz_pub_;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr image_pub_;
    rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr goal_pub_; // Added pointer
    rclcpp::TimerBase::SharedPtr timer_;

    std::deque<double> history_x_, history_y_, history_z_;
    int tracking_confidence_;
    cv::VideoCapture cap_;

    double CAM_X_, CAM_Y_, CAM_Z_;
    float POUCH_WIDTH_M_;
    float WORKSPACE_W_, WORKSPACE_H_;
};

int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<MarkerFollower>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}