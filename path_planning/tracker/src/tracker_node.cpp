#include <ros/ros.h>
#include <iostream>
#include <cmath>
#include <ctime>
#include <std_msgs/String.h>
#include <std_msgs/Float32.h>
#include <nav_msgs/Path.h>
#include <nav_msgs/Odometry.h>
#include <geometry_msgs/Point.h>

using namespace std;
using namespace std_msgs;
using namespace nav_msgs;
using namespace geometry_msgs;

/*
struct Point
{
	// the variables to represent relative location and direction
	double x;
	double y;
	double theta; 
}
*/

class Tracker
{
	private:
		ros::NodeHandle nh;
		
		Float32 steering_angle;
		Path curr_local_path;
		Odometry curr_odom;

		// configuration constants
		double look_ahead_oval_ratio{2}; // ratio of look ahead area which is oval shape
		double upper_radius{160};
		double lower_radius{80};

		double current_vel{0};
		Point look_ahead_point{Point()};
		double curvature{0};
		double nonslip_steering_angle{0}; // right side is positive
		double rotational_radius{0};

	public:

		ros::Publisher steering_angle_pub;
		ros::Subscriber local_path_sub;
		ros::Subscriber odometry_sub;


		// initializer
		Tracker() 
			:steering_angle(Float32()), curr_local_path(Path()), curr_odom(Odometry()),
				look_ahead_oval_ratio(0)
			{
				steering_angle_pub = nh.advertise<Float32>("configuration",1000);
				local_path_sub = nh.subscribe("local_path",100,&Tracker::local_path_callback,this);
				odometry_sub = nh.subscribe("odometory",100,&Tracker::odometory_callback,this);
			}

		// setter function
		void set_steering_angle(const double angle) {steering_angle.data = angle;}
		void set_curr_local_path(const Path& path) {curr_local_path=path;}
		void set_curr_odom(const Odometry odom) {curr_odom = odom;}
		// getter function
		const Float32 get_steering_angle() {return steering_angle;}
		const Path get_current_path() {return curr_local_path;}
		const Odometry get_current_odom() {return curr_odom;}
		// callback function
		void local_path_callback(const Path::ConstPtr msg);
		void odometory_callback(const Odometry::ConstPtr msg);

		// Fuctions for determind steering angle
		// 1. set look ahead point under considering velocity
		void set_look_ahead_point();
		// 2. solve pure pursuit and yield curvature
		void solve_pure_pursuit();
		// 3. using experimental result to determind actual steering angle
		void adjust_steering_angle(); 
		// 1~3. determind steering angle
		void determind_steering_angle();

		double determind_major_axis_radius();
};

void Tracker::local_path_callback(const Path::ConstPtr msg)
{
	cout << "local_path_callback is called\n";
	//set_curr_local_path(msg);
	curr_local_path = Path();
	curr_local_path.header = msg->header;
	int i{0};
	while(1)
	{
		if(msg->poses[i].header.seq != 0)
		{
			curr_local_path.poses.push_back(msg->poses[i]);
		}
		else
		{
			ROS_INFO("%x",msg->poses[i].header.seq);
			curr_local_path.poses.push_back(msg->poses[i]);
			break;
		}
		i++;
	}
}

void Tracker::odometory_callback(const Odometry::ConstPtr msg)
{
	cout << "odometry_callback is called\n";
	//set_curr_odom(msg);
	curr_odom.header = msg->header;
	curr_odom.pose = msg->pose;
	curr_odom.twist = msg->twist;

	int time{static_cast<int>(clock())};
	
	cout << "main_check1, ";
	determind_steering_angle();
	cout << "main_check2, ";
	steering_angle_pub.publish(get_steering_angle());
	cout << "main_check3" << endl;
	cout << "current_vel : " << current_vel << endl;
	cout << "look_ahead_point : (" << look_ahead_point.x << "," << look_ahead_point.y << ")\n";
	cout << "steering_angle : " << steering_angle << endl;
	cout << "duration time : " << (time-clock())/double(CLOCKS_PER_SEC) << endl << endl;


}

// input : curren_vel, variables related to look ahead area, curr_local_path
// output : look_ahead_point
void Tracker::set_look_ahead_point()
{
	current_vel = sqrt((curr_odom.twist.twist.linear.x)*(curr_odom.twist.twist.linear.x)+(curr_odom.twist.twist.linear.y)*(curr_odom.twist.twist.linear.y));
	double major_axis_radius{determind_major_axis_radius()};
	int idx{0};
	double check_outside{0};
	while(1)
	{
		if (curr_local_path.poses[idx].header.seq == -1)
		{
			major_axis_radius -= 50;
			idx = 0;
			continue;
		}
		check_outside = (curr_local_path.poses[idx].pose.position.x)*(curr_local_path.poses[idx].pose.position.x)/((major_axis_radius/look_ahead_oval_ratio)* (major_axis_radius/look_ahead_oval_ratio))
			+ (curr_local_path.poses[idx].pose.position.y)*(curr_local_path.poses[idx].pose.position.y)/((major_axis_radius)*(major_axis_radius));
		if (check_outside>1)
		{
			look_ahead_point.x = curr_local_path.poses[idx].pose.position.x;
			look_ahead_point.y = curr_local_path.poses[idx].pose.position.y;
			break;
		}
		idx++;
	}
}

// input : look_ahead_point
// output : rotational_radius, curvature, nonslip_steering_angle
void Tracker::solve_pure_pursuit()
{
	Point rotational_center{Point()};
	rotational_center.x = look_ahead_point.x/2.0 - look_ahead_point.y*(-1.7*100/3.0-look_ahead_point.y/2.0)/double(look_ahead_point.x);
	rotational_center.y = -1.7*100/3.0;
	curvature = 1/(sqrt(rotational_center.x*rotational_center.x+rotational_center.y*rotational_center.y));
	rotational_radius = 1/curvature;
	double temp_angle =  atan2(-rotational_center.y,rotational_center.x);
	if (temp_angle < 3.141592/2.0)
		nonslip_steering_angle = temp_angle;
	else
		nonslip_steering_angle = temp_angle-3.141592;
	cout << "atan2(" << -rotational_center.y << "," << rotational_center.x << ")\n";
}

// input : curvature, current_vel (!!!!!!!!!! discussion is required. choose between current_vel vs goal_vel)
// output : steering_angle
void Tracker::adjust_steering_angle()
{
	// experimental result should be reflected
	cout << "Experimental result should be reflected.\n";
	steering_angle.data = nonslip_steering_angle;
}

// capsulized module
void Tracker::determind_steering_angle()
{
	set_look_ahead_point();
	solve_pure_pursuit();
	adjust_steering_angle();
}

// input : current_vel
// output : major_axis_radius
// check is needed. 
double Tracker::determind_major_axis_radius()
{
	double major_axis_radius{0};
	major_axis_radius = lower_radius + (upper_radius-lower_radius)*current_vel/20;
	return major_axis_radius;
}


int main(int argc, char *argv[])
{
	int count{0};
	
	ros::init(argc,argv,"path_traker");
	ros::NodeHandle nh;

	Tracker tracker{Tracker()};
	ros::Rate loop_rate(10);

	ros::spin();

//	while (ros::ok())
//	{
//		ros::spinOnce();
//		loop_rate.sleep();
//		count++;
//	}

	return 0;
}