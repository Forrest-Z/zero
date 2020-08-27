#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <set>
#include <string>
#include <fstream>
#include <iomanip>
#include <map>
#include <math.h>
#include <sstream>
#include <vector>
#include "opencv2/opencv.hpp"

#include "geometry_msgs/Point.h"
#include "slam/Cluster.h"
#include "slam/Clusters.h"
#include "slam/Lidar.h"
#include "slam/LidarPoint.h"
#include "slam/Data.h"
#include "slam/ParkingSpot.h"
#include "std_msgs/UInt32.h"
#include "std_msgs/Int32.h"

#include "ros/ros.h"
#include "ros/time.h"
#include "ros/package.h"


using namespace std;

typedef pair<double, double> pdd;

class ParkingSpotDetector{
    private:
    ros::NodeHandle nh;
    ros::Publisher pub;
    ros::Subscriber sub_position;
    ros::Subscriber sub_lidar;
    ros::Subscriber sub_mission;
    bool find_parking_spot;
    int count_spot;
    vector<slam::ParkingSpot> parking_spot;
    stringstream path_stream;
    bool is_kcity;
    const char delimiter = ' ';
    pdd position;
    double heading;
    int empty_spot;
    int spot_threshold;
    bool is_parking_mission;
    double lidar_threshold;


    public:
    ParkingSpotDetector(){
        pub = nh.advertise<std_msgs::Int32>("/parking_spot", 10);
        sub_position = nh.subscribe("/filtered_data", 1, &ParkingSpotDetector::callback_position, this);
        sub_lidar = nh.subscribe("/point_cloud_clusters", 1, &ParkingSpotDetector::callback_lidar, this);
        sub_mission = nh.subscribe("/mission_state", 1, &ParkingSpotDetector::callback_mission, this);
        find_parking_spot = false;
        is_kcity = false;
        parking_spot.clear();
        count_spot = 0;
        empty_spot = -1;
        spot_threshold = 50;
        lidar_threshold = 10.0;
        is_parking_mission = false;
        if(is_kcity) path_stream << ros::package::getPath("slam")<<"/config/KCity/KCity_parking_spot.txt";
        else path_stream << ros::package::getPath("slam")<<"/config/FMTC/FMTC_parking_spot.txt";
        load_parking_spot();
    }

    void load_parking_spot(){
        string in_line;
        ifstream in(path_stream.str());
        while(getline(in, in_line)){
            stringstream ss(in_line);
            string token;
            vector<string> result;
            while(getline(ss,token,delimiter)){
                result.push_back(token);
            }
            slam::ParkingSpot spot;
            if(result.size()==0) break;
            spot.x = stod(result.at(0));
            spot.y = stod(result.at(1));
            spot.theta = stod(result.at(2));
            spot.width = stod(result.at(3));
            spot.height = stod(result.at(4));
            spot.available = true;
            parking_spot.push_back(spot);
            count_spot ++;
        }
    }

    void callback_mission(const std_msgs::UInt32::ConstPtr& msg){
        if((msg->data >> 8) == 10) is_parking_mission = true;
        else {
            is_parking_mission = false;
            find_parking_spot = false;
            empty_spot = -1;
            for(slam::ParkingSpot spot : parking_spot) spot.available = true;
        }
    }

    void callback_position(const slam::Data::ConstPtr& msg){
        position = {msg->x, msg->y};
        heading = msg->theta;
    }

    void callback_lidar(const slam::Clusters::ConstPtr& msg){
        std_msgs::Int32 rt;
        if(!is_parking_mission){
            rt.data = empty_spot;
            pub.publish(rt);
            return;
        }
        if(find_parking_spot) {
            rt.data = empty_spot;
            pub.publish(rt);
            return;
        }
        for(slam::ParkingSpot spot : parking_spot) spot.count = 0;
        for(slam::Cluster cluster : msg->clusters){
            for(slam::LidarPoint point : cluster.points){
                double x = position.first + point.point_2d.x*cos(heading) - point.point_2d.y*sin(heading);
                double y = position.second + point.point_2d.x*sin(heading) + point.point_2d.y*cos(heading);
                for(slam::ParkingSpot spot : parking_spot){
                    double horizontal_distance = abs((x-spot.x)*cos(spot.theta) + (y-spot.y)*sin(spot.theta));
                    double vertical_distance = abs((x-spot.x)*sin(spot.theta)-(y-spot.y)*cos(spot.theta));
                    if(horizontal_distance < spot.width/2 && vertical_distance < spot.height/2) spot.count ++;
                }
            }
        }
        for(slam::ParkingSpot spot : parking_spot){
            if(!spot.available) continue;
            if(spot.count > spot_threshold) spot.available = false;
        }
        for(int index=0; index<count_spot ; index++){
            slam::ParkingSpot spot = parking_spot.at(index);
            if(!spot.available) continue;
            if(sqrt(pow(position.first-spot.x,2)+pow(position.second-spot.y,2))>lidar_threshold - sqrt(pow(spot.width/2,2)+pow(spot.height/2,2))) continue;
            if((spot.x-position.first)*cos(heading)+(spot.y-position.second)*sin(heading)<0) continue;
            empty_spot = index;
            rt.data = empty_spot;
            find_parking_spot = true;
            pub.publish(rt);
            return;
        }
        rt.data = -1;
        pub.publish(rt);
    }
};


int main(int argc, char **argv){
    ros::init(argc, argv, "parking_spot_detector");
    ParkingSpotDetector parking_spot_detector;
    ros::spin();
}
