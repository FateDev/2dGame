g++ -g `find -name '*.cpp'` -o server -ldl -lsfml-system -lsfml-network -lcurl -lpthread -std=c++11
chmod +x server
#./server