CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

if( UNIX )
	#set(CMAKE_CXX_COMPILER "/usr/bin/c++")
	#set(CMAKE_C_COMPILER "gcc-9")
	#set(CMAKE_CXX_COMPILER "/usr/bin/g++-9")
endif()

PROJECT(Vk_4
	VERSION 1.0
	DESCRIPTION "Texture mapping (image view and sampler). Rotating 3D textured square."
	LANGUAGES CXX
	)

SET(CMAKE_CXX_STANDARD 17)
FIND_PACKAGE(Vulkan REQUIRED)

MESSAGE(STATUS "Project: " ${PROJECT_NAME})

ADD_DEFINITIONS(
	-std=c++17
	-O2
)
#ADD_COMPILE_DEFINITIONS( IMGUI_IMPL_OPENGL_LOADER_GLEW=1 )
#ADD_COMPILE_DEFINITIONS( IMGUI_IMPL_OPENGL_LOADER_GLAD=1 )

ADD_EXECUTABLE(${PROJECT_NAME}
	src/main.cpp
	src/triangle.cpp

	include/params.hpp
	include/triangle.hpp

	shaders/triangleV.vert
	shaders/triangleF.frag

	../../files/TODO.txt
	CMakeLists.txt

	#shaders/terrain.vs
	#shaders/terrain.fs
	#shaders/sea.vs
	#shaders/sea.fs
	#shaders/axis.vs
	#shaders/axis.fs
	#shaders/sun.vs
	#shaders/sun.fs
	#shaders/icosahedron.vs
	#shaders/icosahedron.fs
)

TARGET_SOURCES(${PROJECT_NAME} PRIVATE
	#../../extern/glad/src/glad.c
	#../../extern/glew/glew-2.1.0/src/glew.c

	#../../extern/imgui/imgui-1.72b/imgui.cpp
	#../../extern/imgui/imgui-1.72b/imgui_demo.cpp
	#../../extern/imgui/imgui-1.72b/imgui_draw.cpp
	#../../extern/imgui/imgui-1.72b/imgui_widgets.cpp
	#../../extern/imgui/imgui-1.72b/examples/imgui_impl_glfw.cpp
	#../../extern/imgui/imgui-1.72b/examples/imgui_impl_opengl3.cpp
)

TARGET_INCLUDE_DIRECTORIES( ${PROJECT_NAME} PUBLIC
	include
	../../extern/glfw/glfw-3.3.2/include
	../../extern/glm/glm-0.9.9.5
	../../extern/stb
	#../../extern/vulkansdk-linux-x86_64-1.2.170.0/1.2.170.0/x86_64/include
	
	#../../extern/FastNoise

	#../../extern/glad/include
	#../../extern/glew/glew-2.1.0/include

	#../../extern/imgui/imgui-1.72b

	#../../extern/eigen-3.3.8
)

if( UNIX )
	#TARGET_INCLUDE_DIRECTORIES( ${PROJECT_NAME} PUBLIC /usr/include/c++/9 )

	TARGET_LINK_LIBRARIES( ${PROJECT_NAME} 
		${PROJECT_SOURCE_DIR}/../../_BUILD/extern/glfw/glfw-3.3.2/src/libglfw3.a
		${PROJECT_SOURCE_DIR}/../../_BUILD/extern/glm/glm-0.9.9.5/glm/libglm_static.a
		-lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi #-lglfw 
	)
elseif( WIN32 )
	TARGET_LINK_LIBRARIES( ${PROJECT_NAME} 
		optimized	${PROJECT_SOURCE_DIR}/../../_BUILD/extern/glfw/glfw-3.3.2/src/Release/glfw3.lib
		optimized	${PROJECT_SOURCE_DIR}/../../_BUILD/extern/glm/glm-0.9.9.5/glm/Release/glm_static.lib
		Vulkan::Vulkan
	)
endif()











