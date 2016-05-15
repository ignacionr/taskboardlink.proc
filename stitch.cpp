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

string file_name(const string &path, int y, int x) {
	char buff[250];
	sprintf(buff, "%s/pic_%03d_%03d.jpg", path.c_str(), y, x);
	return string(buff);
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
				max_x = max(x, max_x);
				max_y = max(y, max_y);
			}
		}
		free(file_entries);
		
		auto widthCount = max_x + 1;
		auto heightCount = max_y + 1;
		
		
		for(auto y = 0; y < heightCount; y++) {
			list<pair<int,ImageFeatures::suggestion>> suggestions;	
			auto x = 0;
			ImageWithFeatures initial;
			for(auto got_first = false; !got_first; x++) {
				try {
					initial = ImageWithFeatures(file_name(argv[1], y, x));
					got_first = true;
				}
				catch(CImgException &ex) {
					cout << ex.what();
				}
			}

			auto oneheight = initial.image().height();
			auto canvasHeight = oneheight;

			auto left = initial;			
			for (x++; x < widthCount; x++) {
				try {
					ImageWithFeatures right(file_name(argv[1], y, x));
					auto features_left = initial.features();
					suggestions.push_back(make_pair(x,right.features().suggest_correction(features_left)));
					CImg<unsigned char> &img = right.image();
					if (img.size()) {
						cout << '.';
					}
					left = right;
				}
				catch(CImgException &ex) {
					cout << ex.what();
				}
			}
			cout << endl;
			
			// now actually create the row image
			// the width will be the sum of X corrections plus the width of the last image
			auto canvasWidth = 0;
			auto last = 0;
			for(auto suggestion : suggestions) {
				canvasWidth += suggestion.second.x_correction;
				last = suggestion.first;
			}
			CImg<unsigned char> lastImage (file_name(argv[1], y, last).c_str());
			canvasWidth += lastImage.width();
			cout << "Creating a canvas " << canvasWidth << " x " << canvasHeight << endl;
			CImg<unsigned char> canvas(canvasWidth,canvasHeight,1,3, 255);
			// paint the first image
			int x_correction = 0;
			int y_correction = 0;
			canvas.draw_image(x_correction, y_correction, initial.image());
			auto drawn = 1;
			for(auto suggestion: suggestions) {
				if (suggestion.second.valid()) {
					y_correction += suggestion.second.y_correction;
					x_correction += suggestion.second.x_correction;
				}
				else {
					x_correction += (x_correction / drawn); // use the average
				}
				CImg<unsigned char> img(file_name(argv[1], y, suggestion.first).c_str());
				canvas.draw_image(x_correction, y_correction, img, 0.85f);
			}
			char fname[30];
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
