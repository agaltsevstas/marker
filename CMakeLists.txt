# Название проекта
project(marker)

# Проверка CMake
cmake_minimum_required(VERSION "${MIN_VER_CMAKE}" FATAL_ERROR)

############### Идентификация платформы ################
set(PLATFORM 0)
if(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL x86_64)
  set(PLATFORM 1)
  add_definitions(-DPLATFORM=1)
  message("PLATFORM: x86_64")
  set(COMMON_LIBS_DIR /usr/lib/x86_64-linux-gnu)
elseif(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL aarch64)
  set(PLATFORM 2)
  add_definitions(-DPLATFORM=2)
  message("PLATFORM: aarch64")
  set(COMMON_LIBS_DIR /usr/lib/aarch64-linux-gnu)
else()
  message("UNKNOWN PLATFORM: " ${CMAKE_HOST_SYSTEM_PROCESSOR})
  return()
endif()

# Поиск библиотек OpenCV
set(OpenCV_FOUND 1)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

############# Конфигурация CMake ##############
# Выбор уровня предупреждения
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unknown-pragmas")

############### Конфигурация компилятора ###############
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

################# Выходная конфигурация ################
# Путь для выходного файла
if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/)
else()
  set(CMAKE_BUILD_TYPE Release)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/)
endif()

# Qt et common
if(${PLATFORM} EQUAL 1)
  set(COMMON_LIBS_DIR /usr/lib/x86_64-linux-gnu)
else()
  set(COMMON_LIBS_DIR /usr/lib/aarch64-linux-gnu)
endif()

# Хедеры
include_directories(${PROJECT_SOURCE_DIR}/include/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# Исходники
file(GLOB SRC_LIST ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB INCLUDE_LIST ${PROJECT_SOURCE_DIR}/include/*.h)

################### Определение цели ###################
# Создание исполняемого приложения
add_executable(marker ${INCLUDE_LIST} ${SRC_LIST} ${FORMS_LIST})

################ Подключаемые библиотеки ###############
# Общие библиотеки
link_directories(${COMMON_LIBS_DIR})

# OpenCV
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

# Boost
target_link_libraries(${PROJECT_NAME} libboost_regex.so)
target_link_libraries(${PROJECT_NAME} libboost_system.so)
target_link_libraries(${PROJECT_NAME} libboost_filesystem.so)
target_link_libraries(${PROJECT_NAME} libboost_log.so)	    
target_link_libraries(${PROJECT_NAME} libboost_log_setup.so)	
target_link_libraries(${PROJECT_NAME} libboost_thread.so)
target_link_libraries(${PROJECT_NAME} libpthread.so)         
target_link_libraries(${PROJECT_NAME} libdl.so)	              
target_link_libraries(${PROJECT_NAME} libX11.so)	      
