g++ *.cpp -o server -ldl -lsfml-system -lsfml-network
echo "Compiled"
chmod +x server
./server
