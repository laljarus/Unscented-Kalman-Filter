#include "ukf.h"
#include "Eigen/Dense"
#include <iostream>

using namespace std;
using Eigen::MatrixXd;
using Eigen::VectorXd;
using std::vector;

/**
 * Initializes Unscented Kalman filter
 * This is scaffolding, do not modify
 */
UKF::UKF() {
  // if this is false, laser measurements will be ignored (except during init)
  use_laser_ = true;

  // if this is false, radar measurements will be ignored (except during init)
  use_radar_ = true;

  // initial state vector
  x_ = VectorXd(5);

  // initial covariance matrix
  P_ = MatrixXd(5, 5);

  // Process noise standard deviation longitudinal acceleration in m/s^2
  std_a_ = 1.5;

  // Process noise standard deviation yaw acceleration in rad/s^2
  std_yawdd_ = 1.5;
  
  //DO NOT MODIFY measurement noise values below these are provided by the sensor manufacturer.
  // Laser measurement noise standard deviation position1 in m
  std_laspx_ = 0.15;

  // Laser measurement noise standard deviation position2 in m
  std_laspy_ = 0.15;

  // Radar measurement noise standard deviation radius in m
  std_radr_ = 0.3;

  // Radar measurement noise standard deviation angle in rad
  std_radphi_ = 0.03;

  // Radar measurement noise standard deviation radius change in m/s
  std_radrd_ = 0.3;
  //DO NOT MODIFY measurement noise values above these are provided by the sensor manufacturer.
  
  /**
  TODO:

  Complete the initialization. See ukf.h for other member properties.

  Hint: one or more values initialized above might be wildly off...
  */
  is_initialized_ = false;

  previous_timestamp_ = 0;

  n_x_ = 5;
  n_aug_ = 7;
  n_z_ = 3;
  lambda_ = 3 - n_aug_;

  weights_ = VectorXd(2*n_aug_+1);

  double weight_0 = lambda_/(lambda_+n_aug_);
  weights_(0) = weight_0;
  for (int i=1; i<2*n_aug_+1; i++) {  //2n+1 weights
    double weight = 0.5/(n_aug_+lambda_);
    weights_(i) = weight;
  }

  R_laser_ = MatrixXd(2, 2);
  R_radar_ = MatrixXd(3, 3);
  H_laser_ = MatrixXd(2, 5);
  Xsig_pred_ = MatrixXd(n_x_,2*n_aug_+1);

  R_laser_<< std_laspx_*std_laspx_,0,
		     0, std_laspx_*std_laspx_;

  R_radar_<< std_radr_*std_radr_,0,0,
		     0, std_radphi_*std_radphi_,0,
			 0,0, std_radrd_*std_radrd_;

  H_laser_<<1,0,0,0,0,
  		    0,1,0,0,0;

  Q_ = MatrixXd(2,2);
  Q_ << pow(std_a_,2),0,
       0,pow(std_yawdd_,2);

  x_<< 0,0,0,0,0;

  P_<< 1,0,0,0,0,
	   0,1,0,0,0,
	   0,0,1,0,0,
	   0,0,0,1,0,
	   0,0,0,0,1;
  Xsig_pred_.fill(0.0);


}

UKF::~UKF() {}

/**
 * @param {MeasurementPackage} meas_package The latest measurement data of
 * either radar or laser.
 */
void UKF::ProcessMeasurement(MeasurementPackage meas_package) {
  /**
  TODO:

  Complete this function! Make sure you switch between lidar and radar
  measurements.
  */
  if(!is_initialized_){

	if(meas_package.sensor_type_ == MeasurementPackage::RADAR){

		double rho = meas_package.raw_measurements_[0];
		double theta = meas_package.raw_measurements_[1];
		double rho_dot = meas_package.raw_measurements_[2];

		x_(0) = rho*cos(theta);
		x_(1) = rho*sin(theta);
		x_(2) = rho_dot;

	}else {

		x_(0) = meas_package.raw_measurements_[0];
		x_(1) = meas_package.raw_measurements_[1];

	}
	previous_timestamp_ = meas_package.timestamp_;
	is_initialized_ = true;
	return;
  }
  float dt = (meas_package.timestamp_ - previous_timestamp_) / 1000000.0;	//dt - expressed in seconds
  previous_timestamp_ = meas_package.timestamp_;
  Prediction(dt);

  if ((meas_package.sensor_type_ == MeasurementPackage::RADAR)&(use_radar_)) {
	  UpdateRadar(meas_package);
  }else if ((meas_package.sensor_type_ == MeasurementPackage::LASER)&(use_laser_)){
	  UpdateLidar(meas_package);
  }
}

/**
 * Predicts sigma points, the state, and the state covariance matrix.
 * @param {double} delta_t the change in time (in seconds) between the last
 * measurement and this one.
 */
