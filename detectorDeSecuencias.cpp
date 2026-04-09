#include <iostream>
#include <string>
#include <vector>
#include <math.h>
#include <windows.h>
#include <conio.h>
#include <thread> 
#include <chrono> 
using namespace std;

//Maneja los estados*****************************************************************************************************************************************************
struct Estado{
	string id;
	string bits;
	string sgtEstadoX0;
	string sgtEstadoX1;
	string salida0;
	string salida1;
	Estado* sgt;
	Estado* ant;
};

//Maneja el mapa Karnaught****************************************************************************************************************************************************
struct Reflejo{
	int eje;
	int fil;
	int col;
	Reflejo* sgt;
};

struct Kcelda{
	string izq;	//numero bit posicion
	string der;	//numero bit posicion	
	string valor; //"0", "1" o "X"
	Reflejo* horizontal;
	Reflejo* vertical;
};

//Maneja el proceso de simplificacion****************************************************************************************************************************************************
struct Pasado{		//para el retroceso
	size_t x;
	size_t y;
	Pasado* sgt;	
};
struct Celda{	//agregar por el inicio (no por el final)  |  se guarda las celdas q se estan agrrupando, pero aun no llega al final
	size_t x;
	size_t y;
	Pasado* celdasPasadas;
	Celda* sgt;
	Celda* ant;
}; 

//Maneja el vector de 1s***************************************************************************************************************************************************
struct Buscado{
	size_t x;
	size_t y;
	bool marcado;	
};

struct Eje{
	int valor;
	int posicion;	//0: horizontal, 1: vertical
};

//Maneja las agrupaciones encontradas******************************************************************************************************************************************************
struct Grupo{
	Celda* unGrupo;
	Grupo* sgt;
};

//**********************************************************************************************************************************************************************

void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}


//TODO PARA EL KARNAUGHT
bool esPotencia2(int contCeldas) {		
    return (contCeldas > 0) && ((contCeldas & (contCeldas - 1)) == 0);
}

Buscado* buscando1SinMarcar(vector<Buscado>& buscados) {
    int tam = buscados.size();
    
    for (int i = 0; i < tam; i++) {
        if (!buscados[i].marcado) {
            return &buscados[i]; // Retorna un puntero al struct encontrado
        }
    }
    return NULL; // Retorna null si no se encontro
}

bool sinRecoger(Celda*& encontrados, size_t x, size_t y){		//busca el valor. Si no esta, manda true
	Celda* actual= encontrados;
	
	while(actual!=NULL){
		if(actual->x==x && actual->y==y){
			return false;
		}
		actual= actual->sgt;
	}
	return true;
}

bool sinPasar(Pasado* pasados, size_t x, size_t y) {
    Pasado* actual = pasados;

    while (actual != NULL) {
        if (actual->x == x && actual->y == y) {
            return false; 
        }
        actual = actual->sgt; 
    }
    return true; 
}

void agregarAlGrupo(Celda*& encontrados, int x, int y, int eje, int posEje, vector<Eje> &ejes) {
	// Crear una nueva celda
	Celda* nuevo = new Celda;
	nuevo->x = x;
	nuevo->y = y;
	nuevo->sgt = NULL;
	nuevo->ant = NULL;
	nuevo->celdasPasadas = NULL; 
	
	if (eje == 0) { 

		encontrados = nuevo;

	} else {

		Eje nuevoEje;
		nuevoEje.valor = eje;
		nuevoEje.posicion = posEje;
		ejes.push_back(nuevoEje);


		nuevo->sgt = encontrados; 
		if (encontrados != NULL) { 
			encontrados->ant = nuevo;
		}

		Pasado* nuevoPasado = new Pasado;
		nuevoPasado->x = x;
		nuevoPasado->y = y;
		nuevoPasado->sgt = NULL; 

		if (encontrados->celdasPasadas == NULL) {
			encontrados->celdasPasadas = nuevoPasado; 
		} else {
	
			Pasado* actual = encontrados->celdasPasadas;
			while (actual->sgt != NULL) {
				actual = actual->sgt;
			}
			actual->sgt = nuevoPasado;
			//cout<<nuevoPasado->x<<nuevoPasado->y<<endl;
		}

		encontrados = nuevo;
	}
}

bool buscarPosicion(Celda* actual, vector<vector<Kcelda>> &mapa, int eje, int posEje, size_t& x, size_t& y) {		//un poco raro el -1, pero SI funciona bien
    Celda* temp = actual;
    Kcelda* temp2 = &mapa[temp->x][temp->y];
    Reflejo* temp3 = (posEje == 0) ? temp2->horizontal : temp2->vertical;

    while (temp3 != NULL) {
        if (temp3->eje == eje) {
            x = temp3->fil;
            y = temp3->col;
            return true;
        }
        temp3 = temp3->sgt;
    }
    return false; // No se encontro reflejo
}

bool estaEnElMapa(vector<vector<Kcelda>> &mapa, size_t x, size_t y) {
    
    if (x >= mapa.size() || y >= mapa[0].size()) {
        return false;  
    }

    if (mapa[x][y].valor == "0") {
        return false;  
    } else {
        return true;  
    }
}

bool esPosible(const vector<Eje> &ejes, Celda*& encontrados, vector<vector<Kcelda>> &mapa){ //si los encontrados tienen posibilidad de encontrar a su reflejo (en el mapa)
	for (const auto &eje : ejes) {
        Celda* actual = encontrados;
        while (actual != NULL) {
            // Buscar posicion del reflejo
            size_t x, y;
            if(!buscarPosicion(actual, mapa, eje.valor, eje.posicion, x, y)){
            	return false;
			}else{
				// Buscar si esta en encontrados
            	if (!estaEnElMapa(mapa, x, y)) {
                	return false;
           		}
			}
            actual = actual->sgt;
        }
    }
    return true;
}

