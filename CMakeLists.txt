cmake_minimum_required(VERSION 3.22)

# set the project name
project(nvim-singleton)

# add the executable
add_executable(nvim-singleton src/main.cpp)
target_compile_features(nvim-singleton PRIVATE cxx_std_17)

# In order to build as Win32 GUI application (without console), the following
# property can be used (see also cmake manual):
set_target_properties(nvim-singleton PROPERTIES WIN32_EXECUTABLE TRUE)