void UKF::Prediction(double delta_t) {
  /**
  TODO:

  Complete this function! Estimate the object's location. Modify the state
  vector, x_. Predict sigma points, the state, and the state covariance matrix.
  */

  // Calculation of augmented sigma points from the state vector and P matrix

  MatrixXd Xsig_aug_ = MatrixXd(n_aug_, 2 * n_aug_ + 1);
  AugmentedSigmaPoints(Xsig_aug_);
  SigmaPointPrediction(delta_t, Xsig_aug_,Xsig_pred_);
  PredictMeanAndCovariance(Xsig_pred_,x_,P_);

}

/**
 * Updates the state and the state covariance matrix using a laser measurement.
 * @param {MeasurementPackage} meas_package
 */
void UKF::UpdateLidar(MeasurementPackage meas_package) {
  /**
  TODO:

  Complete this function! Use lidar data to update the belief about the object's
  position. Modify the state vector, x_, and covariance, P_.

  You'll also need to calculate the lidar NIS.
  */
  VectorXd z = meas_package.raw_measurements_;
  VectorXd y = z - H_laser_ * x_;
  MatrixXd Ht = H_laser_.transpose();
  MatrixXd S = H_laser_ * P_ * Ht + R_laser_;
  MatrixXd Si = S.inverse();
  MatrixXd PHt = P_ * Ht;
  MatrixXd K = PHt * Si;

  //new estimate
  x_ = x_ + (K * y);
  long x_size = x_.size();
  MatrixXd I = MatrixXd::Identity(x_size, x_size);
  P_ = (I - K * H_laser_) * P_;
}

/**
 * Updates the state and the state covariance matrix using a radar measurement.
 * @param {MeasurementPackage} meas_package
 */
void UKF::UpdateRadar(MeasurementPackage meas_package) {
  /**
  TODO:

  Complete this function! Use radar data to update the belief about the object's
  position. Modify the state vector, x_, and covariance, P_.

  You'll also need to calculate the radar NIS.
  */
  VectorXd z_pred = VectorXd(n_z_);
  MatrixXd S = MatrixXd(n_z_,n_z_);
  MatrixXd Zsig = MatrixXd(n_z_,2*n_aug_+1);
  PredictRadarMeasurement(Xsig_pred_,z_pred,S,Zsig);


  MatrixXd Tc = MatrixXd(n_x_, n_z_);
  Tc.fill(0.0);
  for(int i = 0;i<2*n_aug_+1;i++){
      MatrixXd x_diff = (Xsig_pred_.col(i) - x_);
      MatrixXd z_diff = (Zsig.col(i) - z_pred);

      //angle normalization
      while (x_diff(3)> M_PI) x_diff(3)-=2.*M_PI;
      while (x_diff(3)<-M_PI) x_diff(3)+=2.*M_PI;


      //angle normalization
      while (z_diff(1)> M_PI) z_diff(1)-=2.*M_PI;
      while (z_diff(1)<-M_PI) z_diff(1)+=2.*M_PI;

      Tc += weights_(i)*x_diff*z_diff.transpose();
  }
  //calculate Kalman gain K;

   MatrixXd k = Tc*S.inverse();

   //update state mean and covariance matrix
   VectorXd z = meas_package.raw_measurements_;
   VectorXd z_diff = z - z_pred;

   //angle normalization
   while (z_diff(1)> M_PI) z_diff(1)-=2.*M_PI;
   while (z_diff(1)<-M_PI) z_diff(1)+=2.*M_PI;

   x_ = x_ + k * z_diff;

   P_ = P_ - k*S*k.transpose();

   nis_ = z_diff.transpose()*S.inverse()*z_diff;


}


void UKF::AugmentedSigmaPoints(MatrixXd& Xsig_out){

	// Augmented state vector
	VectorXd x_aug = VectorXd(n_aug_);

	//augmented state covariance matrix
	MatrixXd P_aug = MatrixXd(n_aug_,n_aug_);

	//create sigma point matrix
	MatrixXd Xsig_aug = MatrixXd(n_aug_, 2 * n_aug_ + 1);

	x_aug.fill(0.0);
	x_aug.head(n_x_) = x_;

	P_aug.fill(0.0);
	P_aug.topLeftCorner(n_x_,n_x_) = P_;
	P_aug.bottomRightCorner(n_aug_- n_x_, n_aug_- n_x_) = Q_;

	//create square root matrix
	MatrixXd A = P_aug.llt().matrixL();

	//create augmented sigma points
	Xsig_aug.fill(0.0);
	Xsig_aug.col(0)  = x_aug;

	//set remaining sigma points
	for (int i = 0; i < n_aug_; i++)
	{
	  Xsig_aug.col(i+1)     = x_aug + sqrt(lambda_+n_aug_) * A.col(i);
	  Xsig_aug.col(i+1+n_aug_) = x_aug - sqrt(lambda_+n_aug_) * A.col(i);
	}

	Xsig_out = Xsig_aug;

}

