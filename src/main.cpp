#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include "Eigen/Dense"
#include "measurement_package.h"
#include "ukf.h"
#include "tools.h"


using namespace std;
using Eigen::MatrixXd;
using Eigen::VectorXd;
using std::vector;



int main() {

	/*******************************************************************************
	 *  Set Measurements															 *
	 *******************************************************************************/
	vector<MeasurementPackage> measurement_pack_list;
	vector<VectorXd> estimations;
	vector<VectorXd> ground_truth;

	//Create a Tracking instance
	UKF ukf;

	Tools tools;


	double p_x,p_y,v1,v2;
	VectorXd estimate(4),RMSE(4);

	fstream out_file;
	out_file.open("obj_pose-laser-radar-output.txt");
	if (out_file.is_open()){

	  out_file<<"Px \tPy \tVx \tVy \tPx_rmse \tPy_rmse \tVx_rmse \tVy_rmse \tSensorType"<<endl;

	}else{
	  cout<<"The output log file cannot be opened"<<endl;
	}

	// hardcoded input file with laser and radar measurements
	string in_file_name_ = "obj_pose-laser-radar-synthetic-input.txt";
	ifstream in_file(in_file_name_.c_str(),std::ifstream::in);

	if (!in_file.is_open()) {
		cout << "Cannot open input file: " << in_file_name_ << endl;
	}

	string line;
	// set i to get only first 3 measurments
	int i = 0;
	while(getline(in_file, line)){

		MeasurementPackage meas_package;

		istringstream iss(line);
		string sensor_type;
		iss >> sensor_type;	//reads first element from the current line
		long long timestamp;
		if(sensor_type.compare("L") == 0){	//laser measurement
			//read measurements
			meas_package.sensor_type_ = MeasurementPackage::LASER;
			meas_package.raw_measurements_ = VectorXd(2);
			float x;
			float y;
			iss >> x;
			iss >> y;
			meas_package.raw_measurements_ << x,y;
			iss >> timestamp;
			meas_package.timestamp_ = timestamp;
			measurement_pack_list.push_back(meas_package);

		}else if(sensor_type.compare("R") == 0){
			//Skip Radar measurements

			meas_package.sensor_type_ = MeasurementPackage::RADAR;
			meas_package.raw_measurements_ = VectorXd(3);
			float ro;
			float theta;
			float ro_dot;
			iss >> ro;
			iss >> theta;
			iss >> ro_dot;
			meas_package.raw_measurements_ << ro,theta, ro_dot;
			iss >> timestamp;
			meas_package.timestamp_ = timestamp;

		}
		float x_gt;
		float y_gt;
		float vx_gt;
		float vy_gt;
		iss >> x_gt;
		iss >> y_gt;
		iss >> vx_gt;
		iss >> vy_gt;

		VectorXd gt_values(4);
		gt_values(0) = x_gt;
		gt_values(1) = y_gt;
		gt_values(2) = vx_gt;
		gt_values(3) = vy_gt;
		ground_truth.push_back(gt_values);
		i++;

		ukf.ProcessMeasurement(meas_package);

		p_x = ukf.x_(0);
		p_y = ukf.x_(1);
		v1  = ukf.x_(2);
		v2 = ukf.x_(3);

		estimate(0) = p_x;
		estimate(1) = p_y;
		estimate(2) = v1;
		estimate(3) = v2;

		estimations.push_back(estimate);

		RMSE = tools.CalculateRMSE(estimations, ground_truth);

		if (out_file.is_open()){

		        	  out_file<<p_x<<"\t";
		              out_file<<p_y<<"\t";
		              out_file<<v1<<"\t";
		              out_file<<v2<<"\t";
		              out_file<<RMSE(0)<<"\t";
		              out_file<<RMSE(1)<<"\t";
		              out_file<<RMSE(2)<<"\t";
		              out_file<<RMSE(3)<<"\t";
		              out_file<<meas_package.sensor_type_<<endl;
		}

	}


	//call the ProcessingMeasurement() function for each measurement
	size_t N = measurement_pack_list.size();
	for (size_t k = 0; k < N; ++k) { //start filtering from the second frame (the speed is unknown in the first frame)
	}

	if(in_file.is_open()){
		in_file.close();
	}
	if(out_file.is_open()){
		out_file.close();
	}

	return 0;
}
