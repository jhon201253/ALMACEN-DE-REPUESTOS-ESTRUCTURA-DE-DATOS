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

/* ===================== INTERFAZ: COLORES Y MARCOS ===================== */
const int ANCHO = 79;          // ancho total de los marcos

// Colores (atributos de consola de Windows: texto + fondo * 16)
const int C_NORMAL = 7, C_BORDE = 11, C_TITULO = 14, C_OPCION = 10,
          C_OK = 10, C_ERROR = 12, C_AVISO = 14, C_CABECERA = 31, C_DATO = 15;

void setColor(int c) {
#ifdef _WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), c);
#else
    static const int ansi[8] = {0, 4, 2, 6, 1, 5, 3, 7};   // prueba en Linux/Mac
    int fg = c & 15, bg = (c >> 4) & 15;
    cout << "\033[" << (fg > 7 ? 90 : 30) + ansi[fg & 7] << ";"
         << (bg > 7 ? 100 : 40) + ansi[bg & 7] << "m";
#endif
}

// Caracteres de dibujo (codigos de la consola de Windows)
string G(int codigo) {
#ifdef _WIN32
    return string(1, (char)codigo);
#else
    switch (codigo) {
        case 196: return "\u2500"; case 179: return "\u2502"; case 218: return "\u250c";
        case 191: return "\u2510"; case 192: return "\u2514"; case 217: return "\u2518";
        case 194: return "\u252c"; case 193: return "\u2534"; case 195: return "\u251c";
        case 180: return "\u2524"; case 197: return "\u253c"; case 205: return "\u2550";
        case 186: return "\u2551"; case 201: return "\u2554"; case 187: return "\u2557";
        case 200: return "\u255a"; case 188: return "\u255d"; case 204: return "\u2560";
        case 185: return "\u2563"; case 219: return "\u2588";
    }
    return "?";
#endif
}

string repetir(const string& s, int n) {
    string r;
    for (int i = 0; i < n; i++) r += s;
    return r;
}

string aTexto(int n) { stringstream ss; ss << n; return ss.str(); }
string aTexto(double d) { stringstream ss; ss << fixed << setprecision(2) << d; return ss.str(); }

string ajustar(string s, int ancho) {            // recorta o rellena con espacios
    if ((int)s.size() > ancho) s = s.substr(0, ancho);
    return s + string(ancho - s.size(), ' ');
}

void limpiarPantalla() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pausa() {
    setColor(C_NORMAL);
    cout << "\n  Presione Enter para continuar...";
    cin.get();
}

// ----- mensajes -----
void ok(const string& m)    { setColor(C_OK);    cout << "  [OK] " << m << "\n"; setColor(C_NORMAL); }
void error(const string& m) { setColor(C_ERROR); cout << "  [X]  " << m << "\n"; setColor(C_NORMAL); }
void aviso(const string& m) { setColor(C_AVISO); cout << "  [!]  " << m << "\n"; setColor(C_NORMAL); }

// ----- marcos dobles -----
void marcoSuperior() { setColor(C_BORDE); cout << G(201) << repetir(G(205), ANCHO - 2) << G(187) << "\n"; }
void marcoInferior() { setColor(C_BORDE); cout << G(200) << repetir(G(205), ANCHO - 2) << G(188) << "\n"; setColor(C_NORMAL); }
void marcoDivisor()  { setColor(C_BORDE); cout << G(204) << repetir(G(205), ANCHO - 2) << G(185) << "\n"; }

void marcoTexto(const string& t, int color, bool centrado) {
    int interior = ANCHO - 2;
    int izq = centrado ? (interior - (int)t.size()) / 2 : 2;
    if (izq < 0) izq = 0;
    string linea = string(izq, ' ') + t;
    setColor(C_BORDE); cout << G(186);
    setColor(color);   cout << ajustar(linea, interior);
    setColor(C_BORDE); cout << G(186) << "\n";
}
void marcoOpcion(const string& tecla, const string& texto) {
    int interior = ANCHO - 2;
    string resto = "  " + texto;
    setColor(C_BORDE);  cout << G(186) << "   ";
    setColor(C_OPCION); cout << "[" << tecla << "]";
    setColor(C_NORMAL); cout << ajustar(resto, interior - 3 - (int)tecla.size() - 2);
    setColor(C_BORDE);  cout << G(186) << "\n";
}

void marcoVacio() { marcoTexto("", C_NORMAL, false); }