bool hayReflejo(Celda*& encontrados, vector<vector<Kcelda>> &mapa, vector<Eje> &ejes) {
	//cout<<"Celda actual: "<<encontrados->x<<","<<encontrados->y<<endl;	//<----- esto servia al depurar
	Kcelda celda_actual = mapa[encontrados->x][encontrados->y];
	bool encontrado = false;
	
	Reflejo* horiz = celda_actual.horizontal;
	Reflejo* verti = celda_actual.vertical;
	Pasado* pasados = encontrados->celdasPasadas;

	// HORIZONTAL 1
	while (horiz != NULL) {
		if (mapa[horiz->fil][horiz->col].valor == "1" && sinRecoger(encontrados, horiz->fil, horiz->col) && sinPasar(pasados, horiz->fil, horiz->col) && esPosible(ejes, encontrados, mapa)){
			agregarAlGrupo(encontrados, horiz->fil, horiz->col, horiz->eje, 0, ejes); // 0 indica horizontal
			encontrado = true;
			break; // Sal del bucle porque ya encontraste un reflejo
		}
		horiz = horiz->sgt;
	}
	if (encontrado) return true; // Si ya encontramos un reflejo, no necesitamos seguir

	// VERTICAL 1
	while (verti != NULL) {
		if (mapa[verti->fil][verti->col].valor == "1" && sinRecoger(encontrados, verti->fil, verti->col) && sinPasar(pasados, verti->fil, verti->col) && esPosible(ejes, encontrados, mapa)) {
			agregarAlGrupo(encontrados, verti->fil, verti->col, verti->eje, 1, ejes); // 1 indica vertical
			encontrado = true;
			break; // Sal del bucle porque ya encontraste un reflejo
		}
		verti = verti->sgt;
	}
	if (encontrado) return true; // Si ya encontramos un reflejo, no necesitamos seguir

	// Reiniciamos los punteros
	horiz = celda_actual.horizontal;
	verti = celda_actual.vertical;

	// HORIZONTAL X
	while (horiz != NULL) {
		if (mapa[horiz->fil][horiz->col].valor == "X" && sinRecoger(encontrados, horiz->fil, horiz->col) && sinPasar(pasados, horiz->fil, horiz->col) && esPosible(ejes, encontrados, mapa)) {
			agregarAlGrupo(encontrados, horiz->fil, horiz->col, horiz->eje, 0, ejes); // 0 indica horizontal
			encontrado = true;
			break; // Sal del bucle porque ya encontraste un reflejo
		}
		horiz = horiz->sgt;
	}
	if (encontrado) return true; // Si ya encontramos un reflejo, no necesitamos seguir

	// VERTICAL X
	while (verti != NULL) {
		if (mapa[verti->fil][verti->col].valor == "X" && sinRecoger(encontrados, verti->fil, verti->col) && sinPasar(pasados, verti->fil, verti->col) && esPosible(ejes, encontrados, mapa)) {
			agregarAlGrupo(encontrados, verti->fil, verti->col, verti->eje, 1, ejes); // 1 indica vertical
			encontrado = true;
			break; // Sal del bucle porque ya encontraste un reflejo
		}
		verti = verti->sgt;
	}
	return encontrado; // Devuelve si se encontro algun reflejo
}	

bool estaLaCelda(Celda*& encontrados, size_t x, size_t y){
	Celda* actual= encontrados;
	
	while(actual!=NULL){
		if(actual->x==x && actual->y==y){
			return true;
		}
		actual= actual->sgt;
	}
	return false;
}

bool todosTienenReflejo(const vector<Eje> &ejes, Celda*& encontrados, vector<vector<Kcelda>> &mapa) {
    for (const auto &eje : ejes) {
        Celda* actual = encontrados;
        while (actual != NULL) {
            // Buscar posicion del reflejo
            size_t x, y;
            if(!buscarPosicion(actual, mapa, eje.valor, eje.posicion, x, y)){
            	return false;
			}

            // Buscar si esta en encontrados
            if (!estaLaCelda(encontrados, x, y)) {
                return false;
            }
            actual = actual->sgt;
        }
    }
    return true;
}

void retrocederCelda(Celda*& encontrados, vector<Eje>& ejes) {		//quita el elemento inicial (esto es asi porque el agregar() es por el inicio)
    if (encontrados != NULL) { 
    	//Pasado* pasados= encontrados->sgt->celdasPasadas;
    	//cout<<pasados->x<<pasados->y<<endl;
        Celda* temp = encontrados;
        //cout<<temp->x<<temp->y<<endl;
        //cout<<temp->sgt->x<<temp->sgt->y<<endl;
        encontrados = temp->sgt;  
        if (encontrados != NULL) {  
            encontrados->ant = NULL;
        }
        delete temp; 
    }
    
    if (!ejes.empty()) {  
        ejes.pop_back();
    }
}

void marcarAlNuevoGrupo(vector<Buscado> &buscados, Celda*& encontrados){
	//tener la posicion de cada encontrado
	//buscarlo en el vector
	Celda* actual= encontrados;
	int tam= buscados.size();
	
	while(actual!=NULL){
		for(int i=0; i<tam; i++){
			if(buscados[i].x==actual->x  && buscados[i].y==actual->y){
				buscados[i].marcado= true;
				break;
			}
		}
		actual= actual->sgt;
	}
}

void simplificacion(vector<vector<Kcelda>>& mapa, vector<Buscado>& buscados, Grupo*& grupos) {
    if (grupos == NULL) {
        grupos = new Grupo;		//necesario
        grupos->unGrupo = NULL;  
        grupos->sgt = NULL;
    }

    Grupo* gruposK = grupos; 
    Buscado* actual = buscando1SinMarcar(buscados);  // Obtener el primer buscado sin marcar

    while (actual != NULL) {  
        //cout<< "----------------------"<<endl;
		//cout << "Procesando elemento en (" << actual->x << ", " << actual->y << ")" << endl;

        if (gruposK->unGrupo == NULL) {
            gruposK->unGrupo = NULL;  
        }
		
        vector<Eje> ejes; 
        agregarAlGrupo(gruposK->unGrupo, actual->x, actual->y, 0, -1, ejes);  
        int contCeldas = 1;
        bool huboCambios;
		
        do {
            huboCambios = false;

            while (hayReflejo(gruposK->unGrupo, mapa, ejes)) {
                contCeldas++;
                //cout<<"+"<<contCeldas<<endl;         <----- esto servia al depurar
            }
			
           
            while (!esPotencia2(contCeldas)) {
                if (contCeldas == 1) break;  // No se puede reducir mas
                retrocederCelda(gruposK->unGrupo, ejes);  // Retroceder una celda
                contCeldas--;
                //cout<<"*"<<contCeldas<<endl;   <----- esto servia al depurar
                huboCambios = true;
                break;
            }
            if(huboCambios==true){
            	continue;
			}

            // Verificar si todos tienen reflejo
            while (!todosTienenReflejo(ejes, gruposK->unGrupo, mapa)) {
                if (contCeldas == 1) break;  
                retrocederCelda(gruposK->unGrupo, ejes);
                contCeldas--;
                //cout<<"-"<<contCeldas<<endl;      <----- esto servia al depurar
                huboCambios = true;
                break;
            }
        } while (huboCambios);

        marcarAlNuevoGrupo(buscados, gruposK->unGrupo);

        // Pasar al siguiente grupo
        if (gruposK->sgt == NULL) {
            gruposK->sgt = new Grupo;  
            gruposK->sgt->unGrupo = NULL;  
            gruposK->sgt->sgt = NULL;
        }
        gruposK = gruposK->sgt;  

        actual = buscando1SinMarcar(buscados);
    }
}

