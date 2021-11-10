@ECHO OFF
ECHO Building project dependencies:
ECHO Current directory: 
CD
MKDIR ..\..\_BUILD
MKDIR ..\..\_BUILD\extern
CD ..\..\_BUILD\extern
cmake -G"Visual Studio 16 2019" ..\..\extern
PAUSE

REM The next step is to build the libraries (your dependencies for hansvisual):
REM 	- Open MVS and set the project you want as "startup project" (proyecto de inicio).
REM 	- Compile with MVS: first, compile GLFW static library; then, the project you want. 
REM 	- Compile both in release mode because this program is linking (in his CMakeLists.txt) the GLFW static library compiled in release mode.
REM 	- Put the correct shader's directory names when creating the shading program
