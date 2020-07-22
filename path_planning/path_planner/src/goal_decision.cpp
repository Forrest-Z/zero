#include "rrt_star.h"
#include <vector>
#include <cmath>
#include "ros/ros.h"
#include "nav_msgs/Path.h"
#include "nav_msgs/Odometry.h"
#include "geometry_msgs/PoseStamped.h"

const static int OBSTACLE = 100;

enum taskState{
	DRIVING_SECTION,
	INTERSECTION_STRAIGHT,
	INTERSECTION_LEFT,
	INTERSECTION_RIGHT,
	INTERSECTION_STRAIGHT_UNSIGNED,
    INTERSECTION_LEFT_UNSIGNED,
    INTERSECTION_RIGHT_UNSIGNED,
	OBSTACLE_STATIC,
	OBSTACLE_SUDDEN,
	CROSSWALK,
	PARKING
};

enum lightState{
	GREEN_LIGHT,
	LEFT_LIGHT,
	YELLOW_LIGHT,
	RED_LIGHT
};

enum motionState{
	FORWARD_MOTION,
	FORWARD_MOTION_SLOW,
	HALT_MOTION,
	LEFT_MOTION,
	RIGHT_MOTION,
	PARKING_MOTION
};

enum parkingState{
	SEARCHING_PARKING_SPOT,
    PARKING_SPOT_0,
    PARKING_SPOT_1,
    PARKING_SPOT_2,
    PARKING_SPOT_3,
    PARKING_SPOT_4,
    PARKING_SPOT_5
}


Cor decision(const vector<geometry_msgs::PoseStamped> & goals, const vector<vector<double>> & costmap, int task, int light, int motion, int x, int y, double angle){
	///////////////////////////////////////////////
	static bool parking_complished = false;
	static bool unparking_complished = false;
	///////////////////////////////////////////////
	bool flag[12];
	for(int i = 0;i<5;i++) flag[i] = false;
	switch(motion){
		case FORWARD_MOTION :
		case FORWARD_MOTION_SLOW :
			flag[0] = true;
			flag[1] = true;
			flag[4] = true;
			break;
		case HALT_MOTION :
			flag[0] = true;
			flag[1] = true;
			break;
		case LEFT_MOTION :
			flag[0] = true;
			flag[1] = true;
			flag[2] = true;
			break;
		case RIGHT_MOTION :
			flag[0] = true;
			flag[1] = true;
			flag[3] = true;
			break;
		//////////////////////////////
		case PARKING_MOTION :
			switch(parking_space){
				case SEARCHING_PARKING_SPOT :
					flag[0] = true;
					flag[1] = true;
					break;
				case PARKING_SPOT_0 :
					flag[5] = true;
					break;
				case PARKING_SPOT_1 :
					flag[6] = true;
					break;
				case PARKING_SPOT_2 :
					flag[7] = true;
					break;
				case PARKING_SPOT_3 :
					flag[8] = true;
					break;
				case PARKING_SPOT_4 :
					flag[9] = true;
					break;
				case PARKING_SPOT_5 :
					flag[10] = true;
					break;
			}
			break;
		//////////////////////////////
	}

	double look_ahead_radius;
	if(motion == FORWARD_MOTION_SLOW) look_ahead_radius = 50;
	else if(motion == LEFT_MOTION || motion == RIGHT_MOTION) look_ahead_radius = 100;
	///////////////////////////////////////
	else if(motion == PARKING_MOTION) look_ahead_radius = 30;
	///////////////////////////////////////
	else look_ahead_radius = 200;


	// get value index which is closest to look_ahead_radius (except sub path), get value_sub for sub path
	double value = -1, value_sub = -1;
	double key = 100000, key_sub = 100000;
	int sz = goals.size();
	for(int i = 0; i<sz;i++){
		geometry_msgs::PoseStamped poseStamped = goals[i];		

		// check flag
		if(!flag[poseStamped.header.seq]) continue;

		double goal_angle = poseStamped.pose.orientation.z;
		double ang_diff = angle - goal_angle;
		ang_diff = min(abs(ang_diff), min(abs(ang_diff + 2 * M_PI), abs(ang_diff - 2 * M_PI)));
		
		// check if same dir
		if(ang_diff > M_PI/2) continue;
		
		// check if on path
		double dx = poseStamped.pose.orientation.x;
		double dy = poseStamped.pose.orientation.y;
		/////////////////////////////////////////////////
		if(motion != PARKING_MOTION){
			if((dx - x) * cos(angle) + (dy-y) * sin(angle) <= 0) continue;
		}
		else
		{
			if(!parking_complished){
				if((dx - x) * cos(angle) + (dy-y) * sin(angle) <= 0) continue;
			}
			else
				if((dx - x) * cos(angle) + (dy-y) * sin(angle) >= 0) continue;
		}
		/////////////////////////////////////////////////

		// check obstacle
		if(costmap[(int)dx][(int)dy] >= OBSTACLE) continue;

		double dist = sqrt((dx-x)*(dx-x) + (dy-y)*(dy-y));

		// not sub path
		if(flag[poseStamped.header.seq]!=1){
			if(abs(dist - look_ahead_radius) > key) continue;
			key = abs(dist - look_ahead_radius);
			value = i;
		}

		// sub path
		else{
			if(abs(dist - look_ahead_radius) > key_sub) continue;
			key_sub = abs(dist - look_ahead_radius);
			value_sub = i;
		}
	}

	if(value != -1){
		double gx = goals[value].pose.orientation.x;
		double gy = goals[value].pose.orientation.y;
		return Cor(gx,gy);
	}


	if(task == OBSTACLE_STATIC){
		if(value_sub == -1) {
			return Cor(100,0);		
		}
		double gx = goals[value_sub].pose.orientation.x;
		double gy = goals[value_sub].pose.orientation.y;
		return Cor(gx,gy);
	}
	else if(task == OBSTACLE_SUDDEN){
		return Cor(100,0);
	}
	/////////////////////////////////////////
	// when parking complished or unparking complished stop!
	else if(motion==PARKING_MOTION){
		if(parking_complished==false){
			parking_complished = true;
			return Cor(100,0);
		}
		else{
			unparking_complished = true;
			return Cor(100,0);
		}
	}
	/////////////////////////////////////////
	// when there is no goal available stop! ////////////////////////should be checked!!
	else{
		// stop!!!!
		return Cor(100,0);
	}
}


































