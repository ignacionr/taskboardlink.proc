compile: stitch

test: test.done

test.done: stitch
	rm -Rf test* && mkdir test && cp ~/Downloads/imgs3/*.jpg ./test && ./stitch test/. && touch test.done
	
test-one: stitch
	./stitch -one ./st_004_023-024.jpg ./st_004_025-026.jpg ./st_004_023-026.jpg

stitch: stitch.cpp ImageFeatures.hpp
	g++ stitch.cpp -o stitch -Wall -Wextra -std=c++11 -pedantic -Dcimg_display=0 -O2 -mtune=generic -lm -Dcimg_use_jpeg -ljpeg

