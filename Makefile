all:
	g++ -Wall -o BicubicInterpolator main.cpp src/Interpolator.cpp src/Image.cpp -Isrc ./libs/SOIL/lib/libSOIL.a -I./libs/SOIL/include -framework OpenGL -lpthread
