#include "./CImg.h"
#include <iostream>

using namespace cimg_library;

void help() {
	std::cout << "   Usage: stitch img1 [.. imgn] out" << std::endl;
}

int main(int argc, char * argv[]) {
	if (argc < 2)
	{
		help();
		return 1;
	}
	try {
		CImg<unsigned char> img(argv[1]);
		CImg<unsigned char> second(img.width()*2, img.height(), 1, 3, 0);
		second
			.draw_image(0,0, img)
			.draw_image(img.width(),0, img);
			
		second.save_jpeg(argv[2]);
	}
	catch(CImgException& ex) {
		std::cout << "ERROR: " << ex.what() << std::endl;
	}
	return 0;
}
