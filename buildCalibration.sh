g++ calibrationMain.cpp findCircle.cpp --std=c++0x `pkg-config --cflags opencv` -g -o autoCalibrate -Wall -Werror -ljpeg -pthread -lX11 `pkg-config --libs opencv`
