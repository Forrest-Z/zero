#include <vector>
#include "defs.h"
#include "ros/ros.h"
#include "rrt_star.h"
#include "std_msgs/UInt32.h"

using namespace std;

//
//enum taskState{
//	DRIVING_SECTION,
//	INTERSECTION_STRAIGHT,
//	INTERSECTION_LEFT,
//	INTERSECTION_RIGHT,
//	INTERSECTION_STRAIGHT_UNSIGNED,
//	INTERSECTION_LEFT_UNSIGNED,
//	INTERSECTION_RIGHT_UNSIGNED,
//	OBSTACLE_STATIC,
//	OBSTACLE_SUDDEN,
//	CROSSWALK,
//	PARKING
//};
//
//// if LEFT_LIGHT && RED_LIGHT, then light_state = 1010 (bit) = 10
//enum lightState{
//	GREEN_LIGHT,
//	LEFT_LIGHT,
//	YELLOW_LIGHT,
//	RED_LIGHT
//};
//
//enum motionState{
//	FORWARD_MOTION,
//	FORWARD_SLOW_MOTION,
//	HALT_MOTION,
//	LEFT_MOTION,
//	RIGHT_MOTION,
//	PARKING_MOTION
//};
//
//enum parkingState{
//    SEARCHING_PARKING_SPOT,
//    PARKING_SPOT_0,
//    PARKING_SPOT_1,
//    PARKING_SPOT_2,
//    PARKING_SPOT_3,
//    PARKING_SPOT_4,
//    PARKING_SPOT_5
//}
//


void pub_one_sector(ros::Publisher & pub, int sector, int num){
	ros::Rate rate(5);
	for(int i = 0;i<num;i++){
		std_msgs::UInt32 msg;
		msg.data = sector;
		pub.publish(msg);
		rate.sleep();
	}
}

void pub_two_sector(ros::Publisher & pub, int sector1, int sector2, int num){
	ros::Rate rate(5);
	for(int i = 0;i<num;i++){
		std_msgs::UInt32 msg;
		msg.data = sector1;
		pub.publish(msg);
		rate.sleep();

		std_msgs::UInt32 msg2;
		msg2.data = sector2;
		pub.publish(msg2);
		rate.sleep();
	}
}

int main(int argc, char **argv)
{
	ros::init(argc, argv, "sector_info_pub_test");
	ros::NodeHandle n;
	ros::Publisher sector_info_pub = n.advertise<std_msgs::UInt32>("sector_info", 22);

	ROS_INFO("start");

	


// A,B,A,C,A,D
	pub_one_sector(sector_info_pub, 0, 22);
	pub_two_sector(sector_info_pub, 0, 1, 6);
	pub_one_sector(sector_info_pub, 1, 22);
	pub_two_sector(sector_info_pub, 1, 0, 6);
	pub_one_sector(sector_info_pub, 0, 22);
	pub_two_sector(sector_info_pub, 0, 2, 6);
	pub_one_sector(sector_info_pub, 2, 22);
	pub_two_sector(sector_info_pub, 2, 0, 6);
	pub_one_sector(sector_info_pub, 0, 22);
	pub_two_sector(sector_info_pub, 0, 3, 6);
	pub_one_sector(sector_info_pub, 3, 22);
	pub_two_sector(sector_info_pub, 3, 0, 6);

// A,B,A,C,A,B
	pub_one_sector(sector_info_pub, 0, 22);
	pub_two_sector(sector_info_pub, 0, 1, 6);
	pub_one_sector(sector_info_pub, 1, 22);
	pub_two_sector(sector_info_pub, 1, 0, 6);
	pub_one_sector(sector_info_pub, 0, 22);
	pub_two_sector(sector_info_pub, 0, 2, 6);
	pub_one_sector(sector_info_pub, 2, 22);
	pub_two_sector(sector_info_pub, 2, 0, 6);
	pub_one_sector(sector_info_pub, 0, 22);
	pub_two_sector(sector_info_pub, 0, 1, 6);
	pub_one_sector(sector_info_pub, 1, 22);
	pub_two_sector(sector_info_pub, 1, 0, 6);

// A,D,A,E
	pub_one_sector(sector_info_pub, 0, 22);
	pub_two_sector(sector_info_pub, 0, 3, 6);
	pub_one_sector(sector_info_pub, 3, 22);
	pub_two_sector(sector_info_pub, 3, 0, 6);
	pub_one_sector(sector_info_pub, 0, 22);
	pub_two_sector(sector_info_pub, 0, 4, 6);
	pub_one_sector(sector_info_pub, 4, 22);


	return 0;
}