void UKF::SigmaPointPrediction(double delta_t, MatrixXd Xsig_in, MatrixXd& Xsig_out){

	MatrixXd Xsig_pred = MatrixXd(n_x_,2*n_aug_+1);
	Xsig_pred.fill(0.0);

	//predict sigma points
	for (int i = 0; i< 2*n_aug_+1; i++)
	{
	  //extract values for better readability
	  double p_x = Xsig_in(0,i);
	  double p_y = Xsig_in(1,i);
	  double v = Xsig_in(2,i);
	  double yaw = Xsig_in(3,i);
	  double yawd = Xsig_in(4,i);
	  double nu_a = Xsig_in(5,i);
	  double nu_yawdd = Xsig_in(6,i);

	  //predicted state values
	  double px_p, py_p;

	  //avoid division by zero
	  if (fabs(yawd) > 0.001) {
	      px_p = p_x + v/yawd * ( sin (yaw + yawd*delta_t) - sin(yaw));
	      py_p = p_y + v/yawd * ( cos(yaw) - cos(yaw+yawd*delta_t) );
	  }
	  else {
	      px_p = p_x + v*delta_t*cos(yaw);
	      py_p = p_y + v*delta_t*sin(yaw);
	  }

	  double v_p = v;
	  double yaw_p = yaw + yawd*delta_t;
	  double yawd_p = yawd;

	  //add noise
	  px_p = px_p + 0.5*nu_a*delta_t*delta_t * cos(yaw);
	  py_p = py_p + 0.5*nu_a*delta_t*delta_t * sin(yaw);
	  v_p = v_p + nu_a*delta_t;

	  yaw_p = yaw_p + 0.5*nu_yawdd*delta_t*delta_t;
	  yawd_p = yawd_p + nu_yawdd*delta_t;

	  //write predicted sigma point into right column
	  Xsig_pred(0,i) = px_p;
	  Xsig_pred(1,i) = py_p;
	  Xsig_pred(2,i) = v_p;
	  Xsig_pred(3,i) = yaw_p;
	  Xsig_pred(4,i) = yawd_p;
	}

	Xsig_out = Xsig_pred;

}

void UKF::PredictMeanAndCovariance(MatrixXd Xsig_pred_in,VectorXd& x_pred, MatrixXd& P_pred){
	// vector for storing difference between current sigma point and mean
	VectorXd x_diff = VectorXd(n_x_);
	VectorXd x = VectorXd(n_x_);
	MatrixXd P = MatrixXd(n_x_,n_x_);
	x_diff.fill(0.0);
	x.fill(0.0);
	P.fill(0.0);

	for(int i = 0;i<2*n_aug_+1;i++){
		  x += weights_(i)*Xsig_pred_in.col(i);
	}

	for(int i = 0; i<2*n_aug_+1;i++){
	  x_diff = (Xsig_pred_in.col(i)-x);

	  //angle normalization
	  while (x_diff(3)> M_PI) x_diff(3)-=2.*M_PI;
	  while (x_diff(3)<-M_PI) x_diff(3)+=2.*M_PI;

	  P += weights_(i)*x_diff*x_diff.transpose();

	}
	x_pred = x;
	P_pred = P;

}
void UKF::PredictRadarMeasurement(MatrixXd& Xsig_pred_in, VectorXd& z_out, MatrixXd& S_out,MatrixXd& Zsig_out){


	MatrixXd Zsig = MatrixXd(n_z_,2*n_aug_+1);
	Zsig.fill(0.0);

	//transform sigma points into measurement space
	for (int i = 0; i < 2 * n_aug_ + 1; i++) {  //2n+1 simga points

	  // extract values for better readibility
	  double p_x = Xsig_pred_in(0,i);
	  double p_y = Xsig_pred_in(1,i);
	  double v  = Xsig_pred_in(2,i);
	  double yaw = Xsig_pred_in(3,i);

	  double v1 = cos(yaw)*v;
	  double v2 = sin(yaw)*v;

	  // measurement model
	  Zsig(0,i) = sqrt(p_x*p_x + p_y*p_y);                        //r
	  Zsig(1,i) = atan2(p_y,p_x);                                 //phi
	  Zsig(2,i) = (p_x*v1 + p_y*v2 ) / sqrt(p_x*p_x + p_y*p_y);   //r_dot
	}

	//mean predicted measurement
	VectorXd z_pred = VectorXd(n_z_);
	z_pred.fill(0.0);
	for (int i=0; i < 2*n_aug_+1; i++) {
	    z_pred = z_pred + weights_(i) * Zsig.col(i);
	}

	//innovation covariance matrix S
	MatrixXd S = MatrixXd(n_z_,n_z_);
	S.fill(0.0);
	for (int i = 0; i < 2 * n_aug_ + 1; i++) {  //2n+1 simga points
	  //residual
	  VectorXd z_diff = Zsig.col(i) - z_pred;

	  //angle normalization
	  while (z_diff(1)> M_PI) z_diff(1)-=2.*M_PI;
	  while (z_diff(1)<-M_PI) z_diff(1)+=2.*M_PI;

	  S = S + weights_(i) * z_diff * z_diff.transpose();
	}
	S = S + R_radar_;
	z_out = z_pred;
	S_out = S;
	Zsig_out = Zsig;

}
