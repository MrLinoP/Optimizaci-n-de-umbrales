#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <numeric> // Para std::accumulate

using namespace std;

// Estructura para representar una ballena
struct Ballena {
    vector<int> umbrales; // Los umbrales representan posiciones
    double fitness;
};

// Parámetros del algoritmo
const int tamanhoPoblacion = 100;
const int maxIteraciones = 100;
const int limiteInferior = 0;
const int limiteSuperior = 255; // Valores de intensidad de píxeles
const double b = 1.0; // Constante para la espiral
const double M_PI = 3.14159265358979323846;

// Leer la imagen desde un archivo de texto
vector<vector<int>> leerImagenDesdeArchivo(const string& nombreArchivo, int filas, int columnas) {
    vector<vector<int>> imagen(filas, vector<int>(columnas));
    ifstream archivo(nombreArchivo,ios::in);
    if (!archivo) {
        cerr << "Error al abrir el archivo: " << nombreArchivo << endl;
        exit(1);
    }

    string linea;
    int fila = 0;
    while (getline(archivo, linea) && fila < filas) {
        stringstream ss(linea);
        for (int col = 0; col < columnas; ++col) {
            ss >> imagen[fila][col];
        }
        ++fila;
    }
    archivo.close();
    return imagen;
}

// Función para calcular el histograma 1D
vector<int> calcularHistograma1D(const vector<vector<int>>& img) {
    vector<int> hist(256, 0); // Inicializa el histograma con 256 bins, todos en 0.

    for (const auto& fila : img) {
        for (int valor : fila) {
            hist[valor]++; // Incrementa la posición correspondiente al nivel de intensidad.
        }
    }
    return hist;
}

// Función para calcular la entropía de Kapur usando el histograma 1D
double calcularEntropiaKapur(const vector<int>& hist, const vector<int>& umbrales) {
    vector<double> probabilidades;
    double total = accumulate(hist.begin(), hist.end(), 0.0); // Total de píxeles

    // Calcular probabilidades acumuladas para los umbrales
    for (size_t i = 0; i < umbrales.size(); ++i) {
        int inicio = (i == 0) ? 0 : umbrales[i - 1];
        int fin = umbrales[i];
        double suma = accumulate(hist.begin() + inicio, hist.begin() + fin, 0.0);
        probabilidades.push_back(suma / total);
    }

    // Última región (umbrales[k] a 255)
    double sumaUltima = accumulate(hist.begin() + umbrales.back(), hist.end(), 0.0);
    probabilidades.push_back(sumaUltima / total);

    // Calcular la entropía total
    double entropia = 0.0;
    for (double p : probabilidades) {
        if (p > 0) entropia += p * log(p);
    }
    return entropia; // Maximizar la entropía
}

// Inicializar una ballena con posiciones aleatorias
Ballena inicializarBallena(int umbrales, mt19937& rng, uniform_int_distribution<int>& dist) {
    Ballena ballena;
    for (int i = 0; i < umbrales; ++i) {
        ballena.umbrales.push_back(dist(rng));
    }
    sort(ballena.umbrales.begin(), ballena.umbrales.end()); // Ordenar umbrales
    ballena.fitness = 0.0; // Se calculará después
    return ballena;
}

// Actualizar la posición de una ballena
void actualizarPosicionBallena(Ballena& ballena, const Ballena& mejorBallena, mt19937& rng, 
                               uniform_int_distribution<int>& distPosiciones, double a, double pro) {
    uniform_real_distribution<double> distR(0.0, 1.0);
    uniform_real_distribution<double> distL(-1.0, 1.0);
    vector<int> nuevaPosicion(ballena.umbrales.size());
    double l = distL(rng);
    double r = distR(rng);
    double A = 2 * a * r - a;
    double C = 2 * r;

    if (pro < 0.5) { // Fase de encerrar presa
        if (fabs(A) < 1) { // Exploitación
            for (size_t i = 0; i < ballena.umbrales.size(); ++i) {
                int D = fabs(C * mejorBallena.umbrales[i] - ballena.umbrales[i]);
                nuevaPosicion[i] = mejorBallena.umbrales[i] - A * D;
            }
        } else { // Exploración
            for (size_t i = 0; i < ballena.umbrales.size(); ++i) {
                int D = fabs(C * distPosiciones(rng) - ballena.umbrales[i]);
                nuevaPosicion[i] = distPosiciones(rng) - A * D;
            }
        }
    } else { // Fase de ataque en espiral
        for (size_t i = 0; i < ballena.umbrales.size(); ++i) {
            int D = fabs(mejorBallena.umbrales[i] - ballena.umbrales[i]);
            nuevaPosicion[i] = D * exp(b * l) * cos(2 * M_PI * l) + mejorBallena.umbrales[i];
        }
    }

    // Restringir posición dentro de los límites
    for (int& pos : nuevaPosicion) {
        pos = max(min(pos, limiteSuperior), limiteInferior);
    }
    sort(nuevaPosicion.begin(), nuevaPosicion.end()); // Ordenar para mantener consistencia
    ballena.umbrales = nuevaPosicion;
}

// Comparar fitness
bool compara(const Ballena& a, const Ballena& b) {
    return a.fitness > b.fitness;
}

int main() {
    // Leer la imagen desde el archivo
    string nombreArchivo = "radiografia.txt";
    vector<vector<int>> img = leerImagenDesdeArchivo(nombreArchivo, 800, 800);

    // Calcular el histograma 1D
    vector<int> hist = calcularHistograma1D(img);

    // Parámetros del algoritmo
    int umbrales = 3; // Número de umbrales
    int semilla = 1102;
    mt19937 rng(semilla);
    uniform_int_distribution<int> distPosiciones(limiteInferior, limiteSuperior);
    uniform_real_distribution<double> distP(0.0, 1.0);

    // Inicializar población
    vector<Ballena> poblacion(tamanhoPoblacion);
    for (Ballena& ballena : poblacion) {
        ballena = inicializarBallena(umbrales, rng, distPosiciones);
        ballena.fitness = calcularEntropiaKapur(hist, ballena.umbrales);
    }

    // Mejor ballena inicial
    Ballena mejorBallena = *max_element(poblacion.begin(), poblacion.end(), compara);

    // Algoritmo principal
    for (int t = 0; t < maxIteraciones; ++t) {
        double a = 2.0 - t * (2.0 / maxIteraciones);

        for (Ballena& ballena : poblacion) {
            double pro = distP(rng);
            actualizarPosicionBallena(ballena, mejorBallena, rng, distPosiciones, a, pro);
            ballena.fitness = calcularEntropiaKapur(hist, ballena.umbrales);
        }

        // Actualizar la mejor ballena global
        Ballena actualMejor = *max_element(poblacion.begin(), poblacion.end(), compara);
        if (actualMejor.fitness > mejorBallena.fitness) {
            mejorBallena = actualMejor;
        }
    }

    // Resultado final
    cout << "Mejores umbrales encontrados: ";
    for (int x : mejorBallena.umbrales) {
        cout << x << " ";
    }
    cout << endl;

    return 0;
}
