TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

#Отключить "теневую сборку" в криейторе!
CONFIG(release, debug|release) {

message(Project $$TARGET (Release))

OBJECTS_DIR = build/release
MOC_DIR = build/release
RCC_DIR = build/release
UI_DIR = build/release
}
CONFIG(debug, debug|release) {

message(Project $$TARGET (Debug))

OBJECTS_DIR = build/debug
MOC_DIR = build/debug
RCC_DIR = build/debug
UI_DIR = build/debug
DEFINES += DEBUG_BUILD
}

INCLUDEPATH += include/
# Input
HEADERS += include/forward_player.hpp \
           include/reverse_player.hpp \
           include/road_layout.hpp \
           include/road_layout_marker.hpp \
           include/road_layout_storage.hpp \
           include/road_scene.hpp \
           include/road_scene_marker.hpp \
           include/road_scene_storage.hpp \
           include/segmentation.hpp \
           include/sign.hpp \
           include/sign_marker.hpp \
           include/sign_storage.hpp \
           include/util.hpp \
           include/tinyxml2.h

SOURCES += src/forward_player.cpp \
           src/main.cpp \
           src/reverse_player.cpp \
           src/road_layout.cpp \
           src/road_layout_marker.cpp \
           src/road_layout_storage.cpp \
           src/road_scene.cpp \
           src/road_scene_marker.cpp \
           src/road_scene_storage.cpp \
           src/scene_export.cpp \
           src/segmentation.cpp \
           src/sign.cpp \
           src/sign_exporter.cpp \
           src/sign_marker.cpp \
           src/sign_storage.cpp \
           src/util.cpp \
           src/tinyxml2.cpp

INCLUDEPATH += /usr/local/include
LIBS += -L/usr/local/lib \
        -lopencv_core \
        -lopencv_imgproc \
        -lopencv_highgui \
        -lopencv_video \
        -lopencv_videoio \
        -lopencv_imgcodecs \
        -lopencv_tracking \
        -lopencv_dnn

# Boost and common
LIBS += -L/usr/lib/x86_64-linux-gnu \
        -lboost_filesystem \
        -lboost_system \
        -lboost_program_options \
        -lboost_log \
        -lboost_log_setup \
        -lboost_thread \
        -lboost_regex \
        -lboost_unit_test_framework \
        -lpthread \
        -ldl -fPIC
