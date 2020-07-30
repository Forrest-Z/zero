#include <ros/ros.h>
#include <iostream>
#include <cmath>
#include <ctime>
#include "core_msgs/Control.h"
#include "std_msgs/Float32.h"
#include "std_msgs/Float64.h"
#include "slam/Data.h"

class PID{
    public:
        ros::NodeHandle nh;

        double recommend_vel{0};
        double current_vel{0};

        double P_gain{0.4};
        double I_gain{20.0};
        double D_gain{0.0};
        double error{0};
        double integral_error{0};
        double differential_error{0};
        double Prev_error{0};
        double pid_input{0};
        double start_time{0};
        clock_t time = clock();

        ros::Publisher car_signal_pub;
        ros::Subscriber current_vel_sub;
        ros::Subscriber recommend_vel_sub;
        ros::Subscriber time_sub_vel_sub;

        PID(){
            car_signal_pub = nh.advertise<core_msgs::Control>("/car_signal", 10);
            current_vel_sub = nh.subscribe("filtered_data",100, &PID::current_vel_callback, this);
            recommend_vel_sub = nh.subscribe("recommend_vel",100, &PID::recommend_vel_callback, this);
        }

        void current_vel_callback(const slam::Data::ConstPtr);
        void recommend_vel_callback(const std_msgs::Float32::ConstPtr);
        void time_callback(const std_msgs::Float64::ConstPtr);
        void vehicle_output_signal();
        void calculate_PID();

};

void PID::current_vel_callback(const slam::Data::ConstPtr msg){
    current_vel = msg->v;
}
void PID::recommend_vel_callback(const std_msgs::Float32::ConstPtr msg){
    recommend_vel = msg->data;
}

void PID::time_callback(const std_msgs::Float64::ConstPtr msg){
    start_time = msg->data;
}

void PID::calculate_PID(){
    static bool first = true;
    if(first == true){
        time = clock();
        first = false;
        return;
    }

    error = recommend_vel - current_vel;
    integral_error = integral_error + error * (double(clock() - time)/(double)CLOCKS_PER_SEC);
    differential_error = (error - Prev_error)/(double(clock() - time)/(double)CLOCKS_PER_SEC);
    std::cout << 1 << std::endl;
    pid_input = P_gain * error + I_gain * integral_error + D_gain * differential_error;
    Prev_error = error;
    std::cout << "dt : " << (clock()-time)/(double)CLOCKS_PER_SEC << std::endl;
    time = clock();
    std::cout << "error : " << error << std::endl;
    std::cout << "integral_error : " << integral_error << std::endl;
    std::cout << "differential_error : " << differential_error << std::endl;
}

void PID::vehicle_output_signal(){
    core_msgs::Control msg;
    if (start_time > 0){
        msg.is_auto = 1;
        msg.estop = 0;
        msg.gear = 0;
        msg.brake = 0;
        msg.speed = (pid_input>0)?pid_input:0;
        msg.steer = 0; 
    }

    else{
        msg.is_auto = 1;
        msg.estop = 0;
        msg.gear = 0;
        msg.brake = 0;
        msg.speed = (pid_input>0)?pid_input:0;
        msg.steer = 0;   
    }
    car_signal_pub.publish(msg);
}

int main(int argc, char *argv[])
{
    std::cout << "PID start" << std::endl;  
	int count{0};

	ros::init(argc,argv,"PID_tracker");
	ros::NodeHandle nh;

	PID pid;
	ros::Rate loop_rate(10);

	while (ros::ok())
    {

        pid.calculate_PID();
        pid.vehicle_output_signal();
	    ros::spinOnce();
		loop_rate.sleep();
	
    }

	return 0;
}
