plp_serial: PLP_serial.cpp
	g++ -std=c++11 PLP_serial.cpp -o plp_serial
plp: PLP.cpp
	g++ -std=c++11 -fopenmp PLP.cpp -o plp
clean:
	rm -f plp