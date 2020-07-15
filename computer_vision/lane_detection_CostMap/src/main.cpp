#include "func_util.h"
#include "lane_fit_sliding_window.h"
#include "lane_class.h"
#include <ctime>
#include <ros/ros.h>
//#include <cv_bridge/cv_bridge.h>
//#include <image_transport/image_transport.h> // check
//#include <opencv2/imgproc/imgproc.hpp> // check
//#include <opencv2/highgui/highgui.hpp> // check
#include <opencv2/opencv.hpp>
//#include <core_msgs/CostMapwithGoalVector.h>
//#include <geometry_msgs/~~.h>
//#include <nav_msgs/~~.h>
#include <nav_msgs/OccupancyGrid.h>
#include <nav_msgs/MapMetaData.h>
#include <std_msgs/Int8.h>

int main(int argc, char **argv)
{
	ros::init(argc,argv, "cv_main");
	ros::NodeHandle nh;
	ros::Publisher costmap_with_goal_vector_pub = nh.advertise<nav_msgs::OccupancyGrid>("cost_map_with_goal_vector",100);
    ros::Rate loop_rate(10);
    
	nav_msgs::OccupancyGrid cost_map;
	cost_map = nav_msgs::OccupancyGrid();

	int time = clock();
	cv::Mat img;
	img = cv::imread("/home/giholee97/catkin_ws/src/zero/computer_vision/lane_detection_CostMap/test_images/test2.jpg");

	if(img.empty())
	{
		std::cerr << "Image load failed!" << std::endl;
		return -1;
	}

	cv::Mat perspective_img;
	perspective_img = birdeye(img);

//	cv::namedWindow("perspective_img");
//	cv::imshow("perspective_img",perspective_img);

	cv::Mat yellow;
	yellow = thresh_frame_in_HSV(perspective_img);
//	cv::namedWindow("yellow_img");
//	cv::imshow("yellow_img",yellow);

	cv::Mat grad;
	grad = thresh_frame_sobel(perspective_img);
//	cv::namedWindow("grad_img");
//	cv::imshow("grad_img", grad);	

	cv::Mat gray;
	gray = get_binary_from_equalized_grayscale(perspective_img);
//	cv::namedWindow("gray_img");
//	cv::imshow("gray_img", gray);	

	cv::Mat lane_mask;
	lane_mask = get_lane_mask(yellow, grad, gray);
//	cv::namedWindow("lane_mask");
//	cv::imshow("lane_mask", lane_mask);

	cv::Mat fitting_mask;
	fitting_mask = get_fits_by_sliding_window(lane_mask, 10);
	//cv::namedWindow("fitting_mask");
	//cv::imshow("fitting_mask", fitting_mask);

	cv::Point2i goal_point;
    goal_point = get_goal_point(fitting_mask, lane_mask.cols);
    std::cout<<goal_point<<std::endl;

	cost_map.info.width = fitting_mask.size().width;
	cost_map.info.height = fitting_mask.size().height;
	//std_msgs::Int8 cost;
	for (int i = 0; i < cost_map.info.width; i++){
		for( int j = 0; j < cost_map.info.height; j++){
			//cost.data = fitting_mask.at<uchar>(j,cost_map.info.width-i-1);
			int cost = (fitting_mask.at<uchar>(i,cost_map.info.width-j-1))/4;
			cost_map.data.push_back(cost);
		}
	}

	//std_msgs::Int8 x;
	//std_msgs::Int8 y;
	//std_msgs::Int8 theta;

	//x.data = goal_point.y;
	//y.data = 199 - goal_point.x;
	//theta.data = 0;

	int x = (goal_point.x)/2;
	int y = (199 - goal_point.y)/2;
	int theta = 0;

	cost_map.data.push_back(x);
	cost_map.data.push_back(y);
    cost_map.data.push_back(theta);
	//ros::Rate loop_rate(5);

	while (nh.ok()) {
        costmap_with_goal_vector_pub.publish(cost_map);
        ros::spinOnce();
        loop_rate.sleep();
    }

	std::cout << (clock() - time)/(double)CLOCKS_PER_SEC << std::endl;

	cv::waitKey();
	return 0;
}
