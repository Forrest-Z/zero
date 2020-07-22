#include "ros/ros.h"
#include "nmea_msgs/Sentence.h"
#include "slam/Gps.h"
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include "UTM.h"
using namespace std;


class GPS_Decoder{
    public:
    GPS_Decoder(){
        pub_ = n_.advertise<slam::Gps>("gps", 1000);
        sub_ = n_.subscribe("/nmea_sentence", 10000, &GPS_Decoder::callback, this);
    }

    void callback(const nmea_msgs::Sentence::ConstPtr& msg){
	slam::Gps rt;
	string s = msg->sentence;
        string delimiter = ",";
        vector<string> tokens;
        string token;
        size_t pos = 0;

        while ((pos = s.find(delimiter)) != std::string::npos) {
            token = s.substr(0, pos);
            tokens.push_back(token);
            s.erase(0, pos + delimiter.length());
        }
        if(tokens[0] == "$GNGGA"){
            if(stoi(tokens[6])<2){ROS_ERROR("failedByQualityIndicator(No Satellite), GPS Quality Indicator: %s", tokens[6].c_str()); return;}
            if(stoi(tokens[6])<4){ROS_WARN("warnByQualityIndicator(Non-RTK), GPS Quality Indicator: %s", tokens[6].c_str()); return;}
            if(stoi(tokens[7])<4){ROS_ERROR("failedByStatellitesN, The number of Satellite: %s", tokens[7].c_str()); return;}
            if(stoi(tokens[7])<8){ROS_WARN("warnByStatellitesN, The number of Satellite: %s", tokens[7].c_str()); return;}
            if(stod(tokens[8])>7.5 || stod(tokens[8])==0.0){ROS_ERROR("failedByHDOP, HDOP value: %s", tokens[8].c_str()); return;}
            if(stod(tokens[8])>3.5){ROS_WARN("warnByHDOP, HDOP value: %s", tokens[8].c_str()); return;}
            {   //check checksum 
                s = msg->sentence;
                delimiter = "$";
                pos = s.find(delimiter);
                s.erase(0, pos + delimiter.length());
                delimiter = "*";
                pos = s.find(delimiter);
                token = s.substr(0, pos);
                s.erase(0, pos + delimiter.length());
                int checksum = 0;
                for (string::iterator iter = token.begin(); iter != token.end(); ++iter){
                    checksum ^= (int)(*iter);
                }
                int c;
                stringstream ss;
                ss << std::hex << s;
                ss >> c;
                if(checksum != c){ROS_ERROR("FailedByChecksum, %d != %d == 0x%s", checksum, c, s.c_str()); return;}
            }
            double time_raw, lat_raw, lon_raw;
            double time, lat, lon;
            time_raw = stod(tokens[1]);
            lat_raw = stod(tokens[2]);
            lon_raw = stod(tokens[4]);
            int hour = (int)time_raw/10000;
            int min = (int)time_raw/100 - hour*100;
            double sec = time_raw -(hour*10000+min*100);

            time = hour*3600 + min*60 + sec;

            lat = (int)lat_raw/100;
            lon = (int)lon_raw/100;

            lat += (lat_raw-lat*100)/60;
            lon += (lon_raw-lon*100)/60;
            
            vector<double> xy(2);
            LatLonToUTMXY(lat, lon, 52, xy.at(0), xy.at(1));
            
            rt.header = msg->header;
            rt.x = xy.at(0);
            rt.y = xy.at(1);
            //rt.header.stamp.sec= int(time);
            //rt.header.stamp.nsec= (time-rt.header.stamp.sec)*1e9;
            //ros::Time tm = ros::Time::now();
            //rt.header.stamp.sec = tm.sec;
            //rt.header.stamp.nsec = tm.nsec;

            pub_.publish(rt);
        }
    }

    private:
    ros::NodeHandle n_;
    ros::Publisher pub_;
    ros::Subscriber sub_;
};


int main(int argc, char** argv) {
    ros::init(argc, argv, "gps_decoder");
    GPS_Decoder GPSObject;
    ros::spin();
}
