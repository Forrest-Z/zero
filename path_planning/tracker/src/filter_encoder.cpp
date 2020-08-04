#include <ros/ros.h>
#include <iostream>
#include <cmath>
#include <ctime>
#include "core_msgs/VehicleState.h"
#include "core_msgs/Encoderfilter.h"

class Filter{

    public: 
        ros::NodeHandle nh;

        double* encoder_data;
        int * time_sec;
	int * time_nsec;
	int data_num{10};
        //int check = 0;
        double threshold = 0.1;
	bool first_callback{false};

        ros::Publisher filter_encoder_pub;
        ros::Subscriber encoder_sub;

        Filter(){
		encoder_data = new double[data_num];	
		time_sec = new int[data_num];
		time_nsec = new int[data_num];
            filter_encoder_pub = nh.advertise<core_msgs::Encoderfilter>("filter_encoder_data", 1000);
            encoder_sub = nh.subscribe("/vehicle_state",100, &Filter::encoder_sub_callback, this);
        }

        void encoder_sub_callback(const core_msgs::VehicleState::ConstPtr);
        void determind_filter_encoder();

};

void Filter::encoder_sub_callback(const core_msgs::VehicleState::ConstPtr msg){

	//std::cout << "speed : " << msg->speed << std::endl;
	if (first_callback == false)
	{
		for (int i{0}; i<data_num; i++)
		{
			encoder_data[i] = msg->speed;
			time_sec[i] = msg->header.stamp.sec;
			time_nsec[i] = msg->header.stamp.nsec;
		}
		first_callback =true;
	}
	else
	{
		for (int i{0}; i<data_num-1; i++)
		{
			encoder_data[i] = encoder_data[i+1];
			time_sec[i] = time_sec[i+1];
			time_nsec[i] = time_nsec[i+1];
		}
		if(encoder_data[data_num-1] - msg->speed > threshold)
		{
			std::cout << "peak!\n";
		}
		else
			encoder_data[data_num-1] = msg->speed;
		time_sec[data_num-1] = msg->header.stamp.sec;
	        time_nsec[data_num-1] = msg->header.stamp.nsec;
	}
	printf("time : %03d.%09d\t", msg->header.stamp.sec%1000, msg->header.stamp.nsec);
/*    encoder_data[0] = encoder_data[1];
    encoder_data[1] = encoder_data[2];
    encoder_data[2] = msg->speed;
    time[0] = time[1];
    time[1] = time[2];
    time[2] = float(msg->header.stamp.sec) + msg->header.stamp.nsec / 1000000000.0;
    std::cout << float(msg->header.stamp.sec) + msg->header.stamp.nsec / 1000000000.0 << std::endl;
*/
	determind_filter_encoder();
}

void Filter::determind_filter_encoder(){

    core_msgs::Encoderfilter msg;
    msg.filtered_encoder = encoder_data[data_num-1];
    long double time_diff = (long double)((long long)(time_sec[data_num-1]-time_sec[0])*1000000000 +(long long)(time_nsec[data_num-1] - time_nsec[0]))/(long double)1000000000.0;
    if (time_diff == 0)
	    msg.slope = 0;
    else
	    msg.slope = (encoder_data[data_num-1] - encoder_data[0]) / (double)time_diff;
    msg.time =(double)( (long double)((long long)time_sec[data_num-1]*1000000000 + time_nsec[data_num-1]) / 1000000000.0);

    filter_encoder_pub.publish(msg);
    std::cout << "filtered_encoder : " << msg.filtered_encoder << "\t";
    std::cout << "slop : " << msg.slope << std::endl;
}

int main(int argc, char *argv[])
{

	ros::init(argc,argv,"filter_encoder");
	ros::NodeHandle nh;
    
    Filter filter{Filter()};
	ros::Rate loop_rate(10);

	ros::spin();
/*
	while (ros::ok())
	{
        filter.determind_filter_encoder();
		ros::spinOnce();
		loop_rate.sleep();
	}
*/
	return 0;
}