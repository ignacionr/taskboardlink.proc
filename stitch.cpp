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
	ImageWithFeatures(const string& path) {
		cout << "reading " << path << endl;
		_img = CImg<unsigned char>(path.c_str());
		_features = ImageFeatures(_img);
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
	cout << "   Usage: stitch <img1.jpg> <img2.jpg> <out.jpg>" << endl;
}

void stitch_images(string left_src, string right_src, string output) {
	ImageWithFeatures left(left_src.c_str());
	ImageWithFeatures right(right_src.c_str());

	auto suggestion = right.features().suggest_correction(left.features());
	if (!suggestion.valid()) {
		throw runtime_error("Images seem unrelated.");
	}

	auto canvasHeight = left.image().height() + abs(suggestion.y_correction);
	auto canvasWidth = left.image().width() + right.image().width() + suggestion.x_correction;
				
	cout << "Creating a canvas " << canvasWidth << " x " << canvasHeight << endl;
	CImg<unsigned char> canvas(canvasWidth,canvasHeight,1,3, 255);
	
	auto y = suggestion.y_correction > 0 ? 0 : abs(suggestion.y_correction);
	// paint the first image
	canvas.draw_image(0, y, left.image());
	canvas.draw_image(suggestion.x_correction, y + suggestion.y_correction, right.image(), 0.85f);
	canvas.save_jpeg(output.c_str());
	cout<< output << " saved." << endl;
}

int main(int argc, char * argv[]) {
	if (argc < 4)
	{
		help();
		return 1;
	}
	try {
		stitch_images(argv[1], argv[2], argv[3]);
	}
	catch(exception& ex) {
		cout << "ERROR: " << ex.what() << endl;
	}
	return 0;
}
