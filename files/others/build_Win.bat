ECHO OFF
ECHO Current directory: 
CD
MKDIR _BUILD
CD _BUILD
cmake -G"Visual Studio 16 2019" ..
#PAUSE

# Open MVS and set the project you want as "startup project" (proyecto de inicio).
# Compile with MVS: first, compile GLFW static library; then, the project you want. 
# Compile both in release mode because this program is linking (in his CMakeLists.txt) the GLFW static library compiled in release mode.
# Put the correct shader's directory names when creating the shading program
