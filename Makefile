compile: stitch

stitch: stitch.cpp
	g++ stitch.cpp -o stitch -Wall -Wextra -std=c++11 -pedantic -Dcimg_display=0 -O2 -mtune=generic -lm
