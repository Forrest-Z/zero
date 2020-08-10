#include "ros/ros.h"
#include "ros/time.h"
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <math.h>
#include <set>
#include "slam/Lidar.h"
#include "slam/Cluster.h"
#include "geometry_msgs/Point.h"
#include "sensor_msgs/Image.h"
#include <sensor_msgs/image_encodings.h>
#include "Eigen/Eigen"
#include "opencv2/opencv.hpp"
#include <cv_bridge/cv_bridge.h>

using namespace std;
using namespace Eigen;
using namespace cv;

class ObstacleImage{
    public:
    ObstacleImage(){
        pub_ = nh_.advertise<sensor_msgs::Image>("/obstacle_map/image_raw", 10);
        sub_ = nh_.subscribe("/2d_obstacle_clouds", 1, &ObstacleImage::callback, this);
    }

    void callback(const slam::Cluster::ConstPtr& msg){
        clock_t begin = clock();
        Mat obstacle_map = Mat(height_, width_, CV_8UC3,Scalar(255,255,255));
        vector<geometry_msgs::Point> points = msg->points;
        for(geometry_msgs::Point point : points){
            int x = (int)(point.x/resolution_);
            int y = (int)(point.y/resolution_) + width_/2;
            if (x<0 || x>=height_ || y<0 || y>=width_) continue;
            Vec3b& color = obstacle_map.at<Vec3b>(x,y);
            color[0] = 0; color[1] = 0; color[2] = 0;
        }
        sensor_msgs::Image rt;
        std_msgs::Header header;
        header.seq = msg->header.seq; // user defined counter
        header.stamp = ros::Time::now(); // time
        img_bridge = cv_bridge::CvImage(header, sensor_msgs::image_encodings::RGB8, obstacle_map);
        img_bridge.toImageMsg(rt); // from cv_bridge to sensor_msgs::Image
        pub_.publish(rt); // ros::Publisher pub_img = node.advertise<sensor_msgs::Image>("topic", queuesize);
        clock_t end = clock();
        ROS_INFO("elapsed time : %lf", double(end-begin)/CLOCKS_PER_SEC);
    }

    private:
    ros::NodeHandle nh_;
    ros::Publisher pub_;
    ros::Subscriber sub_;
    int height_ = 300;
    int width_ = 300;
    double resolution_ = 0.03;
    cv_bridge::CvImage img_bridge;
};

int main(int argc, char **argv){
    ros::init(argc, argv, "obstacle_image");
    ObstacleImage obstacle_image;
    ros::spin();
}