void interpretar(Grupo* grupos, vector<vector<Kcelda>> &mapa, vector<string> &funciones) {		//asume que cada grupo esta unido (osea q no hay celdas aisladas, sino todos unidos)
    Grupo* actual2 = grupos;																//se asume que los valores de primeraCelda->x y primeraCelda->y son admisibles (dentro del rango)
    int j = 0;

    // Recorremos los grupos
    while (actual2 != NULL) {
        Grupo* actual = actual2;
        Celda* primeraCelda = actual->unGrupo;	//agregado* , no manejar directamente con actual->unGrupo
        if (!primeraCelda) {		//agregado *
            actual2 = actual2->sgt;
            continue;
        }

        string cadena1 = mapa[primeraCelda->x][primeraCelda->y].izq + mapa[primeraCelda->x][primeraCelda->y].der;
		
        for (size_t i = 0; i < cadena1.size(); i++) {
            bool bandera = true;

            Celda* grupoAux = primeraCelda;		
            while (grupoAux != NULL) {
                string cadena2 = mapa[grupoAux->x][grupoAux->y].izq + mapa[grupoAux->x][grupoAux->y].der;
				
                if (cadena1[i] != cadena2[i]) {
                    bandera = false;
                    break;	//agregado *
                }
                grupoAux = grupoAux->sgt; 
            }

            if (bandera) {
                funciones[j] += cadena1[i];
            } else {
                funciones[j] += "X";
            }
        }
        actual2 = actual2->sgt; 
        j++;
    }
}

void mostrarResultado(vector<string> &funciones, int a, char tipo, size_t nFF) {		//IZQUIERDA= ESTADO S0 	|  no muestra nada si sale 1
    int tam = funciones.size();
    bool huboAlguno= false;

	cout << tipo << a << ": ";
	
    for (int i = 0; i < tam; i++) {
        string cad = funciones[i];
        
        
	        for (size_t j = 0; j < cad.size(); j++) {
	            if (cad[j] != 'X') {
	            	if(j==nFF){
	            		cout<<"E";
					}else{
						cout << "Q" << j;
					}
					
	                huboAlguno= true;
	                if (cad[j] == '0') {
	                    cout << "*";
	                }
	                cout << " ";
	            }
	        }
	        
	        if(i!=tam-1){
	        	string aux= funciones[i+1];
	        	if(aux!="") cout << "+ "; 
			}        	
		
    }
    
    if(!huboAlguno){
    	cout<<"1";
	}
    cout<<endl;
}

//**********************************************************************************************************************************************************************
//TODO LO ANTERIOR AL KARNAUGHT
string numEnBinario(size_t nbits, size_t numero){
	string n="";
	size_t cociente=2;

	if(numero!=0){
		if(numero!=1){
			while(cociente>=2){
				cociente = numero/2;
				n += to_string(numero%2);
				numero= cociente;
			}
		}
		n += "1";
	}

	if(nbits>n.size()){
		int restantes= nbits-n.size();
		for(int i=0; i<restantes; i++){
			n += "0";
		}
	}
	
	string nEnBinario(n.rbegin(), n.rend());
	return nEnBinario;
}

int binarioADecimal(string numero) {
    size_t tam = numero.size();
    int decimal = 0;

    for (size_t i = tam; i > 0; i--) {
        int bit = numero[i - 1] - '0'; 
        decimal += bit * static_cast<int>(pow(2, tam - i));
    }

    return decimal;
}

string gray(string numero){
	
	int longitud= numero.size();
	string nuevoNum="";
	
	nuevoNum += numero[0];
	
	for(int i=1; i<longitud; i++){
		if(numero[i-1]==numero[i]){
			nuevoNum += "0";
		}else{
			nuevoNum += "1";
		}
	}
	return nuevoNum;
}

void armarKmapa(vector<vector<Kcelda>> &mapa, int var_izq, int var_der){
	
	for(size_t i=0; i<mapa.size(); i++){
		for(size_t j=0; j<mapa[i].size(); j++){
			mapa[i][j].izq= gray(numEnBinario(var_izq, i));
			mapa[i][j].der= gray(numEnBinario(var_der, j));
		}
	}
}

void imprimirMapa(vector<vector<Kcelda>> &mapa){
	
	cout<<endl<<"MAPA: "<<endl;
	for(size_t i=0; i<mapa.size(); i++){
		for(size_t j=0; j<mapa[i].size(); j++){
			cout<<mapa[i][j].izq<<mapa[i][j].der<<" | ";
		}
		cout<<endl;
	}
	cout<<endl;
}

void llenarMapa(Estado*& estados, vector<vector<Kcelda>> &mapa, string ff_valores, vector<Buscado> &buscados){
	
	Estado* actual= estados;
	Estado* inicio= estados;
	
	if(actual==NULL){
		cout<<"La lista Estados esta vacia!"<<endl;
		return;
	}else{
		int pos= 0;
		
		for(size_t i=0; i<mapa.size(); i++){
			for(size_t j=0; j<mapa[i].size(); j++){
				string cad= mapa[i][j].izq;		//tomo la posicion actual en el mapa (de las variables)
				cad += mapa[i][j].der;
				mapa[i][j].valor="";
				
				while(actual!=NULL){	//busco el estado q tiene esa posicion o combinacion (y mirare su id)
					if(actual->id+"0"==cad){
						mapa[i][j].valor= ff_valores[pos];
						if(ff_valores[pos]=='1'){
							Buscado nuevo;
							nuevo.x= i;
							nuevo.y= j;
							nuevo.marcado= false;
							 buscados.push_back(nuevo);
						}
						break; 
					}
					pos++;
					if(actual->id+"1"==cad){
						mapa[i][j].valor= ff_valores[pos];
						if(ff_valores[pos]=='1'){
							Buscado nuevo;
							nuevo.x= i;
							nuevo.y= j;
							nuevo.marcado= false;
							 buscados.push_back(nuevo);
						}
						break;
					}
					pos++;
					actual= actual->sgt;
				}
				if(mapa[i][j].valor==""){
				    mapa[i][j].valor="X";
				}
				pos=0;
				actual= inicio;		
			}
		}	
	}
}

