/*
 * ALMACEN DE AUTOPARTES - Estructura de Datos (Universidad Continental)
 * Grupo 3 - NRC 39606
 *
 * Estructuras dinamicas implementadas con punteros 
 *   - LISTA enlazada : catalogo de repuestos      (inicio)
 *   - COLA  (FIFO)   : clientes en espera         (frente, ultimo)
 *   - PILA  (LIFO)   : historial de ventas        (tope)
 *
 * Los datos se guardan en archivos de texto (repuestos.txt, clientes.txt,
 * ventas.txt) y se cargan automaticamente al iniciar el programa.
 * La interfaz usa colores y marcos de consola (en Windows, <windows.h>).
 */
#include <iostream>
#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdlib>
#ifdef _WIN32
#include <windows.h>
#endif
using namespace std;

/* ===================== ESTRUCTURAS (NODOS) ===================== */
struct Repuesto {              // nodo de la LISTA
    int codigo;
    string nombre;
    string marca;
    double precio;
    int cantidad;
    Repuesto* siguiente;
};

struct Cliente {               // nodo de la COLA
    string nombre;
    int codigoRepuesto;
    int cantidad;
    Cliente* siguiente;
};

struct Venta {                 // nodo de la PILA
    int id;
    string cliente;
    int codigoRepuesto;
    string nombreRepuesto;
    int cantidad;
    double total;
    Venta* siguiente;
};

/* ===================== PUNTEROS PRINCIPALES ===================== */
Repuesto* inicio = NULL;       // primer nodo de la lista
Cliente*  frente = NULL;       // primer cliente de la cola (sale primero)
Cliente*  ultimo = NULL;       // ultimo cliente de la cola (entra al final)
Venta*    tope   = NULL;       // ultima venta realizada (cima de la pila)
int contadorVentas = 0;

/* ===================== PERSISTENCIA EN ARCHIVOS DE TEXTO ===================== */
/* Cada registro ocupa una linea y sus campos van separados por '|' */
const char* ARCH_REPUESTOS = "repuestos.txt";
const char* ARCH_CLIENTES  = "clientes.txt";
const char* ARCH_VENTAS    = "ventas.txt";

void guardarTodo() {
    ofstream fr(ARCH_REPUESTOS);
    for (Repuesto* a = inicio; a != NULL; a = a->siguiente)
        fr << a->codigo << '|' << a->nombre << '|' << a->marca << '|'
           << a->precio << '|' << a->cantidad << '\n';
    fr.close();

    ofstream fc(ARCH_CLIENTES);                     // de frente a ultimo
    for (Cliente* c = frente; c != NULL; c = c->siguiente)
        fc << c->nombre << '|' << c->codigoRepuesto << '|' << c->cantidad << '\n';
    fc.close();

    ofstream fv(ARCH_VENTAS);                       // de tope a base
    for (Venta* v = tope; v != NULL; v = v->siguiente)
        fv << v->id << '|' << v->cliente << '|' << v->codigoRepuesto << '|'
           << v->nombreRepuesto << '|' << v->cantidad << '|' << v->total << '\n';
    fv.close();
}

void cargarTodo() {
    string linea, campo;
    int nRep = 0, nCli = 0, nVen = 0;

    ifstream fr(ARCH_REPUESTOS);
    Repuesto* ultimoRep = NULL;
    while (getline(fr, linea)) {
        if (linea.empty()) continue;
        stringstream ss(linea);
        Repuesto* nuevo = new Repuesto;
        getline(ss, campo, '|'); nuevo->codigo = atoi(campo.c_str());
        getline(ss, nuevo->nombre, '|');
        getline(ss, nuevo->marca, '|');
        getline(ss, campo, '|'); nuevo->precio = atof(campo.c_str());
        getline(ss, campo, '|'); nuevo->cantidad = atoi(campo.c_str());
        nuevo->siguiente = NULL;
        if (inicio == NULL) inicio = nuevo; else ultimoRep->siguiente = nuevo;
        ultimoRep = nuevo;
        nRep++;
    }
    fr.close();

    ifstream fc(ARCH_CLIENTES);
    while (getline(fc, linea)) {
        if (linea.empty()) continue;
        stringstream ss(linea);
        Cliente* nuevo = new Cliente;
        getline(ss, nuevo->nombre, '|');
        getline(ss, campo, '|'); nuevo->codigoRepuesto = atoi(campo.c_str());
        getline(ss, campo, '|'); nuevo->cantidad = atoi(campo.c_str());
        nuevo->siguiente = NULL;
        if (frente == NULL) frente = nuevo; else ultimo->siguiente = nuevo;
        ultimo = nuevo;
        nCli++;
    }
    fc.close();

    ifstream fv(ARCH_VENTAS);
    Venta* baseVenta = NULL;                        // el archivo viene de tope a base
    while (getline(fv, linea)) {
        if (linea.empty()) continue;
        stringstream ss(linea);
        Venta* nueva = new Venta;
        getline(ss, campo, '|'); nueva->id = atoi(campo.c_str());
        getline(ss, nueva->cliente, '|');
        getline(ss, campo, '|'); nueva->codigoRepuesto = atoi(campo.c_str());
        getline(ss, nueva->nombreRepuesto, '|');
        getline(ss, campo, '|'); nueva->cantidad = atoi(campo.c_str());
        getline(ss, campo, '|'); nueva->total = atof(campo.c_str());
        nueva->siguiente = NULL;
        if (tope == NULL) tope = nueva; else baseVenta->siguiente = nueva;
        baseVenta = nueva;
        if (nueva->id > contadorVentas) contadorVentas = nueva->id;
        nVen++;
    }
    fv.close();

    if (nRep + nCli + nVen > 0)
        cout << "Datos cargados: " << nRep << " repuestos, " << nCli
             << " clientes en cola y " << nVen << " ventas.\n";
}


