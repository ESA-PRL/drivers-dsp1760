rock_library(dsp1760
    SOURCES dsp1760.cpp
    HEADERS dsp1760.hpp
	DEPS_PKGCONFIG iodrivers_base)

rock_executable(dsp1760_bin main.cpp
    DEPS dsp1760
    DEPS_PKGCONFIG iodrivers_base)