void imprimirMapa2(vector<vector<vector<Kcelda>>> &mapas, int tipoFF, vector<vector<string>> &funciones,  vector<vector<Kcelda>>& mapasalida, vector<string>& salidafunciones, size_t nFF){	
	
	size_t tam= mapas.size();
	
	string tipo;
	if(tipoFF==0){
		tipo="D";
	}else if(tipoFF==1){
		tipo="T";
	}else{
		tipo="JK";
		tam= tam/2;
	}
	
	cout<<"Orden de variables: ";
	for(size_t i=0; i<=nFF; i++){
		if(i==nFF){
			cout<<"E";
		}else{
			cout<<tipo[0]<<i<<" ";
		}
	}
	cout<<endl<<endl;
	
	
	for(size_t k=0; k<tam; k++){
		cout<<"MAPA PARA "<<tipo[0]<<k<<":"<<endl;
		for(size_t i=0; i<mapas[k].size(); i++){
			for(size_t j=0; j<mapas[k][i].size(); j++){
				cout<<mapas[k][i][j].valor<<" | ";
			}
			cout<<endl;
		}
		cout<<endl;
		mostrarResultado(funciones[k], k, tipo[0], nFF);
		cout<<endl;		
	}
	
	
	if(tipoFF==2){
		cout<<"Orden de variables: ";
		for(size_t i=0; i<=nFF; i++){
			if(i==nFF){
				cout<<"E";
			}else{
				cout<<tipo[1]<<i<<" ";
			}
		}
		cout<<endl<<endl;
		
		for(size_t k=tam; k<tam*2; k++){
			cout<<"MAPA PARA "<<tipo[1]<<k-tam<<":"<<endl;
			for(size_t i=0; i<mapas[k].size(); i++){
				for(size_t j=0; j<mapas[k][i].size(); j++){
					cout<<mapas[k][i][j].valor<<" | ";
				}
				cout<<endl;
			}
			cout<<endl;	
			mostrarResultado(funciones[k], k-tam, tipo[1], nFF);
			cout<<endl;	
		}		
	}
	
	cout<<"MAPA PARA Z (salida):"<<endl;

		for(size_t i=0; i<mapasalida.size(); i++){
			for(size_t j=0; j<mapasalida[i].size(); j++){
				cout<<mapasalida[i][j].valor<<" | ";
			}
			cout<<endl;
		}
	cout<<endl;	
	mostrarResultado(salidafunciones, 1,'Z', nFF);
		
}//********************************************************************************************************************************

void encabezadoTablaTransicion(){
	gotoxy(10,4); cout<<"================================";
	gotoxy(10,5); cout<<"| Estado  |  Estado siguiente  |";
	gotoxy(10,6); cout<<"| actual  |    0    |    1     |";
	gotoxy(10,7); cout<<"================================";
	cout<<endl;
}
//*********************************************************************************************************************************
void agregarEstado(Estado* &estados, size_t nbits, size_t posicion, string cadena) {
    Estado* nuevo = new Estado();
    nuevo->sgt = NULL;
    nuevo->id = numEnBinario(nbits, posicion);
    nuevo->bits = cadena.substr(0, posicion);

    if (estados == NULL) {
    	nuevo->ant= NULL;
        estados = nuevo;
    } else {
        Estado* actual = estados;
        while (actual->sgt != NULL) {
            actual = actual->sgt;
        }
        nuevo->ant= actual;
        actual->sgt = nuevo;
    }
}

void agregarDont(Estado* &estados, size_t nbits, size_t posicion) {
    Estado* nuevo = new Estado();
    nuevo->sgt = NULL;
    nuevo->id = numEnBinario(nbits, posicion);
    nuevo->salida0= "X";
    nuevo->salida1= "X";
	
	for(size_t i=0; i<nbits; i++){
		nuevo->sgtEstadoX0 +="X"; 
    	nuevo->sgtEstadoX1 +="X";
	}

    if (estados == NULL) {
    	nuevo->ant= NULL;
        estados = nuevo;
    } else {
        Estado* actual = estados;
        while (actual->sgt != NULL) {
            actual = actual->sgt;
        }
        nuevo->ant= actual;
        actual->sgt = nuevo;
    }
}

void llenarEstados(Estado* &estados, string cadena, size_t cantidadEstados, size_t cantidadFF){
	size_t i=0;
	for(; i<cantidadEstados; i++){
		agregarEstado(estados, cantidadFF, i, cadena);	
	}
	size_t dontCare = (static_cast<size_t>(pow(2, cantidadFF))) - cantidadEstados;
	
	for(size_t j=0; j<dontCare; j++, i++){
		agregarDont(estados, cantidadFF, i);
	}
}

void mostrarIDs(Estado* estados){
	encabezadoTablaTransicion();
	Estado* actual= estados;
	
	if(actual==NULL){
		cout<<"No hay nada q mostrar"<<endl;
	}else{
		int x=8;
		while(actual!=NULL){
			if(actual->sgtEstadoX0[0] !='X'){
				gotoxy(10, x); cout<<"|   S"<<binarioADecimal(actual->id);
				gotoxy(10+10, x); cout<<"|   S"<<binarioADecimal(actual->sgtEstadoX0);
				gotoxy(10+20, x); cout<<"|   S"<<binarioADecimal(actual->sgtEstadoX1);
				gotoxy(10+30, x); cout<<" |";
				gotoxy(10, x+1); cout<<"================================";	
			}
			actual= actual->sgt;
			x=x+2;
		}
	}
}


