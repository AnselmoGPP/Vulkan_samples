@ECHO OFF
ECHO Current directory: 
CD

RD /S /Q ..\_BUILD\projects
MKDIR ..\_BUILD
MKDIR ..\_BUILD\projects
CD ..\_BUILD\projects

cmake -G"Visual Studio 16 2019" ..\..\projects
PAUSE

REM The next step is to build the binaries:
REM 	- Open MVS and set the project you want as "startup project" (proyecto de inicio).
REM 	- Compile with MVS: first, compile GLFW static library; then, the project you want. 
REM 	- Compile both in release mode because this program is linking (in his CMakeLists.txt) the GLFW static library compiled in release mode.
REM 	- Put the correct shader's directory names when creating the shading program
