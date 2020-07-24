#include <iostream>
#include <ros/ros.h>
#include "opencv2/opencv.hpp"
#include <vector>
#include "XYToPixel.h"
#include "slam/Data.h"
#include <nav_msgs/Path.h>


class Global_path_gen{
	private:
		ros::NodeHandle nh;
		ros::Publisher pub;
		ros::Subscriber sub;
	
	public:
	
		int prev_pixel_x{}; 
		int prev_pixel_y{};
		nav_msgs::Path global_path;
		cv::Mat path_map = cv::Mat(15000, 15000, CV_8UC3, cv::Scalar(255,255,255));

		Global_path_gen(){
			sub = nh.subscribe("/filtered_data", 1000, &Global_path_gen::callback, this);
		}
		
		void callback(const slam::Data data){
			geometry_msgs::PoseStamped inst_pose;
			int inst_pixel_x, inst_pixel_y;
			bool check{0};	
			//change the last parameter in XYToPixel to choose the specific map
			XYToPixel(path_map, data.x, data.y, inst_pixel_x, inst_pixel_y, 2);
			if(global_path.poses.empty()){
				inst_pose.pose.position.x = inst_pixel_x;
				inst_pose.pose.position.y = inst_pixel_y;
				inst_pose.pose.position.z = data.theta;
				prev_pixel_x = inst_pixel_x;
				prev_pixel_y = inst_pixel_y;

				check = 1;
			}
			else if(((inst_pixel_x-prev_pixel_x)*(inst_pixel_x-prev_pixel_x)+(inst_pixel_y-prev_pixel_y)*(inst_pixel_y-prev_pixel_y))>98){
				inst_pose.pose.position.x = inst_pixel_x;
				inst_pose.pose.position.y = inst_pixel_y;
				inst_pose.pose.position.z = data.theta;
				prev_pixel_x = inst_pixel_x;
				prev_pixel_y = inst_pixel_y;
				check = 1;
			}
			if(check){
				global_path.poses.push_back(inst_pose);
				path_map.at<cv::Vec3b>(inst_pixel_x, inst_pixel_y)[0] = int(data.theta)/2;
				cv::imwrite("/home/parallels/data/global_path/glob_path.png", path_map);
			}

		}
};

int main(int argc, char**argv){
	ros::init(argc, argv, "globpath_generator");
	Global_path_gen global_path_gen;
	ros::spin();
}