void mostrarTablaEstados(int tam, Estado* estados, vector<string> flip1, vector<string> flip2, int tipoFF){
	Estado* actual= estados;
	
	
	gotoxy(10, 4); cout<<"=";
	for(int i=0; i<tam; i++){
		cout<<"=======";
	}
	cout<<"========";	//para la columna entrada
	
	gotoxy(10,5); cout<<"|Estado";
	gotoxy(10,6); cout<<"|actual";
	gotoxy(10+ (7*tam) , 5); cout<<"|";
	gotoxy(10+ (7*tam) , 6); cout<<"|Entrada";
	gotoxy(10+ (7*tam) , 7); cout<<"|  (E)  ";
	
	gotoxy(10+ (7*(tam+1)+1), 5); cout<<"|Estado";
	gotoxy(10+ (7*(tam+1)+1), 6); cout<<"| sgt";
	gotoxy(10+ (7*(tam+1)+1), 7); cout<<"|";
	gotoxy(10+ (7*(tam+1)+1), 8); cout<<"|";
	
	
	//PARA LA COLUMNA SIGUIENTE***************************************
	gotoxy(10+ (7*(tam+1)+2), 4); 
	for(int i=0; i<tam; i++){
		cout<<"=======";
	}
	
	gotoxy(10+ (7*(tam+1)+2), 7); 
	for(int i=0; i<tam; i++){
		cout<<"------";
		if(i==tam-1){
			cout<<"|";
		}else{
			cout<<"-";
		}
	}
	
	gotoxy(10 + (7*(tam+1)+1) + (tam*7), 5); cout<<"|";
	gotoxy(10 + (7*(tam+1)+1) + (tam*7), 6); cout<<"|";
	
	int z=0;
	for(int i=0; i<tam; i++, z=z+7){
		gotoxy(10+ (7*(tam+1)+1) + z,8); cout<<"|  Q"<<i<<"*";
	}
	cout<<" |";
	
	gotoxy(10+ (7*(tam+1)+2), 9); 
	for(int i=0; i<tam; i++){
		cout<<"=======";
	}
	
	int v=0;	//vertical
	while(actual!=NULL){
		int y=0;
		for(int i=0; i<tam; i++, y=y+7){
			gotoxy(10 + (7*(tam+1)+2) +y,10+v); cout<<"  "<<actual->sgtEstadoX0[i]<<"   |";

			gotoxy(10 + (7*(tam+1)+2) +y,10+v+1); cout<<"  "<<actual->sgtEstadoX1[i]<<"   |";

		}
		
		if(actual->sgt==NULL){
			gotoxy(10 + (7*(tam+1)+2) ,10+v+2); 
			for(int i=0; i<tam; i++){
				cout<<"=======";
			}
		}
		
		actual= actual->sgt;
		v=v+2;
	}
	
	//**********************************************************
	
	//PARA LA COLUMNA SALIDA
	gotoxy(10+ (7*(tam+1)+2) + (tam*7), 4);	cout<<"=======";	
	gotoxy(10+ (7*(tam+1)+2) + (tam*7), 6);	cout<<"Salida";
	gotoxy(10+ (7*(tam+1)+2) + (tam*7), 7);	cout<<"  (Z)";
	gotoxy(10+ (7*(tam+1)+2) + (tam*7), 9);	cout<<"=======";
	
	gotoxy(10+ (7*(tam+1)+2) + (tam*7) +6, 5);	cout<<"|";
	gotoxy(10+ (7*(tam+1)+2) + (tam*7) +6, 6);	cout<<"|";
	gotoxy(10+ (7*(tam+1)+2) + (tam*7) +6, 7);	cout<<"|";
	gotoxy(10+ (7*(tam+1)+2) + (tam*7) +6, 8);	cout<<"|";
	
	v=0;	//reiniciamos
	actual= estados;
	
	while(actual!=NULL){
		gotoxy(10+ (7*(tam+1)+2) + (tam*7), 10+v); cout<<"   "<<actual->salida0<<"  |";
		gotoxy(10+ (7*(tam+1)+2) + (tam*7), 10+v+1); cout<<"   "<<actual->salida1<<"  |";
		
		if(actual->sgt==NULL){
			gotoxy(10+ (7*(tam+1)+2) + (tam*7), 10+v+2); cout<<"=======";
		}
		actual= actual->sgt;
		v=v+2;
	}
	
	
	//**********************************************************
	
	gotoxy(10, 7); cout<<"|";
	for(int i=0; i<tam; i++){
		cout<<"------";
		if(i==tam-1){
			cout<<"|";
		}else{
			cout<<"-";
		}
	}
	
	int x=0;
	for(int i=0; i<tam; i++, x=x+7){
		gotoxy(10+x,8); cout<<"|  Q"<<i;
	}
	cout<<"  |";

	
	gotoxy(10, 9); cout<<"=";
	for(int i=0; i<tam; i++){
		cout<<"=======";
	}
	cout<<"========";	//para la columna entrada
	
	v=0;	//reinicio a los 2
	actual= estados;
	while(actual!=NULL){
		int y=0;
		for(int i=0; i<tam; i++, y=y+7){
			gotoxy(10+y,10+v); cout<<"|  "<<actual->id[i]<<"   |";
			if(i==tam-1) cout<<"   0   |";
			gotoxy(10+y,10+v+1); cout<<"|  "<<actual->id[i]<<"   |";
			if(i==tam-1)  cout<<"   1   |";
		}
		
		if(actual->sgt==NULL){
			gotoxy(10,10+v+2); cout<<"=";
			for(int i=0; i<tam; i++){
				cout<<"=======";
			}
			cout<<"========";	//para la columna entrada
		}
		
		actual= actual->sgt;
		v=v+2;
	}
	
	//PARA LOS FLIP FLOPS
	string tipo;
	if(tipoFF==0){
		tipo= "D";
	}else if(tipoFF==1){
		tipo="T";
	}else{
		tipo="JK";
	}
	
	gotoxy(10+ (7*(tam+1)+2) + (tam*7)+ 10, 7); cout<<"=";
	for(size_t i=0; i<flip1.size(); i++){
		cout<<"=======";
	}
	
	gotoxy(10+ (7*(tam+1)+2) + (tam*7)+ 10, 8); cout<<"|";
	for(size_t i=0; i<flip1.size(); i++){
		cout<<"  "<<tipo[0]<<i<<"  |";
	}
	
	gotoxy(10+ (7*(tam+1)+2) + (tam*7)+ 10, 9); cout<<"=";
	for(size_t i=0; i<flip1.size(); i++){
		cout<<"=======";
	}
	
	
	for(size_t j=0; j<flip1[0].size(); j++){
		int v=0;
		for(size_t i=0; i<flip1.size(); i++, v=v+7){
			gotoxy(10+ (7*(tam+1)+2) + (tam*7)+ 10 + v, 10+j); cout<<"|"; 
			cout<<"  "<<flip1[i][j]<<"   |";
		}
		
		if(j==flip1[0].size()-1){
			gotoxy(10+ (7*(tam+1)+2) + (tam*7)+ 10, 10+j+1); cout<<"=";
			
			for(size_t i=0; i<flip1.size(); i++){
				cout<<"=======";
			}
		}
	}
	
	//SOLO PARA EL FLIPFLOP JK
	if(tipoFF==2){
		gotoxy(10+ (7*(tam+1)+2) + (tam*7)+ 10 + (7*flip1.size())+2, 7); cout<<"=";
		for(size_t i=0; i<flip2.size(); i++){
			cout<<"=======";
		}	
		
		gotoxy(10+ (7*(tam+1)+2) + (tam*7)+ 10 + (7*flip1.size())+2, 8); cout<<"|";
		for(size_t i=0; i<flip2.size(); i++){
			cout<<"  "<<tipo[1]<<i<<"  |";
		}		
		
		gotoxy(10+ (7*(tam+1)+2) + (tam*7)+ 10 + (7*flip1.size())+2, 9); cout<<"=";
		for(size_t i=0; i<flip2.size(); i++){
			cout<<"=======";
		}	
		
		for(size_t j=0; j<flip2[0].size(); j++){
			int v=0;
			for(size_t i=0; i<flip2.size(); i++, v=v+7){
				gotoxy(10+ (7*(tam+1)+2) + (tam*7)+ 10 + (7*flip1.size())+2 + v, 10+j); cout<<"|"; 
				cout<<"  "<<flip2[i][j]<<"   |";
			}
			
			if(j==flip2[0].size()-1){
				gotoxy(10+ (7*(tam+1)+2) + (tam*7)+ 10 + (7*flip1.size())+2, 10+j+1); cout<<"=";
				
				for(size_t i=0; i<flip2.size(); i++){
					cout<<"=======";
				}
			}
		}			
	}
	
	
	
}

