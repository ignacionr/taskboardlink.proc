
class ImageFeatures: public vector<pair<int,int>> {
public:
	ImageFeatures() { }
	ImageFeatures(CImg<unsigned char> &img, int feature_x_margin = 10) {
		// obtain the extreme values
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
		// cout << min_brightness << ".." << max_brightness << " (" << max_contrast << ")" << endl;
		auto threshold = max_contrast / 15;
		if (threshold > 10) {
			for(auto y= 0; img.height() > y; y++) {
				auto left_brightness = img(feature_x_margin,y,0) + img(feature_x_margin,y,1) + img(feature_x_margin,y,2); 
				for(auto x = feature_x_margin+1; x < img.width()-feature_x_margin; x++) {
					auto right_brightness = img(x,y,0) + img(x,y,1) + img(x,y,2);
					auto contrast = abs(right_brightness- left_brightness);
					if (contrast > threshold) {
						push_back(make_pair(x,y));
						// cout << "found feature: @" << x << "," << y << endl;
					}
					left_brightness = right_brightness;
				}
			}
			for(auto x = feature_x_margin; x < img.width()-feature_x_margin; x++) {
				auto up_brightness = img(x,0,0) + img(x,0,1) + img(x,0,2); 
				for(auto y= 1; img.height() > y; y++) {
					auto down_brightness = img(x,y,0) + img(x,y,1) + img(x,y,2);
					auto contrast = abs(up_brightness - down_brightness);
					if (contrast > threshold) {
						push_back(make_pair(x,y));
						// cout << "found feature: @" << x << "," << y << endl;
					}
					up_brightness = down_brightness;
				}
			}
		}
		else {
			cout << "WARNING: this image can't be used (contrast is too low)" << endl;
		}

		// unsigned char featureColor[] = {255,0,0};
		// for(auto feat: *this) {
		// 	img.draw_circle(feat.first, feat.second, 5, (unsigned char*)&featureColor);
		// }
		// cout << "Total features: " << size() << endl;
	}
	// suggests a small correction on y (.first) and a bigger one on x (.second)
	struct suggestion {
		int	y_correction;
		int x_correction;
		int	votes = 0;
		bool	valid() {return votes > 6;}
	};
	
	suggestion suggest_correction(const ImageFeatures &other) {
		suggestion best;
		for (auto y_correction = -18; y_correction <= 18; y_correction++) {
			auto current_result = suggest_correction(other, y_correction);
			if (current_result.second > best.votes) {
				best.votes = current_result.second;
				best.y_correction = y_correction;
				best.x_correction = current_result.first;
			}
		}
		// cout << "suggesting a y correction of " << best.y_correction << endl;
		return best;
	}
	
	pair<int,int> suggest_correction(const ImageFeatures &other, int moving_y) const {
		multimap<int,int> features_by_y;
		for(auto feature: *this) {
			features_by_y.insert(make_pair(feature.second + moving_y, feature.first));
		}
		map<int,int> correction_votes;
		for(auto feature: other) {
			auto this_range = features_by_y.equal_range(feature.second);
			for (auto it = this_range.first; it != this_range.second; it++) {
				auto distance = feature.first - (*it).second;
				correction_votes[distance]++;
			}
		}
		auto pmax_vote = max_element(correction_votes.begin(), correction_votes.end(), [] (const pair<int,int>&e1, const pair<int,int>&e2) {
			return e1.second < e2.second;
		});
		// cout << "Distance voted: " << (*pmax_vote).first << " gains " << (*pmax_vote).second << endl;
		return *pmax_vote;
	}
};

