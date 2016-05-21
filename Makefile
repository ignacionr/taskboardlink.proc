compile: stitch

test: stitch
	./stitch ~/Downloads/imgs3
	
test-one: stitch
	./stitch -one ./st_004_023-024.jpg ./st_004_025-026.jpg ./st_004_023-026.jpg

stitch: stitch.cpp ImageFeatures.hpp
	g++ stitch.cpp -o stitch -Wall -Wextra -std=c++11 -pedantic -Dcimg_display=0 -O2 -mtune=generic -lm -Dcimg_use_jpeg -ljpeg