void evaluarSgtEstado(Estado* &estados, string secuencia, size_t cantidadEstados, string traslape, string maquina){
    Estado* actual = estados;
	
    if (actual == NULL) {
        cout << "La lista de estados ESTA VACIA" << endl;
        return;
    } else {
        size_t i = 1;
        size_t j = 0;

        while (actual != NULL && j < cantidadEstados) {
			actual->sgtEstadoX1 = "";
				
			actual->salida1="0";
            // PARA 1
            if (i <= secuencia.size()) {
            	if(!(maquina=="Mealy" && j+1==cantidadEstados)){	//aplicar solo al ultimo estado en un Mealy
            		if (actual->bits + "1" == secuencia.substr(0, i)) {
                    	if (actual->sgt != NULL) {
                        	actual->sgtEstadoX1 = actual->sgt->id;
                    	}
                	}
				}
            }
			if(actual->sgtEstadoX1 == ""){
                Estado* temp = actual;
				Estado* anterior= actual;
                string estadoGanador1 = "";

                if(j+1==cantidadEstados){		//para evaluar el valor de la SALIDA
                    if(maquina=="Moore"){
                        actual->salida1="1";
					}else{
						if(actual->bits+"1"==secuencia){
							actual->salida1="1";
						}
					}
				}
						
                size_t m = 1;
                while (temp != NULL) {	
                    if (m < actual->bits.size() && actual->bits.substr(m) + "1" == temp->bits) {
                        estadoGanador1 = temp->id;

                        if(!(j+1==cantidadEstados && (traslape=="No"))){	//condicion del traslape
                			break;
						}	
                    }

					if(m==actual->bits.size()){
						if("1"== temp->bits){
							estadoGanador1= temp->id;
						}else{
							estadoGanador1= temp->ant->id;
						}
					}
                    m++;
					anterior= temp;
                    temp = temp->ant;
                }

                if (estadoGanador1 == "") {
                    estadoGanador1 = anterior->id;
                }
                actual->sgtEstadoX1 = estadoGanador1;
            }

			// PARA 0
			actual->salida0="0";
			actual->sgtEstadoX0 = "";  // Valor por defecto

            if (i <= secuencia.size()) {
            	if(!(maquina=="Mealy" && j+1==cantidadEstados)){
	                if (actual->bits + "0" == secuencia.substr(0, i)) {
	                    if (actual->sgt != NULL) {
	                        actual->sgtEstadoX0 = actual->sgt->id;
	                    }
	                }            		
				}
            }
			if(actual->sgtEstadoX0 == ""){
                Estado* temp2 = actual;
				Estado* anterior2= actual;
                string estadoGanador0 = "";
				
				
				if(j+1==cantidadEstados){		//para evaluar el valor de la SALIDA
                    if(maquina=="Moore"){
                        actual->salida0="1";
					}else{
						if(actual->bits+"0"==secuencia){
							actual->salida0="1";
						}
					}
				}
				
                size_t k = 1;
                while (temp2 != NULL) {
                    if (k < actual->bits.size() && actual->bits.substr(k) + "0" == temp2->bits) {
                        estadoGanador0 = temp2->id;

                        if(!(j+1==cantidadEstados && (traslape=="No"))){	//condicion del traslape
                			break;
						}	
                    }

					if(k==actual->bits.size()){
						if("0"== temp2->bits){
							estadoGanador0= temp2->id;
						}else{
							estadoGanador0= temp2->ant->id;
						}
					}
                    k++;
					anterior2= temp2;
                    temp2 = temp2->ant;
                }
                
                if (estadoGanador0 == "") {
                    estadoGanador0 = anterior2->id;
                }
                actual->sgtEstadoX0 = estadoGanador0;
            }

            actual = actual->sgt;
            i++;
            j++;
        }
    }
}

void asignarFF_tipoD(Estado*& estados, vector<string> &ff, size_t cantidadFF){
	
	Estado* actual= estados;
	
	if(actual==NULL){
		cout<<"La lista Estados esta vacia!"<<endl;
		return;
	}else{
		while(actual!=NULL){
			for(size_t i=0; i<cantidadFF; i++){
				ff[i] += actual->sgtEstadoX0[i];
				ff[i] += actual->sgtEstadoX1[i];
			}
			actual= actual->sgt;
		}
	}
}

void asignarFF_tipoT(Estado*& estados, vector<string> &ff, size_t cantidadFF){
	Estado* actual= estados;
	
	if(actual==NULL){
		cout<<"La lista Estados esta vacia"<<endl;
		return;
	}else{
		while(actual!=NULL){
			for(size_t i=0; i<cantidadFF; i++){
				if(actual->id[i]==actual->sgtEstadoX0[i]){
					ff[i] += "0";
				}else{
					ff[i] += "1";
				}
				
				if(actual->id[i]==actual->sgtEstadoX1[i]){
					ff[i] += "0";
				}else{
					ff[i] += "1";
				}
			}
			actual= actual->sgt;
		}
	}
}

void asignarFF_tipoJK(Estado*& estados, vector<string> &ff, vector<string> &ff2, size_t cantidadFF){
	Estado* actual= estados;
	
	if(actual==NULL){
		cout<<"La lista Estados esta vacia"<<endl;
		return;
	}else{
		while(actual!=NULL){
			for(size_t i=0; i<cantidadFF; i++){
				//J
				if(actual->id[i]!='0'){
					ff[i] += "XX";
				}else{
					if(actual->id[i]==actual->sgtEstadoX0[i]){
						ff[i] += "0";
					}else{
						ff[i] += "1";
					}
					
					if(actual->id[i]==actual->sgtEstadoX1[i]){
						ff[i] += "0";
					}else{
						ff[i] += "1";
					}
				}
				
				//K
				if(actual->id[i]!='1'){
					ff2[i] += "XX";
				}else{
					if(actual->id[i]==actual->sgtEstadoX0[i]){
						ff2[i] += "0";
					}else{
						ff2[i] += "1";
					}
					
					if(actual->id[i]==actual->sgtEstadoX1[i]){
						ff2[i] += "0";
					}else{
						ff2[i] += "1";
					}
				}
			}
			actual= actual->sgt;
		}
	}	
}

void mostrarFF(vector<string> ff){
	int i=0;
	for(string cadena: ff){
		cout<<"FF"<<i<<": "<<cadena<<endl;
		i++;
	}
}

void agregarReflejo(Reflejo*& reflejos, int eje, int x, int y) {
    Reflejo* nuevo = new Reflejo;
    nuevo->eje = eje;
    nuevo->fil = x;
    nuevo->col = y;

    nuevo->sgt = reflejos;

    reflejos = nuevo;
}

void asignarReflejos(vector<vector<Kcelda>> &mapa, int var_izq, int var_der){
	int filas= mapa.size();
	int columnas= mapa[0].size();
	
	//HORIZONTAL
	int limite1= columnas;
	int nVarsDerecha= var_der;
	for(int i=0; i<var_der; i++){		//para cada eje
		for(int j=0; j<filas; j++){		//para cada celda
			int limite2= limite1;
			for(int k=0, cont=0; k<columnas; k++){
				if(cont==limite1){
					limite2= limite2+((int)pow(2, nVarsDerecha+1));
					cont=0;
				}
				int y= (limite2-1)-k;
				agregarReflejo(mapa[j][k].horizontal, limite1, j, y);
				cont++;
			}
		}
		limite1= limite1/2;
		nVarsDerecha--;
	}
	
	//VERTICAL
	limite1= filas;
	int nVarsIzquierda= var_izq;
	for(int i=0; i<var_izq; i++){		//para cada eje
		for(int j=0; j<columnas; j++){	//para cada celda
			int limite2= limite1;
			for(int k=0, cont=0; k<filas; k++){
				if(cont==limite1){
					limite2= limite2+ ((int)pow(2, nVarsIzquierda+1));
					cont=0;
				}
				int y= (limite2-1)-k;
				agregarReflejo(mapa[k][j].vertical, limite1, y, j);
				cont++;
			}
		}
		limite1= limite1/2;
		nVarsIzquierda--;
	}
}

