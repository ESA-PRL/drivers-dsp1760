rock_library(dsp1760
    SOURCES Dummy.cpp
    HEADERS Dummy.hpp)

rock_executable(dsp1760_bin Main.cpp
    DEPS dsp1760)

