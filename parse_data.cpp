#include <fstream>
#include <vector>
#include <string>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>

//parser for polar coordinate data exported from RPI CSDT CSnap application

void removeDuplicates(std::vector<float>& radii, std::vector<float>& angles) {
	std::vector<float> unique_radii;
	std::vector<float> unique_angles;

	assert(radii.size() == angles.size());
	for(unsigned int i = 0; i < radii.size(); i++) {

		bool is_duplicate = false;

		for(unsigned int j = 0; j < radii.size(); j++) {

			if(radii[i] == radii[j] && angles[i] == angles[j] && i != j) {
				is_duplicate = true;
			}
		}

		if(not(is_duplicate)) {
			unique_radii.push_back(radii[i]);
			unique_angles.push_back(angles[i]);
		}
	}
	radii = unique_radii;
	angles = unique_angles;
}

//rounds radius values and places all angles in the 0-360 degree range
void roundPoints(std::vector<float>& radii, std::vector<float>& angles) {
	for(unsigned int i = 0; i < radii.size(); i++) {
		radii[i] = roundf(radii[i]*100) / 100;
	}

	for(unsigned int j = 0; j < angles.size(); j++) {
		angles[j] = remainder(angles[j], 360.0);
		if(angles[j] < 0) {
			angles[j] = 360.0 + angles[j];
		}
	}
}

//outputs radii and angles, separated by a line break
//this format is easiest to copy into the Arduino IDE as an array
void writeToOutput(std::ofstream& output, const std::vector<float>& radii, const std::vector<float>& angles) {
	assert(radii.size() == angles.size());
	
	for(unsigned int i = 0; i < radii.size(); i++) {
		output << std::setprecision(2) << std::fixed << fabs((radii[i]/10));
		if(i != radii.size() - 1) {
			output << ',';
		}
	}

	output << std::endl;

	for(unsigned int j = 0; j < angles.size(); j++) {
		output << std::setprecision(2) << std::fixed << angles[j];
		if(j != angles.size() - 1) {
			output << ',';
		}
	}
}


//removes all points further out than some user-specified radius
void cutToSize(std::vector<float>& radii, std::vector<float>& angles, const float max_radius) {
	std::vector<float> valid_radii;
	std::vector<float> valid_angles;

	for(unsigned int i = 0; i < radii.size(); i++) {
		if(not(radii[i] > max_radius) && radii[i] != 0) {
			valid_radii.push_back(radii[i]);
			valid_angles.push_back(angles[i]);
		}
	}

	radii = valid_radii;
	angles = valid_angles;
}

//reads csv into two vectors, radii and angles
void readFile(std::ifstream& input, std::vector<float>& radii, std::vector<float>& angles) {
	
	bool is_radius = true; //boolean flag to switch between radius and angle vectors
	char c;
	
	while(input.get(c)) {
		std::string val = "";
		while(c != ',' && c != '\n' && input.good()) { //read variable-length value from file
			val += c;
			input.get(c);
		}

		if (val != "") {

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
	float max_radius = strtof(argv[3], NULL);
	
	input_file.open(input_file_name.c_str());
	output_file.open(output_file_name.c_str());
	
	readFile(input_file, radii, angles);
	roundPoints(radii, angles);
	removeDuplicates(radii, angles);
	cutToSize(radii, angles, max_radius);
	writeToOutput(output_file, radii, angles);
	std::cout << radii.size() << " points in the pattern." << std::endl;
};