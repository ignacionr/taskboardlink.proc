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

#define cimg_use_jpeg
#include "./CImg.h"

using namespace cimg_library;
using namespace std;

#include "./ImageFeatures.hpp"

class ImageWithFeatures {
	CImg<unsigned char> _img;
	ImageFeatures _features;
public:
	ImageWithFeatures(const string& path) {
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
	cout << "   Usage: stitch <path> <out.jpg>" << endl;
}

int main(int argc, char * argv[]) {
	if (argc < 3)
	{
		help();
		return 1;
	}
	try {
		auto max_y = 0;
		auto max_x = 0;

		struct dirent **file_entries;
		map<pair<int,int>,ImageWithFeatures> grid;
		auto count = scandir(argv[1], &file_entries, NULL, NULL);
		if (-1 == count) {
			cout << "Can't enumerate " << argv[1] << " : " << errno << endl;
			return 1;
		}
		
		regex rxFileNames(".*_(\\d+)_(\\d+).jpg");
		smatch match;
		
		for(int idx = 0; idx < count; idx++) {
			string fname(file_entries[idx]->d_name);
			if (regex_match(fname, match, rxFileNames)) {
				int y = atol(match[1].str().c_str());
				int x = atol(match[2].str().c_str());
				cout << "Reading " << y << ", " << x << endl;
				grid[make_pair(x,y)] = ImageWithFeatures(string(argv[1]) + "/" + fname);
				max_x = max(x, max_x);
				max_y = max(y, max_y);
			}
		}
		free(file_entries);
		
		auto widthCount = max_x + 1;
		auto heightCount = max_y + 1;
		
		auto oneimg = (*grid.cbegin()).second;
		
		auto onewidth = oneimg.image().width();
		auto oneheight = oneimg.image().height();
		
		auto totalWidth = widthCount * onewidth;
		auto canvasHeight = oneheight;

		char fname[250];
		for(auto y = 0; y < heightCount; y++) {

			cout << "Creating a canvas " << totalWidth/3 << " x " << canvasHeight << endl;
			
			CImg<unsigned char> canvas(totalWidth/3,canvasHeight,1,3, 255);
			int x_correction = 0;
			int y_correction = 0;
			for (auto x = 0; x < widthCount; x++) {
				auto this_image_features = grid[make_pair(x,y)];
				if (x > 0) {
					auto imgf = grid[make_pair(x-1,y)];
					auto features_left = imgf.features();
					if (features_left.size()) {
						auto suggestion = this_image_features.features().suggest_correction(features_left);
						if (suggestion.valid()) {
							y_correction += suggestion.y_correction;
							x_correction += suggestion.x_correction;
						}
						else {
							x_correction += (x_correction / x); // use the average
						}
					}
				}
				CImg<unsigned char> &img = this_image_features.image();
				if (img.size()) {
					cout << '.';
					canvas.draw_image(x_correction, y_correction, img, 0.8f);
				}
			}
			cout << endl;
			
			sprintf(fname, "row_%03d.jpg", y);
			canvas.save_jpeg(fname);
			cout<< fname << " saved." << endl;		
		}
	}
	catch(CImgException& ex) {
		cout << "ERROR: " << ex.what() << endl;
	}
	return 0;
}
