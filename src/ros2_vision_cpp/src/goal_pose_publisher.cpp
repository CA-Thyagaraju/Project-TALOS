#include <chrono>
#include <memory>
#include <mutex>
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include <moveit/move_group_interface/move_group_interface.h>

using namespace std::chrono_literals;

class GoalPosePublisher : public rclcpp::Node {
public:
    GoalPosePublisher() : Node("goal_pose_publisher"), new_target_available_(false) {
        
        // Reentrant Callback Group for handling subscriptions alongside planning execution
        callback_group_ = this->create_callback_group(rclcpp::CallbackGroupType::Reentrant);
        
        auto sub_options = rclcpp::SubscriptionOptions();
        sub_options.callback_group = callback_group_;

        // 1. Subscribe to the vision node's output stream
        pose_sub_ = this->create_subscription<geometry_msgs::msg::PoseStamped>(
            "/target_arm_pose",
            10,
            std::bind(&GoalPosePublisher::pose_callback, this, std::placeholders::_1),
            sub_options
        );

        // 2. Control Loop Timer to process tracking commands at a stable interval
        control_timer_ = this->create_wall_timer(
            200ms, 
            std::bind(&GoalPosePublisher::update_arm_motion, this),
            callback_group_
        );

        RCLCPP_INFO(this->get_logger(), "Goal Pose Publisher Ready. Listening on /target_arm_pose...");
    }

    void init_move_group() {
        move_group_ = std::make_shared<moveit::planning_interface::MoveGroupInterface>(shared_from_this(), "arm");
        
        // Optimize constraints for responsive tracking
        move_group_->setPlanningTime(1.0);
        move_group_->setMaxVelocityScalingFactor(0.4);       
        move_group_->setMaxAccelerationScalingFactor(0.4);
        
        // Slack tolerances to allow responsive tracking in simulation
        move_group_->setGoalPositionTolerance(0.02); // 2 cm workspace margin
        move_group_->setGoalOrientationTolerance(0.1); // ~5 degrees orientation slack

        RCLCPP_INFO(this->get_logger(), "Connected to Planning Group: %s", move_group_->getName().c_str());
    }

private:
    void pose_callback(const geometry_msgs::msg::PoseStamped::SharedPtr msg) {
        std::lock_guard<std::mutex> lock(target_mutex_);
        latest_target_pose_ = *msg;
        new_target_available_ = true;
    }

    void update_arm_motion() {
        if (!move_group_) return;

        geometry_msgs::msg::PoseStamped target_to_plan;
        {
            std::lock_guard<std::mutex> lock(target_mutex_);
            if (!new_target_available_) {
                return; // Wait silently until a pouch position comes through
            }
            target_to_plan = latest_target_pose_;
            new_target_available_ = false; // Reset flag
        }

        RCLCPP_INFO(this->get_logger(), "New target tracking vector sampled: X=%.2f, Y=%.2f, Z=%.2f",
                    target_to_plan.pose.position.x,
                    target_to_plan.pose.position.y,
                    target_to_plan.pose.position.z);

        // Refresh MoveIt's knowledge of where the simulated joints are sitting right now
        move_group_->setStartStateToCurrentState();

        // Pass the 3D position and orientation directly to the planner
        move_group_->setPoseTarget(target_to_plan.pose);

        moveit::planning_interface::MoveGroupInterface::Plan tracking_plan;
        bool success = (move_group_->plan(tracking_plan) == moveit::core::MoveItErrorCode::SUCCESS);

        if (success) {
            RCLCPP_INFO(this->get_logger(), "Trajectory calculated. Moving arm links...");
            move_group_->execute(tracking_plan);
        } else {
            RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 2000,
                                 "Target position unreachable or out of reach limits.");
        }
    }

    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr pose_sub_;
    rclcpp::TimerBase::SharedPtr control_timer_;
    std::shared_ptr<moveit::planning_interface::MoveGroupInterface> move_group_;
    rclcpp::CallbackGroup::SharedPtr callback_group_;

    std::mutex target_mutex_;
    geometry_msgs::msg::PoseStamped latest_target_pose_;
    bool new_target_available_;
};

int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<GoalPosePublisher>();
    rclcpp::executors::MultiThreadedExecutor executor;
    executor.add_node(node);
    node->init_move_group();
    executor.spin();
    rclcpp::shutdown();
    return 0;
}