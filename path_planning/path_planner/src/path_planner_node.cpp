#include "rrt_star.h"

#include <vector>
#include <ctime>
#include "ros/ros.h"
#include "std_msgs/String.h"
#include "nav_msgs/Path.h"
#include "nav_msgs/OccupancyGrid.h"
#include "nav_msgs/MapMetaData.h"
#include <nav_msgs/Odometry.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>


class RosNode{
private:
	ros::NodeHandle n;
	ros::Subscriber cost_map_sub;
	ros::Publisher path_pub;
public:
	RosNode(){
		cost_map_sub = n.subscribe("cost_map", 50000, &RosNode::costmapCallback, this);
		//cost_map_sub = n.subscribe("cost_map_with_goal_vector", 50000, &RosNode::costmapCallback, this);
		path_pub = n.advertise<nav_msgs::Path>("local_path", 1000);
	}

	void costmapCallback(const nav_msgs::OccupancyGrid & map){
		
		cout << "callback\n";
		static RRT rrt = RRT();
		int t = clock();

		// get costmap	
		vector<vector<double>> cost_map(map.info.height,vector<double>(map.info.width));
		int h = map.info.height;
		int w = map.info.width;
//		for(int i = 0; i<h; i++){
//			for(int j = 0; j<w;j++){
//				cost_map[i][j] = (double)(map.data[j*w+i] * 100 / 32.0 ) + 1;
//			}
//		}
		for(int i = 0; i<h; i++){
			for(int j = 0; j<w;j++){
				cost_map[i][j] = (double)(map.data[i*w+j]);
			}
		}
		cout << "90,198 cost ; " << cost_map[90][198] << endl;

		// rrt star algorithm
		vector<Cor> path;
		Cor x(100,0), y(100,199);
		//y.x = map.data[w*h]*2;
		//y.y = map.data[w*h + 1]*2;
		std::cout << y.x << "," << y.y << std::endl;

// !!!!!!!!!!!!!!!!!! TODO : change solve function (check line path possibility) 
		rrt.solve(path,cost_map,x, y,500);

		// convert path
		nav_msgs::Path local_path;
	
		geometry_msgs::PoseStamped poseStamped;
		int cnt = 0;
//		for(Cor cor : path){
//			poseStamped.header.seq = ++cnt;
//			poseStamped.pose.position.x = cor.x;
//			poseStamped.pose.position.y = cor.y;
//			local_path.poses.push_back(poseStamped);
//		}

		for(int i{0}; i<path.size()-1; ++i)
		{
			double stepsize{5};
			double ds{path[i].dist(path[i+1])};
			double dx{(path[i+1].x-path[i].x)*stepsize/ds};
			double dy{(path[i+1].y-path[i].y)*stepsize/ds};
			double steptime{-1};
			Cor temp{path[i]};
			while (1)
			{
				poseStamped.header.seq = ++cnt;
				poseStamped.pose.position.x = temp.x;
				poseStamped.pose.position.y = temp.y;
				local_path.poses.push_back(poseStamped);
				temp.x += dx;
				temp.y += dy;
				steptime++;
				if (steptime*stepsize > ds)
					break;
			}
		}
		poseStamped.header.seq = ++cnt;
		poseStamped.pose.position.x = path[path.size()-1].x;
		poseStamped.pose.position.y = path[path.size()-1].y;
		local_path.poses.push_back(poseStamped);
			
		poseStamped.header.seq = 0;
		local_path.poses.push_back(poseStamped);

		// publish
		path_pub.publish(local_path);

		ROS_INFO("pub, duration : %ld",clock()-t);

	}
};



int main(int argc, char **argv)
{
	cout << "path_planner_node is called\n";
	ros::init(argc, argv, "path_planner_node");
	RosNode rosnode;
	ros::spin();
	return 0;
}
