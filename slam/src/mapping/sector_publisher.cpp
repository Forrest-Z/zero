#include <iostream>
#include <ros/ros.h>
#include "ros/package.h"
#include "opencv2/opencv.hpp"
#include <vector>
#include <slam/Pixel.h>
#include <slam/Data.h>
#include "XYToPixel.h"
#include "std_msgs/UInt32.h"


class sector_publisher{
    private:
        ros::NodeHandle nh;
        ros::Publisher pub;
        ros::Subscriber sub;
        //double pixel_x, pixel_y;
        int pixel_x, pixel_y;
        bool x_inRange{(pixel_x<=14500)}, y_inRange{(pixel_y<=14500)};
        int nBlue, nGreen, nRed;

    public:
        std::stringstream path_stream;
        cv::Mat color_map;
	    //cv::Mat color_map = cv::imread("/home/snuzero/catkin_ws/src/zero/slam/src/mapping/color_map.png");
        sector_publisher(){
            path_stream << ros::package::getPath("slam") << "/src/mapping/color_map.png";
		    color_map = cv::imread(path_stream.str());
            ROS_INFO("Image loaded");
            pub = nh.advertise<std_msgs::UInt32>("/sector_info", 2);
            sub = nh.subscribe("/filtered_data",2, &sector_publisher::callback, this);
            //sub = nh.subscribe("/position/pixel", 1000, &sector_publisher::callback, this);
          //  nBlue = 0; nGreen = 0; nRed = 0;
        }

        //void callback(const slam::Pixel Data){
        void callback(const slam::Data::ConstPtr& msg){
            XYToPixel(pixel_y, pixel_x, msg->x, msg->y); // pixel_y here is x in cv graphics and column in cv Mat
            //pixel_x = Data.y;
            //pixel_y = Data.x;
            std::cout<<"Pixel information is loaded: "<<pixel_x<<", "<<pixel_y<<std::endl;

            if(x_inRange&&y_inRange){
                std::cout<<"on map"<<std::endl;
                //get the RGB information of a pixel 
                nBlue = color_map.at<cv::Vec3b>(pixel_x, pixel_y)[0];
                nGreen = color_map.at<cv::Vec3b>(pixel_x,pixel_y)[1];
                nRed = color_map.at<cv::Vec3b>(pixel_x,pixel_y)[2];
                
            std::cout<<nBlue<<" "<<nGreen<<" "<<nRed<<std::endl;

                //publish
                // in sector A ==> pub 0
                //           B ==> pub 1
                //           C ==> pub 2
                //           D ==> pub 3

                std_msgs::UInt32 rt;
                if(nBlue==0&&nGreen==0&&nRed==0)
                {
                    rt.data = 0;
                    ROS_INFO("Sector A");
                }
                if(nBlue==0&&nGreen==255&&nRed==0)
                {
                    rt.data = 1;
                    ROS_INFO("Sector B");
                }
                if(nBlue==255&&nGreen==0&&nRed==0)
                {
                    rt.data = 2;
                    ROS_INFO("Sector C");
                }
                if(nBlue==0&&nGreen==0&&nRed==255)
                {
                    rt.data = 3;
                    ROS_INFO("Sector D");
                }
                pub.publish(rt);
            }
        }
};

int main(int argc, char **argv){
    ros::init(argc, argv, "sector_publisher");
    sector_publisher sector_publisher;
    ros::spin();
}
