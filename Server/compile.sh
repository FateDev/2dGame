g++ -g `find -name '*.cpp'` -o server -ldl -lsfml-system -lsfml-network -lcurl -std=c++11
chmod +x server
#./server
