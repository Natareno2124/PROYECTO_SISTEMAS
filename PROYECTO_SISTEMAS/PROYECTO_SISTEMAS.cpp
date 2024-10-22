#include <iostream>
#include <queue>
#include <thread>
#include <chrono>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <stdexcept>

using namespace std;

class Proceso {
public:
    int pid;
    int ppid;
    string pc;
    int registros;
    int tamano;
    int hilos;
    int quantum;
    int iteracion;
    string estado;
    int tiempo_ejecucion;

    Proceso(int _pid, int _ppid, string _pc, int _registros, int _tamano, int _hilos, int _quantum, int _iteracion) {
        pid = _pid;
        ppid = _ppid;
        pc = _pc;
        registros = _registros;
        tamano = _tamano;
        hilos = _hilos;
        quantum = _quantum;
        iteracion = _iteracion;
        estado = "listo";
        tiempo_ejecucion = 0;
    }

    void ejecutar(int quantum) {
        cout << "Ejecutando Proceso " << pid << " en estado " << estado << endl;
        estado = "ejecucion";
        this_thread::sleep_for(chrono::seconds(quantum));
        iteracion--;
        tiempo_ejecucion += quantum;

        if (iteracion <= 0) {
            estado = "terminado";
            cout << "Proceso " << pid << " ha terminado" << endl;
        }
        else {
            estado = "listo";
            cout << "Proceso " << pid << " ha completado " << quantum << " segundos de ejecucion, le quedan " << iteracion << " iteraciones" << endl;
        }
    }
};

class MultilevelFeedbackQueueScheduler {
public:
    queue<Proceso*> cola_A;
    queue<Proceso*> cola_B;
    queue<Proceso*> cola_C;
    queue<Proceso*> cola_espera;  // Cola para procesos que esperan por hilos
    int tiempo_total;
    int ultimo_tiempo_promocion;
    int hilos_disponibles;

    MultilevelFeedbackQueueScheduler(int hilos_totales) {
        tiempo_total = 0;
        ultimo_tiempo_promocion = 0;
        hilos_disponibles = hilos_totales;  // Inicializa con los hilos disponibles en el sistema
    }

    void agregar_proceso(Proceso* proceso) {
        // Validar si los hilos disponibles son suficientes para este proceso
        if (proceso->hilos <= hilos_disponibles) {
            cout << "Agregando Proceso " << proceso->pid << " a la cola de alta prioridad" << endl;
            cola_A.push(proceso);
            hilos_disponibles -= proceso->hilos;  // Reducir los hilos disponibles
        }
        else {
            // Si no hay suficientes hilos, colocarlo en la cola de espera
            cout << "Proceso " << proceso->pid << " necesita " << proceso->hilos
                << " hilos, pero solo hay " << hilos_disponibles << " disponibles. "
                << "Colocando en la cola de espera." << endl;
            cola_espera.push(proceso);
        }
    }

    void liberar_hilos(Proceso* proceso) {
        hilos_disponibles += proceso->hilos;  // Liberar los hilos cuando un proceso termina
    }

    void verificar_cola_espera() {
        // Verificar si alg�n proceso en espera puede ejecutarse
        while (!cola_espera.empty()) {
            Proceso* proceso = cola_espera.front();
            if (proceso->hilos <= hilos_disponibles) {
                cout << "Agregando Proceso " << proceso->pid << " desde la cola de espera a la cola de alta prioridad" << endl;
                cola_espera.pop();
                cola_A.push(proceso);
                hilos_disponibles -= proceso->hilos;
            }
            else {
                // Si el primer proceso no puede ser ejecutado, salir
                break;
            }
        }
    }

    void ejecutar_procesos() {
        while (!cola_A.empty() || !cola_B.empty() || !cola_C.empty() || !cola_espera.empty()) {
            verificar_cola_espera();  // Verificar si alg�n proceso de espera puede ejecutarse

            if (!cola_A.empty()) {
                Proceso* proceso = cola_A.front();
                cola_A.pop();
                proceso->ejecutar(5);
                tiempo_total += 5;

                if (proceso->estado == "terminado") {
                    liberar_hilos(proceso);  // Liberar los hilos si el proceso ha terminado
                }
                else {
                    cola_B.push(proceso);
                }
            }
            else if (!cola_B.empty()) {
                Proceso* proceso = cola_B.front();
                cola_B.pop();
                proceso->ejecutar(10);
                tiempo_total += 10;

                if (proceso->estado == "terminado") {
                    liberar_hilos(proceso);  // Liberar los hilos si el proceso ha terminado
                }
                else {
                    cola_C.push(proceso);
                }
            }
            else if (!cola_C.empty()) {
                Proceso* proceso = cola_C.front();
                cola_C.pop();
                proceso->ejecutar(proceso->quantum);
                tiempo_total += proceso->quantum;

                if (proceso->estado == "terminado") {
                    liberar_hilos(proceso);  // Liberar los hilos si el proceso ha terminado
                }
                else {
                    cola_C.push(proceso);
                }
            }
        }
    }
};


