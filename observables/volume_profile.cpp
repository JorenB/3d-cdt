// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#include "volume_profile.hpp"

void VolumeProfile::process() {
	std::string tmp = "";
	for (auto l : Universe::sliceSizes) {
		tmp += std::to_string(l);
		tmp += " ";
	}
	tmp.pop_back();

	output = tmp;
}
