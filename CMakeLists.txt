# Z:
# cd Z:/Git/Console_software/console_3D_engine/_BUILD
# cmake -G"Visual Studio 15 2017" ..

CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT (console_3D_engine)

MESSAGE(STATUS "Project name " ${PROJECT_NAME})

add_executable( ${PROJECT_NAME}
	src/main.cpp
)