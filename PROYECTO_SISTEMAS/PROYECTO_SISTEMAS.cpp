// PROYECTO_SISTEMAS.cpp 

#include <iostream>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <thread>  // Para manejar hilos
#include <chrono>  // Para simular la ejecuci�n
#include <mutex>
using namespace std;

int N; //procesadores
int K; //hilos


mutex mtx;

//Estructura de los procesos
struct Proceso {
	int ProcessId;
	int ParentProcessId;
	string ProgramCounter;
	int Registros;
	int Tamano;
	int Hilos;
	int Quantum;
	int Iteracion;
};



 
void lectura(const string &parametros) {
	ifstream archivo("parametros.txt");
	if (!archivo.is_open()) {
		cout << "Error al abrir el archivo de configuraci�n." << endl;
		exit(1);
	}

	string clave;
	while (archivo >> clave) {
		if (clave == "Procesadores") {
			archivo >> N;
			if (N<=0)
			{
				cout << "Error no se admiten parametros negativos." << endl;
				exit(1);
			}
		}
		else if (clave == "Hilos") {
			archivo >> K;
			if (K < 0)
			{
				cout << "Error no se admiten parametros negativos." << endl;
				exit(1);
			}
		}
	}
	archivo.close();
}



int main()
{
	lectura("parametros.txt");

	cout << "Configuraci�n del sistema:" << endl;
	cout << "N�mero de procesadores: " << N << endl;
	cout << "N�mero de hilos por procesador: " << K << endl;
}