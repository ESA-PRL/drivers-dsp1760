cmake_minimum_required(VERSION 2.6)
find_package(Rock)

rock_init(dsp1760 0.1)

rock_library(dsp1760 src/dsp1760.cpp
	HEADERS src/dsp1760.hpp
	DEPS_PKGCONFIG iodrivers_base)
	
rock_executable(dsp1760_test src/main.cpp
	DEPS dsp1760)
