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
	ImageWithFeatures(const string& path, bool is_left_image) {
		cout << "reading " << path;
		_img = CImg<unsigned char>(path.c_str());
		cout << " " << _img.width() << "x" << _img.height() << endl;
		_features = ImageFeatures(_img, is_left_image);
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

void stitch_images(string left_src, string right_src, string output) {
	ImageWithFeatures left(left_src.c_str(), true);
	ImageWithFeatures right(right_src.c_str(), false);

	auto suggestion = right.features().suggest_correction(left.features());
	if (!suggestion.valid()) {
		throw runtime_error("Images seem unrelated.");
	}

	auto canvasHeight = left.image().height() + abs(suggestion.y_correction);
	auto canvasWidth = right.image().width() + suggestion.x_correction;
				
	cout << "Creating a canvas " << canvasWidth << " x " << canvasHeight << endl;
	CImg<unsigned char> canvas(canvasWidth,canvasHeight,1,3, 255);
	
	auto y = suggestion.y_correction > 0 ? 0 : abs(suggestion.y_correction);
	// paint the first image
	canvas.draw_image(0, y, left.image(), 0.85f);
	canvas.draw_image(suggestion.x_correction, y + suggestion.y_correction, right.image(), 0.85f);
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

void stitch_pairs_by_row(const string &path) {
	int max_col = 1;
	for(int row = 0; max_col > 0; row++) {
		bool succeeded = true;
		max_col = 0;
		for (int col = 1; succeeded; col += 2) {
			try {
				stitch_images(file_name(path, row, col), file_name(path, row, col+1),
					output_name(row, col, col+1));
				max_col = col;
			}
			catch(exception &) {
				succeeded = false;
			}
		}
	}
}

int main(int argc, char * argv[]) {
	if (argc < 2)
	{
		help();
		return 1;
	}
	try {
		if (0 == strcmp(argv[1], "-one")) {
			stitch_images(argv[2], argv[3], argv[4]);
		} else 
		{
			stitch_pairs_by_row(argv[1]);
		}
	}
	catch(exception& ex) {
		cout << "ERROR: " << ex.what() << endl;
	}
	return 0;
}
