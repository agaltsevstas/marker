cmake_minimum_required(VERSION 2.8.11)

project(marker)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -fopenmp")

find_package(OpenCV REQUIRED)

find_package(Boost COMPONENTS system filesystem REQUIRED)

find_package(Caffe QUIET)

if(Caffe_INCLUDE_DIRS AND Caffe_LIBRARIES)
    include_directories(${Caffe_INCLUDE_DIRS})
    add_definitions(-DDEEPLABV2_FOUND)
else(Caffe_INCLUDE_DIRS AND Caffe_LIBRARIES)
    message(WARNING "deeplabv2 not found. road scene marker is disabled")
endif(Caffe_INCLUDE_DIRS AND Caffe_LIBRARIES)

include_directories(${OpenCV_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
include_directories(marker_/src marker_qt/include marker_qt)

add_executable(marker marker_qt/src/main.cpp

               marker_qt/src/util.cpp

               marker_qt/src/sign_marker.cpp
               marker_qt/src/sign_storage.cpp
               marker_qt/src/reverse_player.cpp
               marker_qt/src/sign.cpp
               marker_qt/src/sign_exporter.cpp

               marker_qt/src/road_layout_marker.cpp
               marker_qt/src/forward_player.cpp
               marker_qt/src/road_layout.cpp
               marker_qt/src/road_layout_storage.cpp

               marker_qt/src/road_scene_marker.cpp
               marker_qt/src/segmentation.cpp
               marker_qt/src/road_scene.cpp
               marker_qt/src/road_scene_storage.cpp
               marker_qt/src/scene_export.cpp)

if(Caffe_INCLUDE_DIRS AND Caffe_LIBRARIES)
    target_link_libraries(marker ${Caffe_LIBRARIES})
endif(Caffe_INCLUDE_DIRS AND Caffe_LIBRARIES)

target_link_libraries(marker ${OpenCV_LIBS} ${Boost_LIBRARIES})

option(BUILD_DOC "Build documentation" OFF)

if(BUILD_DOC)

    set(DOXYGEN_SKIP_DOT FALSE)
    find_package(Doxygen REQUIRED)

    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    add_custom_target(doc_doxygen ALL COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen")

endif(BUILD_DOC)
