all:
	g++ -O3 microscope.cpp -o microscope -lcomctl32 -lcomdlg32 -lgdi32 -lgdiplus -static-libgcc -static-libstdc++ -Wl,-subsystem,windows
debug:
	g++ -ggdb microscope.cpp -o microscope_debug -lcomctl32 -lcomdlg32 -lgdi32 -lgdiplus -static-libgcc -static-libstdc++