// Funci�n para cargar procesos desde archivo con validaci�n de caracteres no num�ricos y n�meros negativos
void cargar_procesos_desde_archivo(string archivo, MultilevelFeedbackQueueScheduler& scheduler, int& procesadores, int& hilos_totales) {
    ifstream infile(archivo);
    string line;
    int linea_actual = 0;

    while (getline(infile, line)) {
        linea_actual++;
        istringstream ss(line);
        string item;

        // Leer las dos primeras l�neas para capturar los procesadores e hilos
        if (linea_actual == 1) {
            ss >> item >> procesadores;  // Leer el n�mero de procesadores
            continue;
        }
        if (linea_actual == 2) {
            ss >> item >> hilos_totales;  // Leer el n�mero de hilos totales
            continue;
        }

        // A partir de la tercera l�nea, procesamos los procesos
        vector<string> data;

        // Parsear los datos del proceso desde la l�nea usando '|' como delimitador
        while (getline(ss, item, '|')) {
            data.push_back(item);
        }

        // Verificar que se hayan le�do todos los campos necesarios
        if (data.size() != 8) {
            cout << "Error: Numero incorrecto de campos en la linea: " << line << endl;
            continue;
        }

        try {
            // Validar los valores num�ricos
            int pid = stoi(data[0]);
            int ppid = stoi(data[1]);
            string pc = data[2];
            int registros = stoi(data[3]);
            int tamano = stoi(data[4]);
            int hilos = stoi(data[5]);
            int quantum = stoi(data[6]);
            int iteracion = stoi(data[7]);

            // Validar que ning�n valor num�rico sea negativo y que los hilos no excedan los hilos totales disponibles
            if (pid < 0 || ppid < 0 || registros < 0 || tamano < 0 || hilos < 0 || quantum < 0 || iteracion < 0 || hilos > hilos_totales) {
                cout << "Error: Proceso con parametros negativos o exceso de hilos encontrado. Linea: " << line << endl;
                continue;
            }

            // Crear el proceso y agregarlo al scheduler
            Proceso* proceso = new Proceso(pid, ppid, pc, registros, tamano, hilos, quantum, iteracion);
            scheduler.agregar_proceso(proceso);
        }
        catch (const invalid_argument& e) {
            cout << "Error: Caracteres no numericos en un campo numerico. Linea: " << line << endl;
        }
        catch (const out_of_range& e) {
            cout << "Error: Valor fuera de rango en la linea: " << line << ". Error: " << e.what() << endl;
        }
    }
    infile.close();
}



//Funcion para validar si tiene decimales
bool contiene_decimales(const string& texto) {
    for (int i = 0; i < texto.length(); i++) {
        if (texto[i] == '.') {
            return true;
        }
    }
}


int main() {
    // Variables para los par�metros de procesadores y hilos que se leer�n del archivo
    int procesadores = 0;
    int hilos_totales = 0;

    // Abrir el archivo para leer los par�metros de procesadores e hilos
    ifstream infile("procesos.dat");
    if (!infile.is_open()) {
        cerr << "Error al abrir el archivo." << endl;
        return 1;
    }

    // Leer las primeras dos l�neas del archivo para obtener procesadores y hilos
    string line;

    // Leer la l�nea de Procesadores
    if (getline(infile, line)) {
        istringstream ss(line);
        string label;
        ss >> label >> procesadores;  // Leer la palabra "Procesadores" y luego el n�mero
    }

    // Leer la l�nea de Hilos
    if (getline(infile, line)) {
        istringstream ss(line);
        string label;
        ss >> label >> hilos_totales;  // Leer la palabra "Hilos" y luego el n�mero
    }
    cout << "Configuracion:" << endl;
    cout << "Procesadores: " << procesadores << endl;
    cout << "Hilos: " << hilos_totales << endl;




    // Crear el scheduler con el n�mero de hilos disponibles
    MultilevelFeedbackQueueScheduler scheduler(hilos_totales);

    // Cargar los procesos desde el archivo (saltando las dos primeras l�neas ya le�das)
    cargar_procesos_desde_archivo("procesos.dat", scheduler, procesadores, hilos_totales);

    // Ejecutar los procesos
    scheduler.ejecutar_procesos();

    return 0;
}