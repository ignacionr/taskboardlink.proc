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

			auto onewidth = initial.image().width();
			auto oneheight = initial.image().height();
			
			auto totalWidth = widthCount * onewidth;
			auto canvasHeight = oneheight;
			cout << "Creating a canvas " << totalWidth/3 << " x " << canvasHeight << endl;
			
			CImg<unsigned char> canvas(totalWidth/5,canvasHeight,1,3, 255);
			int x_correction = 0;
			int y_correction = 0;

			// paint the first image
			canvas.draw_image(x_correction, y_correction, initial.image());
			auto drawn = 1;
			
			for (x++; x < widthCount; x++) {
				try {
					ImageWithFeatures current(file_name(argv[1], y, x));
					if (current.features().size() > 30) {
						auto features_left = initial.features();
						if (features_left.size()) {
							auto suggestion = current.features().suggest_correction(features_left);
							if (suggestion.valid()) {
								y_correction += suggestion.y_correction;
								x_correction += suggestion.x_correction;
							}
							else {
								x_correction += (x_correction / drawn); // use the average
							}
						}
						else {
							x_correction += (x_correction / x); // use the average
						}
						CImg<unsigned char> &img = current.image();
						if (img.size()) {
							cout << '.';
							canvas.draw_image(x_correction, y_correction, img, 0.8f);
							drawn++;
						}
					}
					else {
						x_correction += (x_correction / drawn); // use the average
						cout << "WARNING: this image has too few features" << endl;
					}
					initial = current;
				}
				catch(CImgException &ex) {
					cout << ex.what();
				}
			}
			cout << endl;
			
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