void verReflejos(vector<vector<Kcelda>> &mapa){
	int filas= mapa.size();
	int columnas= mapa[0].size();
	cout<<endl<<"EJE  |  FILA  | COLUMNA"<<endl;
	
	for(int i=0; i<filas; i++){
		for(int j=0; j<columnas; j++){
			Reflejo* actual = mapa[i][j].horizontal;
			cout<<"Para ["<<i<<"]["<<j<<"]:"<<endl;
			while(actual!=NULL){
				cout<<actual->eje<<" | "<<actual->fil<<" | "<<actual->col<<endl;
				actual= actual->sgt;
			}
		}
	}
}
//********************************************************************************************************************************
//AUXILIARES

void titulo(){
    gotoxy(19,2); cout<<" =============================================== ";
    gotoxy(19,3); cout<<" =                 Detector de                 = ";
    gotoxy(19,4); cout<<" =                 Secuencias                  = ";
    gotoxy(19,5); cout<<" =============================================== ";
	cout<<endl<<endl;
}

void manejarNavegacion(char tecla, int& selectedOption, int maxOpciones) {
    if (tecla == 72) {  // Flecha arriba (ASCII 72)
        selectedOption = (selectedOption > 0) ? selectedOption - 1 : maxOpciones - 1;
    } else if (tecla == 80) {  // Flecha abajo (ASCII 80)
        selectedOption = (selectedOption < maxOpciones - 1) ? selectedOption + 1 : 0;
    }
}

int mostrarMenu(const string opciones[], int numOpciones, const string seleccionAnterior[], const string pregunta, const string categorias[]) {
    int selectedOption = 0;
    char tecla;

    do {
        system("cls");
        titulo();
        
        cout << "Seleccion actual:\n";
        for (int i = 0; i < 3; i++) {
            if (!seleccionAnterior[i].empty()) {
                cout << i + 1 << ". " <<categorias[i]<< seleccionAnterior[i] << endl;
            }
        }
        cout <<endl<< pregunta<<endl;

        
        for (int i = 0; i < numOpciones; i++) {
            if (i == selectedOption)
                cout << "> " << opciones[i] << endl; 
            else
                cout << "  " << opciones[i] << endl;
        }

        tecla = _getch(); 
        if (tecla == 72 || tecla == 80) { 
            manejarNavegacion(tecla, selectedOption, numOpciones);
        }

    } while (tecla != 13); 

    return selectedOption;
}

void opciones(int opcionActual) {
    string options[4] = {
        "1. Diagrama de Estados (tabla)",
        "2. Tabla de Estados",
        "3. Simplificacion",
        "4. Resultados Finales"
    };

    for (int i = 0; i < 4; i++) {
        if (i == opcionActual) {
            cout << " > " << options[i] << endl; 
        } else {
            cout << "   " << options[i] << endl;
        }
    }
    
    gotoxy(4, 14); cout<<"Pulse <0> para ingresar otra secuencia.";
    gotoxy(4, 15); cout<<"Pulse <Esc> para salir.";
}

int manejarOpcionesMenu(bool& salir, bool& otro){

	int opcionActual= 0;
	const int TOTAL_OPCIONES = 4;
	
	while(true){
		system("cls");
		titulo();		
		opciones(opcionActual);
		
		char tecla= _getch();
		
		if(tecla==-32 || tecla==224){		//arriba/abajo
			tecla= _getch();
			
			if(tecla==72){
				opcionActual--;
				if(opcionActual < 0) opcionActual = TOTAL_OPCIONES-1;
			}else if(tecla==80){
				opcionActual++;
				if(opcionActual >= TOTAL_OPCIONES) opcionActual= 0;
			}
			
        } else if (tecla == 27) { 
        	salir= true;
            break; 
        } else if (tecla == '0') { 
        	otro= true;
			break;

		}else if(tecla=='\r'){		
			break;
		}
	}
	
	return opcionActual;
}

void salidaString(Estado*& estados, string& salida0101){
	
	Estado* actual= estados;
	
	while(actual!=NULL){
		salida0101 += actual->salida0;
		salida0101 += actual->salida1;
		actual= actual->sgt;
	}
}

void resultadosFinales(string selecciones[4], size_t numFF, vector<vector<vector<Kcelda>>> &mapas, int tipoFF, vector<vector<string>> &funciones, vector<string> &salidafunciones, size_t nFF){
	
	cout<<endl;
	cout<<"SECUENCIA: ";
	gotoxy(11,2); cout<<selecciones[0]<<endl;
	cout<<"MAQUINA: ";
	gotoxy(11,3); cout<<selecciones[1]<<endl;
	cout<<"FLIP FLOP: ";
	gotoxy(11,4); cout<<selecciones[2]<<" ("<<numFF<<")"<<endl;
	cout<<"TRASLAPE: ";
	gotoxy(11,5); cout<<selecciones[3]<<endl<<endl;
	
	cout<<"FUNCIONES DE SALIDA:"<<endl;
	
	
	size_t tam= mapas.size();
	
	string tipo;
	if(tipoFF==0){
		tipo="D";
	}else if(tipoFF==1){
		tipo="T";
	}else{
		tipo="JK";
		tam= tam/2;
	}
	
	for(size_t k=0; k<tam; k++){
		mostrarResultado(funciones[k], k, tipo[0], nFF);
	}
	
	if(tipoFF==2){
		for(size_t k=tam; k<tam*2; k++){
			mostrarResultado(funciones[k], k-tam, tipo[1], nFF);	
		}
	}
	
	mostrarResultado(salidafunciones, 1,'Z', nFF);
}



