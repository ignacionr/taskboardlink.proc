compile: stitch

test: stitch
	./stitch ~/Downloads/imgs3/pic_000_023.jpg ~/Downloads/imgs3/pic_000_024.jpg pic_000_23-24.jpg

stitch: stitch.cpp ImageFeatures.hpp
	g++ stitch.cpp -o stitch -Wall -Wextra -std=c++11 -pedantic -Dcimg_display=0 -O2 -mtune=generic -lm -Dcimg_use_jpeg -ljpeg