// ----- encabezado de cada pantalla -----
void cabecera(const string& titulo) {
    limpiarPantalla();
    marcoSuperior();
    marcoTexto("A L M A C E N   D E   A U T O P A R T E S", C_TITULO, true);
    marcoTexto("Universidad Continental - Estructura de Datos", C_NORMAL, true);
    marcoDivisor();
    marcoTexto(titulo, C_DATO, true);
    marcoInferior();
    cout << "\n";
}

// ----- tablas con bordes -----
void lineaTabla(const int* w, int n, int izq, int med, int der) {
    setColor(C_BORDE);
    cout << "  " << G(izq);
    for (int i = 0; i < n; i++) {
        cout << repetir(G(196), w[i] + 2);
        cout << G(i < n - 1 ? med : der);
    }
    cout << "\n";
    setColor(C_NORMAL);
}

void filaTabla(const string* t, const int* w, int n, const int* colores) {
    setColor(C_BORDE); cout << "  " << G(179);
    for (int i = 0; i < n; i++) {
        setColor(colores[i]); cout << " " << ajustar(t[i], w[i]) << " ";
        setColor(C_BORDE);    cout << G(179);
    }
    cout << "\n";
    setColor(C_NORMAL);
}

// Tabla de repuestos (columnas fijas)
const int WREP[5] = {8, 24, 14, 10, 7};

void encabezadoRepuestos() {
    const string t[5] = {"CODIGO", "NOMBRE", "MARCA", "PRECIO", "STOCK"};
    const int c[5] = {C_TITULO, C_TITULO, C_TITULO, C_TITULO, C_TITULO};
    lineaTabla(WREP, 5, 218, 194, 191);
    filaTabla(t, WREP, 5, c);
    lineaTabla(WREP, 5, 195, 197, 180);
}

void filaRepuesto(Repuesto* r) {
    string t[5] = {aTexto(r->codigo), r->nombre, r->marca, "S/ " + aTexto(r->precio), aTexto(r->cantidad)};
    int c[5] = {C_DATO, C_NORMAL, C_NORMAL, C_NORMAL, r->cantidad <= 5 ? C_ERROR : C_OK};
    filaTabla(t, WREP, 5, c);
}

void pieRepuestos() { lineaTabla(WREP, 5, 192, 193, 217); }

/* ===================== LECTURA VALIDADA ===================== */
int leerEntero(const string& mensaje) {
    int valor;
    while (true) {
        setColor(C_BORDE); cout << mensaje; setColor(C_DATO);
        if (cin >> valor) { cin.ignore(10000, '\n'); setColor(C_NORMAL); return valor; }
        cin.clear();
        cin.ignore(10000, '\n');
        error("Valor invalido. Ingrese un numero entero.");
    }
}

double leerReal(const string& mensaje) {
    double valor;
    while (true) {
        setColor(C_BORDE); cout << mensaje; setColor(C_DATO);
        if (cin >> valor) { cin.ignore(10000, '\n'); setColor(C_NORMAL); return valor; }
        cin.clear();
        cin.ignore(10000, '\n');
        error("Valor invalido. Ingrese un numero.");
    }
}

string leerTexto(const string& mensaje) {
    string texto;
    do {
        setColor(C_BORDE); cout << mensaje; setColor(C_DATO);
        getline(cin, texto);
        if (texto.empty()) error("El campo no puede estar vacio.");
    } while (texto.empty());
    for (size_t i = 0; i < texto.size(); i++)
        if (texto[i] == '|') texto[i] = '-';     // '|' separa los campos del archivo
    setColor(C_NORMAL);
    return texto;
}

/* ===================== LISTA: CATALOGO DE REPUESTOS ===================== */
Repuesto* buscarRepuesto(int codigo) {
    Repuesto* actual = inicio;
    while (actual != NULL && actual->codigo != codigo)
        actual = actual->siguiente;
    return actual;                       // NULL si no existe
}

void insertarRepuesto(int codigo, string nombre, string marca, double precio, int cantidad) {
    if (buscarRepuesto(codigo) != NULL) {
        error("El codigo " + aTexto(codigo) + " ya existe.");
        return;
    }
    Repuesto* nuevo = new Repuesto;
    nuevo->codigo = codigo;
    nuevo->nombre = nombre;
    nuevo->marca = marca;
    nuevo->precio = precio;
    nuevo->cantidad = cantidad;
    nuevo->siguiente = NULL;

    if (inicio == NULL) {                // lista vacia
        inicio = nuevo;
    } else {                             // recorre hasta el ultimo nodo
        Repuesto* actual = inicio;
        while (actual->siguiente != NULL)
            actual = actual->siguiente;
        actual->siguiente = nuevo;
    }
    ok("Repuesto registrado correctamente.");
    guardarTodo();
}

