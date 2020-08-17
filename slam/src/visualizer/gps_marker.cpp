#include "ros/ros.h"
#include "ros/console.h"
#include "slam/Gps.h"
#include "slam/Data.h"
#include "slam/Imu.h"
#include "opencv2/opencv.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include "XYToPixel.h"
#include "ros/package.h"

using namespace std;

#define REF_X 298500
#define REF_Y 4137850
#define VEL 1.0
#define INTER 0.8

class Map_Marker{
    public:
    Map_Marker(){
        ros::param::get("/is_kcity", is_kcity);

        sub_gps = n_.subscribe("/gps", 2000, &Map_Marker::callback_gps, this);
        sub_filtered = n_.subscribe("/filtered_data", 2000, &Map_Marker::callback_filtered, this);
        sub_imu = n_.subscribe("/imu", 2000, &Map_Marker::callback_imu, this);
        path_stream << ros::package::getPath("slam") << "/config/KCity/KCity.png";
        img = cv::imread(path_stream.str(), 1);

        if (img.empty()) {ROS_ERROR("The map image is empty! check the path of the map!");}
        else {ROS_INFO("The map image loaded");}
    }

    void callback_gps(const slam::Gps::ConstPtr& msg){
	    int pixel_x, pixel_y;

        XYToPixel(pixel_x, pixel_y, msg->x, msg->y, is_kcity);
        if (n != 1) {
            cv::line(img, cv::Point(prev_pixel_x, prev_pixel_y), cv::Point(pixel_x, pixel_y), cv::Scalar(255, 0, 0), 7);
            //cv::circle(img, cv::Point(pixel_x, pixel_y), 2, cv::Scalar(255, 0, 0), -1);
            if( n % int(2*INTER*10.0) == 0){
                //cv::arrowedLine(img, cv::Point(filtered_pixel_x, filtered_pixel_y), cv::Point(filtered_pixel_x+INTER*filtered_pixel_vx, filtered_pixel_y+INTER*filtered_pixel_vy), cv::Scalar(255, 0, 255), 4, 8, 0, 0.4);
                //cv::arrowedLine(img, cv::Point(filtered_pixel_x, filtered_pixel_y), cv::Point(filtered_pixel_x+INTER*filtered_pixel_thx, filtered_pixel_y+INTER*filtered_pixel_thy), cv::Scalar(255, 255, 0), 4, 8, 0, 0.4);
                //cv::arrowedLine(img, cv::Point(pixel_x, pixel_y), cv::Point(pixel_x+INTER*mag_pixel_thx, pixel_y+INTER*mag_pixel_thy), cv::Scalar(255, 0, 255), 4, 8, 0, 0.4);
            }
        }
        t = ros::Time::now();
        saved = false;
        prev_pixel_x = pixel_x;
        prev_pixel_y = pixel_y;

        ROS_DEBUG_STREAM("gps_marker: GPS: position(" << msg->x << ", " << msg->y << ") -> position(" << pixel_x << ", " << pixel_y << ")");
        n++;
    }

    void callback_filtered(const slam::Data::ConstPtr& msg){
	    int pixel_x, pixel_y;

        XYToPixel(filtered_pixel_x, filtered_pixel_y, msg->x, msg->y, is_kcity);
        XYToPixel(pixel_x, pixel_y, REF_X, REF_Y, is_kcity);
        XYToPixel(filtered_pixel_vx, filtered_pixel_vy, REF_X+msg->vx, REF_Y+msg->vy, is_kcity);
        filtered_pixel_vx -= pixel_x;
        filtered_pixel_vy -= pixel_y;
        XYToPixel(filtered_pixel_thx, filtered_pixel_thy, REF_X+VEL*cos(msg->theta), REF_Y+VEL*sin(msg->theta), is_kcity);
        filtered_pixel_thx -= pixel_x;
        filtered_pixel_thy -= pixel_y;

        ROS_DEBUG_STREAM("gps_marker: Kalman-filter: (" << msg->x << ", " << msg->y << ") -> position(" << pixel_x << ", " << pixel_y << \
        "), velocity(" << filtered_pixel_vx << ", " << filtered_pixel_vy << "), theta(" << filtered_pixel_thx << ", " << filtered_pixel_thy << ")");
    }

    void callback_imu(const slam::Imu::ConstPtr& msg){
	    int pixel_x, pixel_y;

        XYToPixel(pixel_x, pixel_y, REF_X, REF_Y, is_kcity);
        XYToPixel(mag_pixel_thx, mag_pixel_thy, REF_X+VEL*cos(msg->theta), REF_Y+VEL*sin(msg->theta), is_kcity);
        mag_pixel_thx -= pixel_x;
        mag_pixel_thy -= pixel_y;

        ROS_DEBUG_STREAM("gps_marker: IMU: theta(" << msg->theta << ") -> position(" << pixel_x << ", " << pixel_y << ")");
    }

    void save_if_end(){
        if( n!=1 && saved==false && (ros::Time::now()-t).sec > 4 ){
            path_stream.str(std::string());
            path_stream << ros::package::getPath("slam") << "/config/path" << n << ".png";
            cv::imwrite(path_stream.str(), img);
            ROS_INFO("Image saved %d",n);
            saved = true;
        }
    }

    private:
    ros::NodeHandle n_;
    ros::Subscriber sub_gps;
    ros::Subscriber sub_filtered;
    ros::Subscriber sub_imu;
    cv::Mat img;
    stringstream path_stream;
    int n = 1;
    int prev_pixel_x = 0;
    int prev_pixel_y = 0;
    int filtered_pixel_x = -1;
    int filtered_pixel_y = -1;
    int filtered_pixel_vx = -1;
    int filtered_pixel_vy = -1;
    int filtered_pixel_thx = -1;
    int filtered_pixel_thy = -1;
    int mag_pixel_thx = -1;
    int mag_pixel_thy = -1;
    bool saved = false;
    bool is_kcity;
    ros::Time t;
};

int main(int argc, char** argv) {
    if (ros::console::set_logger_level(ROSCONSOLE_DEFAULT_NAME, ros::console::levels::Debug)) {
        ros::console::notifyLoggerLevelsChanged();
    }

    ros::init(argc, argv, "map_marker");
    Map_Marker map_marker;
    while (ros::ok()){
        map_marker.save_if_end();
        ros::spinOnce();
    }
}
