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

// Función para cargar procesos desde archivo con validación de caracteres no numéricos y números negativos
void cargar_procesos_desde_archivo(string archivo, MultilevelFeedbackQueueScheduler& scheduler) {
    ifstream infile(archivo);
    string line;
    while (getline(infile, line)) {
        istringstream ss(line);
        string item;
        vector<string> data;

        // Parsear los datos del proceso desde la línea usando '|' como delimitador
        while (getline(ss, item, '|')) {
            data.push_back(item);
        }

        // Verificar que se hayan leído todos los campos necesarios
        if (data.size() != 8) {
            cout << "Error: Numero incorrecto de campos en la linea: " << line << endl;
            continue;
        }

        try {
            // Validar que los valores numéricos no contengan letras u otros caracteres no válidos
            int pid = stoi(data[0]);
            int ppid = stoi(data[1]);
            string pc = data[2];  // Este campo puede ser una cadena, no necesita validación numérica
            int registros = stoi(data[3]);
            int tamano = stoi(data[4]);
            int hilos = stoi(data[5]);
            int quantum = stoi(data[6]);
            int iteracion = stoi(data[7]);

            // Validar que ningún valor numérico sea negativo
            if (pid < 0 || ppid < 0 || registros < 0 || tamano < 0 || hilos < 0 || quantum < 0 || iteracion < 0) {
                cout << "Error: Proceso con parametros negativos encontrado. Linea: " << line << endl;
                continue;  // Saltar este proceso y continuar con el siguiente
            }

            // Si los valores son válidos, crear el proceso y agregarlo al scheduler
            Proceso* proceso = new Proceso(pid, ppid, pc, registros, tamano, hilos, quantum, iteracion);
            scheduler.agregar_proceso(proceso);
        }
        catch (const invalid_argument& e) {
            // Si ocurre una excepción al convertir un valor a número, significa que se ingresó una letra u otro carácter no numérico
            cout << "Error: Caracteres no numericos en un campo numerico. Linea: " << line << endl;
        }
        catch (const out_of_range& e) {
            // Manejar valores fuera del rango de los tipos de datos
            cout << "Error: Valor fuera de rango en la linea: " << line << ". Error: " << e.what() << endl;
        }
    }
    infile.close();
}

int main() {
    MultilevelFeedbackQueueScheduler scheduler;

    // Cargar los procesos desde el archivo
    cargar_procesos_desde_archivo("procesos.dat", scheduler);

    // Ejecutar los procesos
    scheduler.ejecutar_procesos();

    return 0;
}