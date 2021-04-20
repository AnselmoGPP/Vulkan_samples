# cd /home/username/dev/visual/_BUILD
# cmake -G"Unix Makefiles" ..
# cmake -G"Visual Studio 15 2017 Win64" ..
# cmake -G"Visual Studio 15 2017 Win32" ..

CMAKE_MINIMUM_REQUIRED (VERSION 3.12)
PROJECT (Vulkan_tests)

SET(PROJ_NAME "Vk_1")
ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/${PROJ_NAME})
SET(PROJ_NAME "Vk_2")
ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/${PROJ_NAME})
