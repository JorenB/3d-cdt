// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#pragma once

#include <string>
#include <vector>
#include "../observable.hpp"
#include "../universe.hpp"

class Minbu : public Observable {
    public:
		using Observable::Observable;
        Minbu(std::string id) : Observable(id) { name = "minbu"; }

        void process();

	private:
		void initialize() { }
};
