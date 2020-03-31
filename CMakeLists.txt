# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.10)

project ("LambdaDetector")

# Include sub-projects.
add_subdirectory ("LambdaDetector")

target_compile_features("LambdaDetector" PRIVATE cxx_std_20)
