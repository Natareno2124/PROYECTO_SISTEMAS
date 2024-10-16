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
        estado = "ejecución";
        this_thread::sleep_for(chrono::seconds(quantum));
        iteracion--;
        tiempo_ejecucion += quantum;

        if (iteracion <= 0) {
            estado = "terminado";
            cout << "Proceso " << pid << " ha terminado" << endl;
        }
        else {
            estado = "listo";
            cout << "Proceso " << pid << " ha completado " << quantum << " segundos de ejecución, le quedan " << iteracion << " iteraciones" << endl;
        }
    }
};

class MultilevelFeedbackQueueScheduler {
public:
    queue<Proceso*> cola_A;
    queue<Proceso*> cola_B;
    queue<Proceso*> cola_C;
    int tiempo_total;
    int ultimo_tiempo_promocion;

    MultilevelFeedbackQueueScheduler() {
        tiempo_total = 0;
        ultimo_tiempo_promocion = 0;
    }

    void agregar_proceso(Proceso* proceso) {
        cout << "Agregando Proceso " << proceso->pid << " a la cola de alta prioridad" << endl;
        cola_A.push(proceso);
    }

    void promocionar_procesos() {
        cout << "Promoviendo todos los procesos a la cola de alta prioridad para evitar Starvation" << endl;
        while (!cola_B.empty()) {
            cola_A.push(cola_B.front());
            cola_B.pop();
        }
        while (!cola_C.empty()) {
            cola_A.push(cola_C.front());
            cola_C.pop();
        }
        ultimo_tiempo_promocion = tiempo_total;
    }

    void ejecutar_procesos() {
        while (!cola_A.empty() || !cola_B.empty() || !cola_C.empty()) {
            if (tiempo_total - ultimo_tiempo_promocion >= 60) {
                promocionar_procesos();
            }

            if (!cola_A.empty()) {
                Proceso* proceso = cola_A.front();
                cola_A.pop();
                proceso->ejecutar(5);
                tiempo_total += 5;
                if (proceso->estado != "terminado") {
                    cola_B.push(proceso);
                }
            }
            else if (!cola_B.empty()) {
                Proceso* proceso = cola_B.front();
                cola_B.pop();
                proceso->ejecutar(10);
                tiempo_total += 10;
                if (proceso->estado != "terminado") {
                    cola_C.push(proceso);
                }
            }
            else if (!cola_C.empty()) {
                Proceso* proceso = cola_C.front();
                cola_C.pop();
                proceso->ejecutar(proceso->quantum);
                tiempo_total += proceso->quantum;
                if (proceso->estado != "terminado") {
                    cola_C.push(proceso);
                }
            }
        }
    }
};

// Función para cargar procesos desde archivo .dat con manejo de errores
void cargar_procesos_desde_archivo(string archivo, MultilevelFeedbackQueueScheduler& scheduler) {
    ifstream infile(archivo);
    if (!infile.is_open()) {
        cerr << "Error al abrir el archivo: " << archivo << endl;
        return;
    }

    string line;
    while (getline(infile, line)) {
        // Verifica si la línea está vacía
        if (line.empty()) {
            cerr << "Línea vacía encontrada, saltando..." << endl;
            continue;
        }

        try {
            stringstream ss(line);
            string item;
            vector<string> data;

            // Usar '|' como delimitador
            while (getline(ss, item, '|')) {
                data.push_back(item);
            }

            // Verifica si el número de campos es el correcto
            if (data.size() != 8) {
                cerr << "Error: Se esperaban 8 campos pero se encontraron " << data.size() << ". Saltando línea..." << endl;
                continue;
            }

            // Crear un nuevo proceso con los datos obtenidos
            Proceso* proceso = new Proceso(
                stoi(data[0]),  // pid
                stoi(data[1]),  // ppid
                data[2],        // pc
                stoi(data[3]),  // registros
                stoi(data[4]),  // tamano
                stoi(data[5]),  // hilos
                stoi(data[6]),  // quantum
                stoi(data[7])   // iteracion
            );

            // Agregar el proceso al scheduler
            scheduler.agregar_proceso(proceso);

        }
        catch (const invalid_argument& e) {
            cerr << "Error al convertir los datos de la línea: " << line << ". Error: " << e.what() << endl;
        }
        catch (const out_of_range& e) {
            cerr << "Valor fuera de rango en la línea: " << line << ". Error: " << e.what() << endl;
        }
    }
    infile.close();
}

int main() {
    MultilevelFeedbackQueueScheduler scheduler;

    // Cargar procesos desde el archivo 'procesos.dat'
    cargar_procesos_desde_archivo("procesos.dat", scheduler);

    // Ejecutar los procesos
    scheduler.ejecutar_procesos();

    return 0;
}
