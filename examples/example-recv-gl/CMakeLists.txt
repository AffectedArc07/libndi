cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

project(example-recv-gl)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
set(LINUX 1)
endif ()

if (LINUX)
	include_directories(/opt/vc/include/)
	link_directories(/opt/vc/lib/)
	link_libraries(GLESv2 EGL avahi-client avahi-common)
endif()

if (NOT LINUX)
# opengl
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
link_libraries(${OPENGL_LIBRARIES})

# glew
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
link_libraries(GLEW::GLEW)

# glfw
find_package(glfw3 REQUIRED)
include_directories(${GLFW_INCLUDE_DIRS})
link_directories(${GLFW_LIBRARY_DIRS})
link_libraries(glfw)
#add_compile_definitions(GLEW_STATIC)

endif ()
if(APPLE)
	include_directories(/usr/local/opt/ffmpeg/include)
	link_directories(/usr/local/opt/ffmpeg/lib)
endif()


include_directories(
	../../include/
)
link_directories(
	../../lib/
)
link_libraries(
	ndi
	avcodec
	avformat
	avutil
	avahi-client
	GLEW
	glut
	glfw
)

add_executable(example-recv-gl
	ndi_recv_gl.cpp
	../common/ogl.cpp
	../common/egl.cpp
	../common/glfw.cpp
	../common/yuv.cpp
)
