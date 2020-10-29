#include <iostream>
#include <string>
#include <vector>

#include "std_msgs/Float64.h"
#include "std_msgs/Float32.h"

#include "slam/Pixel.h"
#include "slam/Data.h"

#include "ros/package.h"
#include "ros/ros.h"
#include "opencv2/opencv.hpp"

#include "XYToPixel.h"


class velocity_publisher{
    private:
        ros::NodeHandle nh;
        ros::Publisher pub;
        ros::Subscriber sub;
        int pixel_x, pixel_y;
        double recommended_velocity;
        bool is_kcity;

    public:
        cv::Mat velocity_map;
        std::stringstream path_stream;

        velocity_publisher(){
            ros::param::get("/is_kcity", is_kcity);

            if(is_kcity==true){
            	path_stream << ros::package::getPath("slam") << "/config/KCity/KCity_velocity_map_5.png";
                velocity_map = cv::imread(path_stream.str(), cv::IMREAD_COLOR);  

                if(!velocity_map.empty()){
                    ROS_INFO("KCity loaded");
                }   
            }
            else if(is_kcity==false){
            	path_stream << ros::package::getPath("slam") << "/config/FMTC/new/FMTC_velocity_map_555_4.png";
                velocity_map = cv::imread(path_stream.str(), cv::IMREAD_COLOR);  
                   if(!velocity_map.empty()){
                        ROS_INFO("FMTC loaded");
                    }   
            }
 
            //pub = nh.advertise<std_msgs::Float64>("/recommend_vel", 2);
            pub = nh.advertise<std_msgs::Float32>("/recommend_vel", 2);
            sub = nh.subscribe("/filtered_data",2, &velocity_publisher::callback, this);
        }

        void callback(const slam::Data::ConstPtr& msg){
            bool x_inRange, y_inRange;
 
            XYToPixel(pixel_y, pixel_x, msg->x, msg->y, is_kcity); // pixel_y here is x in cv graphics and column in cv Mat
            
            if(is_kcity==true){
            x_inRange ={pixel_x<=22489 && pixel_x > 0};
            y_inRange ={pixel_y<=8273 && pixel_y > 0};
            }

            if(is_kcity==false){
            x_inRange ={pixel_x<=14226 && pixel_x > 0};
            y_inRange ={pixel_y<=12072 && pixel_y > 0};
            }

            std::cout<<"Pixel information is loaded: "<<pixel_x<<", "<<pixel_y<<std::endl;

            if(x_inRange&&y_inRange){
                std::cout<<"on map"<<std::endl;
                recommended_velocity = velocity_map.at<cv::Vec3b>(pixel_x, pixel_y)[0];
                //std_msgs::Float64 rt;
                std_msgs::Float32 rt;
                rt.data = (85/recommended_velocity)*4;
                pub.publish(rt);
            }
        }
};

int main(int argc, char **argv){
    ros::init(argc, argv, "velocity_publisher");
    velocity_publisher velocity_publisher;
    ros::spin();
}

