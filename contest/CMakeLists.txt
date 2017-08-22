link_directories(${LIBRARY_OUTPUT_PATH})

find_package(OpenCV)

#for UAV competition 2017 created by Danping ##
include_directories( ${OpenCV_INCLUDE_DIRS})
add_executable(uav_artag_parser 
               artag_parser/artag_parser_main.cpp)
target_link_libraries(uav_artag_parser 
                      ARToolKitPlus
                      ${OpenCV_LIBRARIES})
