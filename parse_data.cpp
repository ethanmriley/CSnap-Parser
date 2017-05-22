#include <fstream>
#include <vector>
#include <string>
#include <cassert>
#include <iostream>
#include <cmath>
#include <cstdlib>

//parser for polar coordinate data exported from RPI CSDT CSnap application

//function to determine distance between two points
float distanceBetween(const float radius_1, const float angle_1, const float radius_2, const float angle_2) {
	//convert to cartesian to make distance calculation easier
	const float x1 = radius_1*cos(angle_1 * 3.14159/180.0);		
	const float x2 = radius_2*cos(angle_2 * 3.14159/180.0);	
	const float y1 = radius_1*sin(angle_1 * 3.14159/180.0);
	const float y2 = radius_2*sin(angle_2 * 3.14159/180.0);
	float arg = pow((x2-x1), 2) + pow((y2-y1), 2);
	float distance = pow(arg, .5);
	return distance;
}

/*
	CSnap data has a lot of extraneous points, and that makes it difficult to print the patterns quickly.
	This will take points close to one another, determined by some threshold, min_distance, and remove them.
*/

void simplifyPoints(std::vector<float>& radii, std::vector<float>& angles, const float min_distance) {
	std::vector<float> valid_radii;
	std::vector<float> valid_angles;

	for(unsigned int i = 0; i < radii.size(); i++) {

		bool point_is_valid = true;

		for (unsigned int j = 0; j < radii.size(); j++) { //set a flag if radii[i] is too close to any other point in the pattern

			float distance = distanceBetween(radii[i], angles[i], radii[j], angles[j]);
			
			if(distance < min_distance && i!= j) {
				point_is_valid = false;
			}
		}

		if (point_is_valid) {
			valid_radii.push_back(radii[i]);
			valid_angles.push_back(angles[i]);
		}
	}

	radii = valid_radii;
	angles = valid_angles;
}

void writeToOutput(std::ofstream& output, const std::vector<float>& radii, const std::vector<float>& angles) {
	assert(radii.size() == angles.size());
	
	/*
		iterate through the points, checking for duplicates
	*/

	std::vector<unsigned int> duplicates;
	assert(radii.size() == angles.size());
	for(unsigned int k = 0; k < radii.size(); k++) {
		for(unsigned int l = 0; l < radii.size(); l++) {
			if (radii[k] == radii[l] && angles[k] == angles[l] && k != l) {
				duplicates.push_back(k);
			}
		}
	}

	for(unsigned int i = 0; i < radii.size(); i++) {
		
		/*
			check if the point is a duplicate
		*/

		bool is_duplicate = false;

		for(unsigned int j = 0; j < duplicates.size(); j++) {
			if(i == duplicates[j]) {
				is_duplicate = true;
			}
		}

		if (not(is_duplicate)) {
			output << radii[i] << ',' << angles[i];
			if(i != radii.size() - 1) {
				output << std::endl;
			}
		}
	}
}

/*
	function to read the data exported from CSnap into two vectors, radii and angles
*/

void readFile(std::ifstream& input, std::vector<float>& radii, std::vector<float>& angles) {
	
	bool is_radius = true; //boolean flag to switch between radius and angle vectors
	char c;
	
	while(input.get(c)) {
		std::string val = "";
		if (c != '_' && c != ',') { //radius and angle are separated by '_'; coordinate pairs by ','
			while(c != '_' && c != ',' && input.good()) { //read variable-length value from file
				val += c;
				input.get(c);
			}

			float f_val = strtof(val.c_str(), NULL); //convert string to float

			if(is_radius) { //check whether the last value added was a radius or an angle, and add the value to the appropriate vector
				radii.push_back(f_val);
				is_radius = false;
			} else {
				angles.push_back(f_val);
				is_radius = true;
			}
		}
	}
}

int main(int argc, char* argv[]) {
	std::ofstream output_file;
	std::ifstream input_file;
	std::vector<float> radii;
	std::vector<float> angles;
	std::string input_file_name = argv[1];
	std::string output_file_name = argv[2];
	float min_distance = strtof(argv[3], NULL);
	
	input_file.open(input_file_name.c_str());
	output_file.open(output_file_name.c_str());
	
	readFile(input_file, radii, angles);
	simplifyPoints(radii, angles, min_distance);
	writeToOutput(output_file, radii, angles);
};