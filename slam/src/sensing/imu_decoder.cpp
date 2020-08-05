#include "ros/ros.h"
#include "ros/time.h"
#include "slam/Imu.h"
#include "sensor_msgs/Imu.h"
#include "sensor_msgs/MagneticField.h"
#include "geometry_msgs/Vector3Stamped.h"
#include "geometry_msgs/Quaternion.h"
#include "UnixtimeToSec.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <math.h>

class IMU_Decoder{
    public:
    IMU_Decoder(){
        pub_ = data_.advertise<slam::Imu>("imu", 2);
        sub_data_ = data_.subscribe("/imu/data", 2, &IMU_Decoder::callback_data, this);
        //sub_mag_ = mag_.subscribe("/imu/mag", 2, &IMU_Decoder::callback_mag, this);
    }
    
    void callback_data(const sensor_msgs::Imu::ConstPtr& msg){
        slam::Imu rt;
        
        rt.header = msg->header;
        //rt.header.stamp.sec = UnixtimeToSec(msg->header.stamp.sec);
	    //ros::Time tm = ros::Time::now();
        //rt.header.stamp.sec = tm.sec;
        //rt.header.stamp.nsec = tm.nsec;
        //rt.local_ax = msg->linear_acceleration.x;
        //rt.local_ay = msg->linear_acceleration.y;
        //rt.omega = msg->angular_velocity.z;
        //rt.theta = theta_;
        geometry_msgs::Quaternion q = msg->orientation;
        rt.theta = std::atan2( 2*(q.x*q.y+q.z*q.w), 1-2*(q.y*q.y+q.z*q.z) ) - theta_diff_;
        rt.theta = remainder(rt.theta,2*M_PI);
        double roll = std::atan2( 2*(q.x*q.w+q.y*q.z), 1-2*(q.x*q.x+q.y*q.y) );
        double pitch = asin(2*(q.y*q.w-q.x*q.z));
        rt.local_ax = std::cos(pitch)*msg->linear_acceleration.x + std::sin(roll)*std::sin(pitch)*msg->linear_acceleration.y + std::cos(roll)*std::sin(pitch)*msg->linear_acceleration.z;
        rt.local_ay = std::cos(roll)*msg->linear_acceleration.y - std::sin(roll)*msg->linear_acceleration.z;
        rt.omega = -std::sin(pitch)*msg->angular_velocity.x + std::sin(roll)*std::cos(pitch)*msg->angular_velocity.y + std::cos(roll)*std::cos(pitch)*msg->angular_velocity.z;
        //rt.err_a = std::abs(msg->linear_acceleration.z - 9.79945);
        //rt.err_omega = sqrt( msg->angular_velocity.x*msg->angular_velocity.x + msg->angular_velocity.y*msg->angular_velocity.y );
        pub_.publish(rt);
    }

    void callback_mag(const geometry_msgs::Vector3Stamped::ConstPtr& msg){
    //void callback_mag(const sensor_msgs::MagneticField::ConstPtr& msg){
        // double theta_mag = std::atan2(msg->magnetic_field.y, msg->magnetic_field.x);
        double theta_mag = std::atan2(msg->vector.y,msg->vector.x);
        theta_ = M_PI/2 - (theta_mag + theta_diff_);
    }

    private:
    ros::NodeHandle data_;
    ros::NodeHandle mag_;
    ros::Publisher pub_;
    ros::Subscriber sub_data_;
    ros::Subscriber sub_mag_;
    double theta_;
    double theta_diff_ = -8.60/180*M_PI;  //difference between magnetic north and truth north
};

int main(int argc, char **argv){
    ros::init(argc, argv, "imu_decoder");
    IMU_Decoder IMUObject;
    ros::spin();
}
