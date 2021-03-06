
class ImageFeatures: public vector<pair<int,int>> {
	
	#define MAX_FEATURES 		1000
	#define MIN_THRESHOLD 		5
	#define	MARGIN_X			7
	#define MIN_DISTANCE		MARGIN_X
	#define MAX_DISTANCE		500
	#define MAX_Y_CORRECTION	30
public:
	ImageFeatures() { }
	ImageFeatures(CImg<unsigned char> &img, bool is_left_image, bool x_ray) {
		auto from_x = MARGIN_X;
		auto to_x = img.width() - MARGIN_X;

		int max_brightness = 0, min_brightness = 255;
		for(auto pixel_it = img.begin(); pixel_it < img.end(); pixel_it++) {
			int brightness = *pixel_it;
			pixel_it++;
			brightness += *pixel_it;
			pixel_it++;
			brightness += *pixel_it;

			min_brightness = min(brightness, min_brightness);
			max_brightness = max(brightness, max_brightness);
		}
		
		auto max_contrast = max_brightness - min_brightness;
		cout << "max_contrast is " << max_contrast << endl;
		
		int threshold = 0;
		for(auto factor = max_contrast / MIN_THRESHOLD; empty() && (factor < 500) && (factor > 1); factor -= 2) {
			auto go_on = true;
			threshold = max_contrast / factor;
			for(auto y= 0; (img.height() > y) && go_on; y++) {
				auto left_brightness = img(from_x,y,0) + img(from_x,y,1) + img(from_x,y,2); 
				for(auto x = from_x+1; (x <= to_x) && go_on; x++) {
					auto right_brightness = img(x,y,0) + img(x,y,1) + img(x,y,2);
					auto contrast = abs(right_brightness- left_brightness);
					if (contrast > threshold) {
						push_back(make_pair(x,y));
						if (size() > MAX_FEATURES) {
							clear();
							go_on = false;
						}
					}
					left_brightness = right_brightness;
				}
			}
			for(auto x = from_x; (x <= to_x) && go_on; x++) {
				auto up_brightness = img(x,0,0) + img(x,0,1) + img(x,0,2); 
				for(auto y= 1; (img.height() > y) && go_on; y++) {
					auto down_brightness = img(x,y,0) + img(x,y,1) + img(x,y,2);
					auto contrast = abs(up_brightness - down_brightness);
					if (contrast > threshold) {
						push_back(make_pair(x,y));
						if (size() > MAX_FEATURES) {
							clear();
							go_on = false;
						}
					}
					up_brightness = down_brightness;
				}
			}
		}

		if (x_ray) {
			unsigned char featureColor[] = {255,0,0};
			if (!is_left_image)
			{
				featureColor[0] = 0;
				featureColor[1] = 255;
			}
			for(auto feat: *this) {
				img.draw_circle(feat.first, feat.second, 2, (unsigned char*)&featureColor);
			}
		}
		cout << "Total features: " << size() << " (threshold " << threshold << ")" << endl;
	}
	// suggests a small correction on y (.first) and a bigger one on x (.second)
	struct suggestion {
		int	y_correction;
		int x_correction;
		int	votes = 0;
		bool	valid() {return votes > 6;}
	};
	
	suggestion suggest_correction(const ImageFeatures &other, int left_width) {
		suggestion best;
		for (auto y_correction = -MAX_Y_CORRECTION; y_correction <= MAX_Y_CORRECTION; y_correction++) {
			auto current_result = suggest_correction(other, y_correction, left_width);
			if (current_result.second > best.votes) {
				best.votes = current_result.second;
				best.y_correction = y_correction;
				best.x_correction = current_result.first;
			}
		}
		cout << "suggesting an x correction of " << best.x_correction << endl;
		cout << "suggesting a y correction of " << best.y_correction << endl;
		return best;
	}
	
	pair<int,int> suggest_correction(const ImageFeatures &other, int moving_y, int left_width) const {
		multimap<int,int> features_by_y;
		for(auto feature: *this) {
			features_by_y.insert(make_pair(feature.second + moving_y, feature.first));
		}
		map<int,int> correction_votes;
		for(auto feature: other) {
			auto this_range = features_by_y.equal_range(feature.second);
			for (auto it = this_range.first; it != this_range.second; it++) {
				auto x_correction = feature.first - (*it).second;
				// auto distance = abs(left_width - abs(x_correction));
				// if ((distance >= MIN_DISTANCE) && (distance <= MAX_DISTANCE)) {
					correction_votes[x_correction]++;
				// }
			}
		}
		auto pmax_vote = max_element(correction_votes.begin(), correction_votes.end(), [] (const pair<int,int>&e1, const pair<int,int>&e2) {
			return e1.second < e2.second;
		});
		// cout << "Distance voted: " << (*pmax_vote).first << " gains " << (*pmax_vote).second << endl;
		return *pmax_vote;
	}
};

