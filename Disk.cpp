#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <cstdlib>
#include <time.h>
#include <vector>
#include <cctype>
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
    char part_fit = 'f'; //las opciones por default
    int part_start;
    int part_size;
    char part_name[16];
};
struct MBR
{
    int mbr_tamano;
    time_t mbr_fecha_creacion;
    int mbr_disk_signature;
    char disk_fit;
    Particion mbr_partition[4];
};

string path="url"; //url raiz para guardar

struct DiscoD{ //estructura solo para tener la infomación de una entrada
	string path="";
	int size;
	int unit; /// 1=kb 2=mb
}
Disk1;
void CrearDisco(DiscoD op){
cout<<"************************CREAR DISCO*************************"<<endl;
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

    if(op.unit==1){ //kb
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
    for(int i = 0; i < 4; i++){
        mbr.mbr_partition[i].part_status = '0';
        mbr.mbr_partition[i].part_size = 0;
        mbr.mbr_partition[i].part_fit = 'f';
        mbr.mbr_partition[i].part_start = tam;
        strcpy(mbr.mbr_partition[i].part_name,"");
    }
    cout<<"-------------DISCO CREADO--------------------"<<endl;
    cout<<"Disco\nFecha de creacion: "<<asctime(gmtime(&mbr.mbr_fecha_creacion))<<endl;
    cout<<"Signature: "<<mbr.mbr_disk_signature <<endl;
    cout<<"Tamaño: "<<mbr.mbr_tamano <<endl;
    cout<<"Fit: " <<mbr.disk_fit <<endl;
    ///escritura del mbr
    fseek(file,0,SEEK_SET);
    fwrite(&mbr,sizeof(MBR),1,file);
    fclose(file);
}
void Comands(){

}
vector<string> SplitSpace(string text){
    vector<string> lineSplit{};
    string space = " ";
    size_t pos =0;
    while ((pos = text.find(space)) != string::npos) {
        lineSplit.push_back(text.substr(0, pos));
        text.erase(0, pos + space.length());
    }
    return lineSplit;
}
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
string CastearMayuscula(char cad1[]){
    int i;
    for ( i = 0; i < strlen(cad1); i++ )  {
    cad1[i] = toupper( cad1[i] );
}
    return cad1;
}



int main(int argc, char const *argv[])
{
    cout<<"*********HARD DISK SIMULATION*********"<<endl;
    char CommandLine[200];//maximo de tamanio estimado de una linea de comandos 200
    //PALABRAS RESERVADAS
    char CLOSE[]="CLOSE";
    do
    {

        cin.getline(CommandLine,200,'\n');//Obtenemos la linea completa del comando
        string a = CastearMayuscula(CommandLine);//Casteamos todo a mayusculas para trabajarlo internamente porque pueden venir mayusculas y minusculas
        vector<string> lineSplit = SplitSpace(a);//Spliteamos por espacios
        if(lineSplit[0]=="MKDISK"){
            
        }


    } while (Comparar(CommandLine,CLOSE)!=0);
    return EXIT_SUCCESS;
}



