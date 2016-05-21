#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <functional>
#include <map>
#include <algorithm>
#include <sys/types.h>
#include <dirent.h>
#include <regex>
#include <sstream>
#include <stdexcept>

#include "./CImg.h"

using namespace cimg_library;
using namespace std;

#include "./ImageFeatures.hpp"

class ImageWithFeatures {
	CImg<unsigned char> _img;
	ImageFeatures _features;
public:
	ImageWithFeatures(const string& path, bool is_left_image, bool x_ray) {
		cout << "reading " << path;
		_img = CImg<unsigned char>(path.c_str());
		cout << " " << _img.width() << "x" << _img.height() << endl;
		_features = ImageFeatures(_img, is_left_image, x_ray);
	}
	ImageWithFeatures(const ImageWithFeatures& src) {
		_img = src._img;
		_features = src._features;
	}
	ImageWithFeatures() {}
	CImg<unsigned char> &image() { return _img; }
	ImageFeatures &features() { return _features; }
};

void help() {
	cout << "   Usage: " << endl;
	cout << "		stitch <source-path>" << endl;
	cout << "		stitch -one <left.jpg> <right.jpg> <output.jpg>" << endl;
}

void stitch_images(string left_src, string right_src, string output, bool x_ray) {
	ImageWithFeatures left(left_src.c_str(), true, x_ray);
	ImageWithFeatures right(right_src.c_str(), false, x_ray);

	auto suggestion = right.features().suggest_correction(left.features(), left.image().width());
	if (!suggestion.valid()) {
		throw runtime_error("Images seem unrelated.");
	}

	auto canvasHeight = left.image().height() + abs(suggestion.y_correction);
	auto canvasWidth = right.image().width() + suggestion.x_correction;
				
	cout << "Creating a canvas " << canvasWidth << " x " << canvasHeight << endl;
	CImg<unsigned char> canvas(canvasWidth,canvasHeight,1,3, 255);
	
	auto y = suggestion.y_correction > 0 ? 0 : abs(suggestion.y_correction);

	if (x_ray) {
		canvas.draw_image(0, y, left.image(), 0.55f);
		canvas.draw_image(suggestion.x_correction, y + suggestion.y_correction, right.image(), 0.55f);
	}
	else {
		canvas.draw_image(0, y, left.image());
		canvas.draw_image(suggestion.x_correction, y + suggestion.y_correction, right.image());
	}
	canvas.save_jpeg(output.c_str());
	cout<< output << " saved." << endl;
}

string file_name(string path, int row, int col) {
	char buff[100];
	sprintf(buff, "pic_%03d_%03d.jpg", row, col);
	return path + "/" + buff;
}

string output_name(int row, int col1, int col2) {
	char buff[100];
	sprintf(buff, "st_%03d_%03d-%03d.jpg", row, col1, col2);
	return buff;
}

void stitch_pairs_by_row(const string &path, bool x_ray) {
	struct dirent **name_list;
	int n = scandir(path.c_str(), &name_list, NULL, alphasort);
	if (n < 0) {
		throw runtime_error("can't scan directory");
	}
	
	regex input_rex("([^_]+)_(\\d+)_(\\d+)(-(\\d+))?\\.jpg");
	smatch input_match;
	
	int current_row = -1;
	int next_col = -1;
	int left_col = -1;
	string left_file;
	string sep("/");
	for(int entry_index = 0; entry_index < n; entry_index++) {
		auto &entry = name_list[entry_index];
		string file_name(entry->d_name);
		cout << "analizing " << file_name << endl;
		if (regex_match(file_name, input_match, input_rex)) {
			int row = atol(input_match[2].str().c_str());
			int col = atol(input_match[3].str().c_str());
			int rightmost = input_match[5].length() ? atol(input_match[5].str().c_str()) : col;
			if (row == current_row && col == next_col) {
				cout << " good for right image" << endl;
				// this is the right part to our left
				// so, stitch both into a third and remove them afterwards
				char output_name[50];
				sprintf(output_name, "img_%03d_%03d-%03d.jpg", current_row, left_col, rightmost);
				try {
					stitch_images(
						path + sep + left_file, 
						path + sep + file_name, 
						path + sep + output_name, x_ray);
					remove((path + sep + left_file).c_str());
					remove((path + sep + file_name).c_str());
				}
				catch(exception &ex) {
					cout << "Discontinuity: " << ex.what();
				}
				current_row = -1;
			}
			else {
				cout << " using row " << row << endl;
				// we'll use this as the left image
				current_row = row;
				left_col = col;
				next_col = rightmost + 1;
				left_file = file_name;
			}
		}
		else {
			cout << "not a match" << endl;
		}
		free(entry);
	}
	
	free (name_list);
}

int main(int argc, char * argv[]) {
	if (argc < 2)
	{
		help();
		return 1;
	}
	bool x_ray = 
		(argc > 2 && 0 == strcmp(argv[2], "-xray")) ||
		(argc > 4 && 0 == strcmp(argv[4], "-xray"));
	try {
		if (0 == strcmp(argv[1], "-one")) {
			stitch_images(argv[2], argv[3], argv[4], x_ray);
		} else 
		{
			stitch_pairs_by_row(argv[1], x_ray);
		}
	}
	catch(exception& ex) {
		cout << "ERROR: " << ex.what() << endl;
	}
	return 0;
}
