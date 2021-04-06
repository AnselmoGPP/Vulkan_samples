# cd /home/username/dev/visual/_BUILD
# cmake -G"Unix Makefiles" ..
# cmake -G"Visual Studio 15 2017 Win64" ..
# cmake -G"Visual Studio 15 2017 Win32" ..

CMAKE_MINIMUM_REQUIRED (VERSION 3.12)
PROJECT (in_source_dependencies)

# Dependencies
ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/glfw/glfw-3.3.2)
ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/glm/glm-0.9.9.5)

# Main project
#SET(PROJ_NAME "Vk_1")
#ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/projects/${PROJ_NAME})