int main(){
	
	int opcionActual= 0;
	string mensajes[3]= {"Seleccione el tipo de maquina: ", "Seleccione el tipo de FlipFlop: ", "Con traslape? "};
	string selecciones[4]= {"Secuencia: ", "Maquina: ", "FlipFlop: ", "Traslape: "};

	
	while(true){
		
		string seleccionAnterior[4] = {"", "", "", ""};
		string secuencia;
		
		system("cls");
		titulo();
		
		while (true) {
	        cout << "Ingrese la secuencia: ";
	        cin >> secuencia;
	
	        if (secuencia.length() <= 31 && secuencia.find_first_not_of("01") == string::npos) {
	            cout << "Secuencia valida: " << secuencia << endl;
	            seleccionAnterior[0] = secuencia;
	            break; 
	        } else {
	            cout << "Error: La secuencia debe tener como maximo 31 digitos y solo contener 0s y 1s"<<endl;
	        }
	    }
		
		string menu1[] = {"Moore", "Mealy"};
    	int opcion1 = mostrarMenu(menu1, 2, seleccionAnterior, mensajes[0], selecciones);
    	seleccionAnterior[1] = menu1[opcion1];
    
    	string menu2[] = {"Tipo D", "Tipo T", "Tipo JK"};
    	int opcion2 = mostrarMenu(menu2, 3, seleccionAnterior, mensajes[1], selecciones);
    	seleccionAnterior[2] = menu2[opcion2];
    
    	string menu3[] = {"Si", "No"};
    	int opcion3 = mostrarMenu(menu3, 2, seleccionAnterior, mensajes[2], selecciones);
    	seleccionAnterior[3] = menu3[opcion3];
		
		system("cls"); 
        titulo();
        
        cout << "Seleccion actual:"<<endl;
        for (int i = 0; i < 4; i++) {
            if (!seleccionAnterior[i].empty()) {
                cout << i + 1 << ". " <<selecciones[i]<< seleccionAnterior[i] << endl;
            }
        }
		
		cout<<endl<<"Bien. Procesando info..."<<endl;
		this_thread::sleep_for(chrono::seconds(2));
		
		size_t cantidadEstados;
		size_t cantidadFF;
		
		if(menu1[opcion1]=="Moore"){
			cantidadEstados = secuencia.size()+1;	//moore
		}else{
			cantidadEstados = secuencia.size();		//mealy
		}
		
		cantidadFF = static_cast<size_t>(ceil(log2(static_cast<double>(cantidadEstados))));	
		
		Estado* estados= NULL;
		llenarEstados(estados, secuencia, cantidadEstados, cantidadFF);
	
	
		evaluarSgtEstado(estados, secuencia, cantidadEstados, menu3[opcion3], menu1[opcion1]);
	
	
		vector<string> flipFlops(cantidadFF, "");
		vector<string> flipFlops2(cantidadFF, "");
	
	
		if(menu2[opcion2]=="Tipo D"){
			asignarFF_tipoD(estados, flipFlops, cantidadFF);
		}else{
			if(menu2[opcion2]=="Tipo JK"){
				asignarFF_tipoJK(estados, flipFlops, flipFlops2, cantidadFF);
			}else{
				asignarFF_tipoT(estados, flipFlops, cantidadFF);
			}
		}

		//DESDE AQUI MANEJAMOS EL MAPA_K
		int var_izq= (cantidadFF+1)/2;
		int var_der= (cantidadFF+1)-var_izq;
		
		vector<vector<Kcelda>> mapa( (int)pow(2, var_izq), vector<Kcelda>((int)pow(2, var_der)) );
		vector<vector<vector<Kcelda>>> mapas(cantidadFF, mapa);
		
		vector<vector<string>> funciones(cantidadFF);
		
		for(size_t i=0; i<cantidadFF; i++){
			
			armarKmapa(mapas[i], var_izq, var_der);
			
			vector<Buscado> buscados;
				
			llenarMapa(estados, mapas[i], flipFlops[i], buscados);
			
			asignarReflejos(mapas[i], var_izq, var_der);
			
			
			Grupo* grupos= NULL;
			
			simplificacion(mapas[i], buscados, grupos);
			
		//INICIALIZAR VECTOR FUNCIONES*******
			Grupo* temp = grupos;
		    int numGrupos = 0;
		    while (temp) {
		        numGrupos++;
		        temp = temp->sgt;
		    }
		    
		    funciones[i].resize(numGrupos, "");
		//***********************************   
		    
		    interpretar(grupos, mapas[i], funciones[i]);
			
		}
		
		
	if(opcion2 == 2){  
		for(size_t i = cantidadFF, k=0; i<cantidadFF*2; i++, k++){
			mapas.push_back(mapa);  
				
			armarKmapa(mapas[i], var_izq, var_der);
				
			vector<Buscado> buscados;
					
			llenarMapa(estados, mapas[i], flipFlops2[k], buscados);
				
			asignarReflejos(mapas[i], var_izq, var_der);
				
				
			Grupo* grupos= NULL;
				
			simplificacion(mapas[i], buscados, grupos);
				
			//INICIALIZAR VECTOR FUNCIONES*******
			Grupo* temp = grupos;
		    int numGrupos = 0;
		    while (temp) {
		        numGrupos++;
		        temp = temp->sgt;
		    }
		    
		     funciones.push_back(vector<string>(numGrupos, ""));
			//***********************************   
		    
		    interpretar(grupos, mapas[i], funciones[i]);	
		}
	}
	
	//Para la funcion de salida
	vector<vector<Kcelda>> mapasalida( (int)pow(2, var_izq), vector<Kcelda>((int)pow(2, var_der)) );
	string salida0101;
	vector<Buscado> buscados;
	Grupo* grupos= NULL;
	vector<string> salidafunciones;
	
	salidaString(estados, salida0101);
	armarKmapa(mapasalida, var_izq, var_der);
	llenarMapa(estados, mapasalida, salida0101, buscados);
	asignarReflejos(mapasalida, var_izq, var_der);
	simplificacion(mapasalida, buscados, grupos);
	
	Grupo* temp = grupos;
		int numGrupos = 0;
	while (temp) {
		numGrupos++;
		temp = temp->sgt;
	}
		    
	salidafunciones.resize(numGrupos, "");
	interpretar(grupos, mapasalida, salidafunciones);

	    
//**********************************************************************************************************************
		bool salir= false;
		bool continuar= false;		
		int opc2;

		while(true){
			system("cls");
			titulo();
			opc2=  manejarOpcionesMenu(salir, continuar);
			
			if(salir){
				break;
			}else if(continuar){
				break;
			}else{
				switch(opc2){
					case 0:{
						system("cls");
						cout<<" < Diagrama de estados (tabla) >"<<endl;		
						cout<<"Numero de estados: "<<cantidadEstados<<endl;
						cout<<"Estado inicial: S0"<<endl;
						mostrarIDs(estados);
						getch();
						break;
					}
					case 1:{
						system("cls");
						cout<<" < Tabla de estados >"<<endl;
						cout<<"Numero de FF: "<<cantidadFF<<endl;
						cout<<"Tipo de FF: "<<menu2[opcion2];
						mostrarTablaEstados(cantidadFF, estados, flipFlops, flipFlops2, opcion2);
						getch();
						break;
					}
					case 2:{
						system("cls");
						cout<<" < Simplificacion >"<<endl;
						imprimirMapa2(mapas, opcion2, funciones, mapasalida, salidafunciones, cantidadFF);	
						
						getch();
						break;
					}
					case 3:{
						system("cls");
						cout<<" < Resultados Finales >"<<endl;
						resultadosFinales(seleccionAnterior, cantidadFF, mapas, opcion2, funciones, salidafunciones, cantidadFF);
						
						getch();
						break;
					}
				}
			}
		}
	
		if(salir){
			break;
		}
	}
	
	return 0;
}
