#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <cstdlib>
#include <time.h>
#include <vector>
#include <cctype>
#include <iostream>
#include <sstream>
#include <cstdlib>
using namespace std;
using std::cout; using std::cin;
using std::endl; using std::string;
using std::vector; using std::istringstream;
using std::stringstream;
////ESTRUCTURAS////
struct Particion
{
    char part_status;
    char part_type = 'p'; // primaria, extendida, logica
    string part_fit; //las opciones por default
    int part_start;
    int part_size;
    char part_name[16];
};
struct MBR
{
    int mbr_tamano;
    time_t mbr_fecha_creacion;
    int mbr_disk_signature;
    string disk_fit;
    Particion mbr_partition[4];
};

string path=""; //url raiz para guardar

struct DiscoD{ //estructura solo para tener la infomación de una entrada
	string path="";
	float size;
	string unit; /// K=kb M=mb
    string fit; // BF= BEST FIT FF=FIRST FIT WF=WORST FIT
}
Disk1;
void CrearDisco(DiscoD op){
cout<<"************************CREAR DISCO*************************\n"<<endl;
    string s = path + op.path;//url raiz + url de la entrada
    char sc[s.size() + 1];
    strcpy(sc, s.c_str());
    FILE *file=NULL;
    file=fopen(sc,"r"); //r= read = si el disco ya existia
    if(file!=NULL){
        cout<<"Ya existe el disco"<<endl;
        return;//error
    }
    //Tamaño para distintas unidades que se piden
    int tam;

    if(op.unit=="K"){ //kb
      tam =  op.size* 1024;
    }
    else{ //mb
        tam =  op.size* 1024 * 1024;
    }

    file=fopen(sc,"wb");
    fwrite("\0",1,1,file);
    /*se pone el puntero en el tamaño deseado del archivo y esto automaticamente 
	hace que el archivo tenga ese size*/
    fseek(file,tam,SEEK_SET); // objeto file, size de cuanto se quiere mover, al inicio
    fwrite("\0",1,1,file);
    //////ESCRITURA DEL MBR/////
    //Modificacion del struct MBR
    MBR mbr;
    mbr.mbr_tamano = tam;
    mbr.mbr_disk_signature = rand()%1000;
    mbr.mbr_fecha_creacion = time(0);
    mbr.disk_fit = op.fit;
    for(int i = 0; i < 4; i++){
        mbr.mbr_partition[i].part_status = '0';
        mbr.mbr_partition[i].part_size = 0;
        mbr.mbr_partition[i].part_fit = op.fit;
        mbr.mbr_partition[i].part_start = tam;
        strcpy(mbr.mbr_partition[i].part_name,"");
    }
    cout<<"Disco nuevo :: "<<op.path<<"\nFecha de creacion: "<<asctime(gmtime(&mbr.mbr_fecha_creacion))<<endl;
    cout<<"Signature: "<<mbr.mbr_disk_signature <<endl;
    cout<<"Tamaño: "<<mbr.mbr_tamano <<" Bytes"<<endl;
    cout<<"Fit: " <<mbr.disk_fit <<endl;
    cout<<"************************DISCO CREADO************************* \n"<<endl;
    ///escritura del mbr
    fseek(file,0,SEEK_SET);
    fwrite(&mbr,sizeof(MBR),1,file);
    fclose(file);
}

/**
 * Funcion que retorna un vector spliteado por el simbolo = para poder sacar los parametros;
 * */
vector<string> SplitEqual(string p){
    vector<string> PositionSplit{};
    stringstream input_stringstream(p);
    string l;
    while (getline(input_stringstream,l, '='))
    {
        //cout << "Un valor: " << l << endl;
        PositionSplit.push_back(l);
    }
    return PositionSplit;

}
/**
 * Funcion que recibe un string y splitea la cadena por espacios retorna un vector con lo spliteado
 * */
vector<string> SplitSpace(string text){
    vector<string> lineSplit{};
    string word;
    stringstream sstream(text);
    while (getline(sstream,word,' ')){
        lineSplit.push_back(word);
    }
    return lineSplit;
}
/**
 * Funcion utilizada para imprimir vectores spliteados(prueba nada mas)
 * */
void ImprimirVector(vector <string> vec){
    for (const auto &str : vec) {
        cout << str << endl;
    }
}
/**
 * Metodo que convierte una cadena a mayuscula y luego lo compara con la palabra 
 * */
int Comparar(char cad1[], char cad2[]){
    int i,resultado=0;
    for ( i = 0; i < strlen(cad1); i++ )  {
    cad1[i] = toupper( cad1[i] );
}
    resultado = strcmp(cad1,cad2);//comparando cadenas por ASCII si SON IGUALES RETORNA 0
    return resultado;

}
/**
 * Funcion que convierte una cadena en mayusculas todas.
 * */
string CastearMayuscula(char cad1[]){
    int i;
    for ( i = 0; i < strlen(cad1); i++ )  {
    cad1[i] = toupper( cad1[i] );
}
    return cad1;
}
/**
 * FUNCION LOGICA PRINCIPAL
 * */
int main(int argc, char const *argv[])
{
    cout<<"*********HARD DISK SIMULATION********* \n"<<endl;
    char CommandLine[200];//maximo de tamanio estimado de una linea de comandos 200
    //PALABRAS RESERVADAS
    string a;
    do
    {
        cout<<"Command :: ";
        cin.getline(CommandLine,200,'\n');//Obtenemos la linea completa del comando
        a = CastearMayuscula(CommandLine);//Casteamos todo a mayusculas para trabajarlo internamente porque pueden venir mayusculas y minusculas
        vector<string> lineSplit = SplitSpace(a);//Spliteamos por espacios
        
        if(lineSplit[0]=="MKDISK"){
            bool unit = false;
            bool fit = false;
            for(size_t i=1; i < lineSplit.size();i++){
                vector<string> aux;
                aux = SplitEqual(lineSplit[i]);
                if(aux[0] == "-PATH"){
                     Disk1.path = aux[1];
                }
                else if(aux[0]=="-F"){
                    Disk1.fit= aux[1];
                    fit = true;
                }
                else if(aux[0]=="-U"){
                     Disk1.unit = aux[1];
                     unit = true;
                }
                else if(aux[0] == "-SIZE"){
                    Disk1.size = stof(aux[1]);
                }
            }
            if(fit==false){
                Disk1.fit = "FF";
            }
            if(unit==false){
                Disk1.unit ="MB";
            }
            CrearDisco(Disk1);
        }
        if(lineSplit[0]=="RMDISK"){
            vector<string> aux;
            aux = SplitEqual(lineSplit[1]);
            if(aux[0] == "-PATH"){
                string auxiliar = aux[1];
                if(remove(auxiliar.c_str())==0) // Eliminamos el archivo
                {
                    printf("El archivo fue eliminado satisfactoriamente\n");
                }
                else{
                    printf("No se pudo eliminar el archivo\n");
                    system("PAUSE");
                }
                    
            }


           
        }
    } while (CommandLine != "CLOSE ");
    
    return EXIT_SUCCESS;
}



