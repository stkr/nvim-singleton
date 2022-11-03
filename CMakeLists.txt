cmake_minimum_required(VERSION 3.22)

# set the project name
project(nvim-singleton)

# add the executable
add_executable(nvim-singleton src/main.cpp)
target_compile_features(nvim-singleton PRIVATE cxx_std_17)
