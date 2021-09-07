#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <cstdlib>
#include <time.h>
using namespace std;
////ESTRUCTURAS////
void crearParticionPrimaria(string, string, int, char, char, string);
void crearParticionExtendida(string, string, int, char, char, string);
void crearParticionLogica(string, string, int, char, char, string);
bool existeParticion(string, string);
bool existeParticion2(string, string);
struct Particion
{
    char part_status;
    char part_type; // primaria, extendida, logica
    char part_fit; //las opciones por default
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
struct EBR{
    char part_status; //Indica si la particion esta activa o no
    char part_fit; //Tipo de ajuste
    int part_start; //Indica en que byte del disco inicia la particion
    int part_size; //Contiene el tamano total de la particion en bytes
    int part_next; //Byte en el que esta el proxima EBR. -1 si no hay siguiente
    char part_name[16]; //Nombre de la particion
};
string path="url"; //url raiz para guardar
struct DiscoD{ //estructura solo para tener la infomación de una entrada
	string path="";
	int size;
	int unit; /// 1=kb 2=mb
}Disk2;
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
        mbr.mbr_partition[i].part_status = '0';
        mbr.mbr_partition[i].part_fit = '0';
        mbr.mbr_partition[i].part_start = -1;
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
void crearParticionPrimaria(string direccion, string nombre, int size, char fit, char unit, string archivo){
    char auxFit= 0;
    char auxUnit = 0;
    string auxPath = direccion;
    int size_bytes = 1024;
    char buffer = '1';

    if(fit != 0)
        auxFit = fit;
        
    else
        auxFit = 'W';

    if(unit != 0){
        auxUnit = unit;
        if(auxUnit == 'b'){
            size_bytes = size;
        }else if(auxUnit == 'k'){
            size_bytes = size * 1024;
        }else{
            size_bytes = size*1024*1024;
        }
    }else{
        size_bytes = size * 1024;
    }

    FILE *fp;
    MBR masterboot;
    if((fp = fopen(auxPath.c_str(), "rb+"))){
        bool flagParticion = false;//Flag para ver si hay una particion disponible
        int numParticion = 0;//Que numero de particion es
        fseek(fp,0,SEEK_SET);
        fread(&masterboot,sizeof(MBR),1,fp);
        //Verificar si existe una particion disponible
        for(int i = 0; i < 4; i++){
            
            if(masterboot.mbr_partition[i].part_start == -1)
            {
                flagParticion = true;
                numParticion = i;
                break;
            }
            if ((masterboot.mbr_partition[i].part_status == '1') && (masterboot.mbr_partition[i].part_size>=size_bytes))
            {

                flagParticion = true;
                numParticion = i;
                break;
                    
            } 
            }
        
        
        if(flagParticion){
            //Verificar el espacio libre del disco
            int espacioUsado = 0;
            for(int i = 0; i < 4; i++){
                if(masterboot.mbr_partition[i].part_status!='1'){
                    espacioUsado += masterboot.mbr_partition[i].part_size;
                }
            }
            if(archivo == "principal"){
                cout << "Espacio disponible: " << (masterboot.mbr_tamano - espacioUsado) << " Bytes" << endl;
                cout << "Espacio necesario:  " << size_bytes << " Bytes" << endl;
            }
            
            //Verificar que haya espacio suficiente para crear la particion
            if((masterboot.mbr_tamano - espacioUsado) >= size_bytes){
                if(!existeParticion(direccion,nombre)){
                    masterboot.disk_fit = auxFit;
                    if(masterboot.disk_fit == 'F'){//FIRST FIT
                        masterboot.mbr_partition[numParticion].part_type = 'P';
                        masterboot.mbr_partition[numParticion].part_fit = auxFit;
                        //start
                        if(numParticion == 0){
                            masterboot.mbr_partition[numParticion].part_start = sizeof(masterboot);
                        }else{
                            masterboot.mbr_partition[numParticion].part_start = masterboot.mbr_partition[numParticion-1].part_start + masterboot.mbr_partition[numParticion-1].part_size;
                        }
                        masterboot.mbr_partition[numParticion].part_size = size_bytes;
                        masterboot.mbr_partition[numParticion].part_status = '0';
                        strcpy(masterboot.mbr_partition[numParticion].part_name,nombre.c_str());
                        //Se guarda de nuevo el MBR
                        fseek(fp,0,SEEK_SET);
                        fwrite(&masterboot,sizeof(MBR),1,fp);
                        //Se guardan los bytes de la particion
                        fseek(fp,masterboot.mbr_partition[numParticion].part_start,SEEK_SET);
                        for(int i = 0; i < size_bytes; i++){
                            fwrite(&buffer,1,1,fp);
                        }
                        if(archivo == "principal") cout << "...\n" << "Particion primaria creada con exito" <<  endl;
                    }else if(masterboot.disk_fit == 'B'){//BEST FIT
                        int bestIndex = numParticion;
                        for(int i = 0; i < 4; i++){
                            if(masterboot.mbr_partition[i].part_start == -1 || (masterboot.mbr_partition[i].part_status == '1' && masterboot.mbr_partition[i].part_size>=size_bytes)){
                                if(i != numParticion){
                                    if(masterboot.mbr_partition[bestIndex].part_size > masterboot.mbr_partition[i].part_size){
                                        bestIndex = i;
                                        break;
                                    }
                                }
                            }
                        }
                        masterboot.mbr_partition[bestIndex].part_type = 'P';
                        masterboot.mbr_partition[bestIndex].part_fit = auxFit;
                        //start
                        if(bestIndex == 0){
                            masterboot.mbr_partition[bestIndex].part_start = sizeof(masterboot);
                        }else{
                            masterboot.mbr_partition[bestIndex].part_start = masterboot.mbr_partition[bestIndex-1].part_start + masterboot.mbr_partition[bestIndex-1].part_size;
                        }
                        masterboot.mbr_partition[bestIndex].part_size = size_bytes;
                        masterboot.mbr_partition[bestIndex].part_status = '0';
                        strcpy(masterboot.mbr_partition[bestIndex].part_name,nombre.c_str());
                        //Se guarda de nuevo el MBR
                        fseek(fp,0,SEEK_SET);
                        fwrite(&masterboot,sizeof(MBR),1,fp);
                        //Se guardan los bytes de la particion
                        fseek(fp,masterboot.mbr_partition[bestIndex].part_start,SEEK_SET);
                        for(int i = 0; i < size_bytes; i++){
                            fwrite(&buffer,1,1,fp);
                        }
                        if(archivo == "principal") cout << "...\n" << "Particion primaria creada con exito" <<  endl;
                    }else if(masterboot.disk_fit == 'W'){//WORST FIT
             
                        int  worstIndex= numParticion;
                        for(int i = 0; i < 4; i++){
                            if(masterboot.mbr_partition[i].part_start == -1 || (masterboot.mbr_partition[i].part_status == '1' && masterboot.mbr_partition[i].part_size>=size_bytes)){
                                if(i != numParticion){
                                    if(masterboot.mbr_partition[worstIndex].part_size < masterboot.mbr_partition[i].part_size){
                                        worstIndex = i;
                                        break;
                                    }
                                }
                            }
                        }
                        masterboot.mbr_partition[worstIndex].part_type = 'P';
                        masterboot.mbr_partition[worstIndex].part_fit = auxFit;
                        //start
                        if(worstIndex == 0){
                            masterboot.mbr_partition[worstIndex].part_start = sizeof(masterboot);
                        }else{
                            masterboot.mbr_partition[worstIndex].part_start = masterboot.mbr_partition[worstIndex-1].part_start + masterboot.mbr_partition[worstIndex-1].part_size;
                        }
                        masterboot.mbr_partition[worstIndex].part_size = size_bytes;
                        masterboot.mbr_partition[worstIndex].part_status = '0';
                        strcpy(masterboot.mbr_partition[worstIndex].part_name,nombre.c_str());
                        //Se guarda de nuevo el MBR
                        fseek(fp,0,SEEK_SET);
                        fwrite(&masterboot,sizeof(MBR),1,fp);
                        //Se guardan los bytes de la particion
                        fseek(fp,masterboot.mbr_partition[worstIndex].part_start,SEEK_SET);
                        for(int i = 0; i < size_bytes; i++){
                            fwrite(&buffer,1,1,fp);
                        }
                        if(archivo == "principal") cout << "...\n" << "Particion primaria creada con exito" <<  endl;
                    }
                }else{
                    cout << "ERROR ya existe una particion con ese nombre" << endl;
                }

            }else{
                cout << "ERROR la particion a crear excede el espacio libre" << endl;
            }
        }else{
            cout << "ERROR: Ya existen 4 particiones, no se puede crear otra" << endl;
            cout << "Elimine alguna para poder crear una" << endl;
        }
    fclose(fp);
    }else{
        cout << "ERROR no existe el disco" << endl;
    }
}
void crearParticionExtendida(string direccion, string nombre, int size, char fit, char unit, string archivo){
    char auxFit = 0;
    char auxUnit = 0;
    string auxPath = direccion;
    int size_bytes = 1024;
    char buffer = '1';

    if(fit != 0)
        auxFit = fit;
    else
        auxFit = 'W';

    if(unit != 0){
        auxUnit = unit;
        if(auxUnit == 'b'){
            size_bytes = size;
        }else if(auxUnit == 'k'){
            size_bytes = size * 1024;
        }else{
            size_bytes = size*1024*1024;
        }
    }else{
        size_bytes = size * 1024;
    }

    FILE *fp;
    MBR masterboot;
    if((fp = fopen(auxPath.c_str(), "rb+"))){
        bool flagParticion = false;//Flag para ver si hay una particion disponible
        bool flagExtendida = false;//Flag para ver si ya hay una particion extendida
        int numParticion = 0;//Que numero de particion es
        fseek(fp,0,SEEK_SET);
        fread(&masterboot,sizeof(MBR),1,fp);
        for(int i = 0; i < 4; i++){
            if (masterboot.mbr_partition[i].part_type == 'E'){
                flagExtendida = true;
                break;
            }
        }
        if(!flagExtendida){
            //Verificar si existe una particion disponible
            for(int i = 0; i < 4; i++){
                if(masterboot.mbr_partition[i].part_start == -1 || (masterboot.mbr_partition[i].part_status == '1' && masterboot.mbr_partition[i].part_size>=size_bytes)){
                    flagParticion = true;
                    numParticion = i;
                    break;
                }
            }
            if(flagParticion){
                //Verificar el espacio libre del disco
                int espacioUsado = 0;
                for(int i = 0; i < 4; i++){
                    if(masterboot.mbr_partition[i].part_status!='1'){
                       espacioUsado += masterboot.mbr_partition[i].part_size;
                    }
                }
                if(archivo == "principal"){
                    cout << "Espacio disponible: " << (masterboot.mbr_tamano - espacioUsado) <<" Bytes"<< endl;
                    cout << "Espacio necesario:  " << size_bytes << " Bytes" << endl;
                }
                //Verificar que haya espacio suficiente para crear la particion
                if((masterboot.mbr_tamano - espacioUsado) >= size_bytes){
                    if(!(existeParticion(direccion,nombre))){
                        masterboot.disk_fit = auxFit;
                        if(masterboot.disk_fit == 'F'){
                            masterboot.mbr_partition[numParticion].part_type = 'E';
                            masterboot.mbr_partition[numParticion].part_fit = auxFit;
                            //start
                            if(numParticion == 0){
                                masterboot.mbr_partition[numParticion].part_start = sizeof(masterboot);
                            }else{
                                masterboot.mbr_partition[numParticion].part_start =  masterboot.mbr_partition[numParticion-1].part_start + masterboot.mbr_partition[numParticion-1].part_size;
                            }
                            masterboot.mbr_partition[numParticion].part_size = size_bytes;
                            masterboot.mbr_partition[numParticion].part_status = '0';
                            strcpy(masterboot.mbr_partition[numParticion].part_name,nombre.c_str());
                            //Se guarda de nuevo el MBR
                            fseek(fp,0,SEEK_SET);
                            fwrite(&masterboot,sizeof(MBR),1,fp);
                            //Se guarda la particion extendida
                            fseek(fp, masterboot.mbr_partition[numParticion].part_start,SEEK_SET);
                            EBR extendedBoot;
                            extendedBoot.part_fit = auxFit;
                            extendedBoot.part_status = '0';
                            extendedBoot.part_start = masterboot.mbr_partition[numParticion].part_start;
                            extendedBoot.part_size = 0;
                            extendedBoot.part_next = -1;
                            strcpy(extendedBoot.part_name, "");
                            fwrite(&extendedBoot,sizeof (EBR),1,fp);
                            for(int i = 0; i < (size_bytes - (int)sizeof(EBR)); i++){
                                fwrite(&buffer,1,1,fp);
                            }
                            if(archivo == "principal") cout << "...\n" << "Particion extendida creada con exito"<< endl;
                        }else if(masterboot.disk_fit == 'B'){
                            int bestIndex = numParticion;
                            for(int i = 0; i < 4; i++){
                                if(masterboot.mbr_partition[i].part_start == -1 || (masterboot.mbr_partition[i].part_status == '1' && masterboot.mbr_partition[i].part_size>=size_bytes)){
                                    if(i != numParticion){
                                        if(masterboot.mbr_partition[bestIndex].part_size > masterboot.mbr_partition[i].part_size){
                                            bestIndex = i;
                                            break;
                                        }
                                    }
                                }
                            }
                            masterboot.mbr_partition[bestIndex].part_type = 'E';
                            masterboot.mbr_partition[bestIndex].part_fit = auxFit;
                            //start
                            if(bestIndex == 0){
                                masterboot.mbr_partition[bestIndex].part_start = sizeof(masterboot);
                            }else{
                                masterboot.mbr_partition[bestIndex].part_start =  masterboot.mbr_partition[bestIndex-1].part_start + masterboot.mbr_partition[bestIndex-1].part_size;
                            }
                            masterboot.mbr_partition[bestIndex].part_size = size_bytes;
                            masterboot.mbr_partition[bestIndex].part_status = '0';
                            strcpy(masterboot.mbr_partition[bestIndex].part_name,nombre.c_str());
                            //Se guarda de nuevo el MBR
                            fseek(fp,0,SEEK_SET);
                            fwrite(&masterboot,sizeof(MBR),1,fp);
                            //Se guarda la particion extendida
                            fseek(fp, masterboot.mbr_partition[bestIndex].part_start,SEEK_SET);
                            EBR extendedBoot;
                            extendedBoot.part_fit = auxFit;
                            extendedBoot.part_status = '0';
                            extendedBoot.part_start = masterboot.mbr_partition[bestIndex].part_start;
                            extendedBoot.part_size = 0;
                            extendedBoot.part_next = -1;
                            strcpy(extendedBoot.part_name, "");
                            fwrite(&extendedBoot,sizeof (EBR),1,fp);
                            for(int i = 0; i < (size_bytes - (int)sizeof(EBR)); i++){
                                fwrite(&buffer,1,1,fp);
                            }
                            if(archivo == "principal") cout << "...\n" << "Particion extendida creada con exito"<< endl;
                        }else if(masterboot.disk_fit == 'W'){
                            int  worstIndex= numParticion;
                            for(int i = 0; i < 4; i++){
                                if(masterboot.mbr_partition[i].part_start == -1 || (masterboot.mbr_partition[i].part_status == '1' && masterboot.mbr_partition[i].part_size>=size_bytes)){
                                    if(i != numParticion){
                                        if(masterboot.mbr_partition[worstIndex].part_size < masterboot.mbr_partition[i].part_size){
                                            worstIndex = i;
                                            break;
                                        }
                                    }
                                }
                            }
                            masterboot.mbr_partition[worstIndex].part_type = 'E';
                            masterboot.mbr_partition[worstIndex].part_fit = auxFit;
                            //start
                            if(worstIndex == 0){
                                masterboot.mbr_partition[worstIndex].part_start = sizeof(masterboot);
                            }else{
                                masterboot.mbr_partition[worstIndex].part_start =  masterboot.mbr_partition[worstIndex-1].part_start + masterboot.mbr_partition[worstIndex-1].part_size;
                            }
                            masterboot.mbr_partition[worstIndex].part_size = size_bytes;
                            masterboot.mbr_partition[worstIndex].part_status = '0';
                            strcpy(masterboot.mbr_partition[worstIndex].part_name,nombre.c_str());
                            //Se guarda de nuevo el MBR
                            fseek(fp,0,SEEK_SET);
                            fwrite(&masterboot,sizeof(MBR),1,fp);
                            //Se guarda la particion extendida
                            fseek(fp, masterboot.mbr_partition[worstIndex].part_start,SEEK_SET);
                            EBR extendedBoot;
                            extendedBoot.part_fit = auxFit;
                            extendedBoot.part_status = '0';
                            extendedBoot.part_start = masterboot.mbr_partition[worstIndex].part_start;
                            extendedBoot.part_size = 0;
                            extendedBoot.part_next = -1;
                            strcpy(extendedBoot.part_name, "");
                            fwrite(&extendedBoot,sizeof (EBR),1,fp);
                            for(int i = 0; i < (size_bytes - (int)sizeof(EBR)); i++){
                                fwrite(&buffer,1,1,fp);
                            }
                            if(archivo == "principal") cout << "...\n" << "Particion extendida creada con exito"<< endl;
                        }
                    }else{
                        cout << "ERROR ya existe una particion con ese nombre" << endl;
                    }
                }else{
                    cout << "ERROR la particion a crear excede el tamano libre" << endl;
                }
            }else{
                cout << "ERROR: Ya existen 4 particiones, no se puede crear otra" << endl;
                cout << "Elimine alguna para poder crear una" << endl;
            }
        }else{
            cout << "ERROR ya existe una particion extendida en este disco" << endl;
        }
    fclose(fp);
    }else{
        cout << "ERROR no existe el disco" << endl;
    }
}
 
   

bool existeParticion(string direccion, string nombre){cout<<"hola"<<endl;
    int extendida = -1;
    FILE *fp;
    if((fp = fopen(direccion.c_str(),"rb+"))){cout<<"Primer if de existeparticion"<<endl;
        MBR masterboot;
        fseek(fp,0,SEEK_SET);
        fread(&masterboot, sizeof(MBR),1,fp);
        for(int i = 0; i < 4; i++){
            if(strcmp(masterboot.mbr_partition[i].part_name,nombre.c_str()) == 0){
                fclose(fp);
                cout<<"Primer retorno TRUE"<<endl;
                return true;
            }else if(masterboot.mbr_partition[i].part_type == 'E'){
                extendida = i;
            }
        }
        
        if(extendida != -1){
            fseek(fp, masterboot.mbr_partition[extendida].part_start,SEEK_SET);
            EBR extendedBoot;
            cout << extendedBoot.part_next  << endl;
            cout << strcmp(extendedBoot.part_name,nombre.c_str()) << endl;
            while((fread(&extendedBoot,sizeof(EBR),1,fp))!=0 && (ftell(fp) < (masterboot.mbr_partition[extendida].part_size + masterboot.mbr_partition[extendida].part_start))){
                if(strcmp(extendedBoot.part_name,nombre.c_str()) == 0){
                  
                    fclose(fp);
                    cout<<"Segundo retorno TRUE"<<endl;
                    return true;
                }
                if(extendedBoot.part_next == -1){
                    fclose(fp);
                    cout<<"Tercer retorno FALSE"<<endl;
                    return false;
                }
            }
        }
    }
    fclose(fp);
    cout<<"Cuarto retorno FALSE"<<endl;
    return false;
}

void crearParticionLogica(string direccion, string nombre, int size, char fit, char unit, string archivo){
    char auxFit = 0;
    char auxUnit = 0;
    string auxPath = direccion;
    int size_bytes = 1024;
    char buffer = '1';

    if(fit != 0)
        auxFit = fit;
    else
        auxFit = 'W';
    if(unit != 0){
        auxUnit = unit;
        if(auxUnit == 'b'){
            size_bytes = size;
        }else if(auxUnit == 'k'){
            size_bytes = size * 1024;
        }else{
            size_bytes = size*1024*1024;
        }
    }else{
        size_bytes = size * 1024;
    }

    FILE *fp;
    MBR masterboot;
    if((fp = fopen(auxPath.c_str(), "rb+"))){
        int numExtendida = -1;
        fseek(fp,0,SEEK_SET);
        fread(&masterboot,sizeof(MBR),1,fp);
        //Verificar si existe una particion extendida
        for(int i = 0; i < 4; i++){
            if(masterboot.mbr_partition[i].part_type == 'E'){
                numExtendida = i;
                break;
            }
        }
        if(!existeParticion(direccion,nombre)){ 
            if(numExtendida != -1){
                EBR extendedBoot;
                int cont = masterboot.mbr_partition[numExtendida].part_start;
                fseek(fp,cont,SEEK_SET);
                fread(&extendedBoot, sizeof(EBR),1,fp);
                if(extendedBoot.part_size == 0){//Si es la primera
                    if(masterboot.mbr_partition[numExtendida].part_size < size_bytes){
                        if(archivo == "principal") cout << "ERROR la particion logica a crear excede el espacio disponible de la particion extendida " << endl;
                    }else{
                        extendedBoot.part_status = '0';
                        extendedBoot.part_fit = auxFit;
                        extendedBoot.part_start = ftell(fp) - sizeof(EBR); //Para regresar al inicio de la extendida
                        extendedBoot.part_size = size_bytes;
                        extendedBoot.part_next = -1;
                        strcpy(extendedBoot.part_name, nombre.c_str());
                        fseek(fp, masterboot.mbr_partition[numExtendida].part_start ,SEEK_SET);
                        fwrite(&extendedBoot,sizeof(EBR),1,fp);
                        if(archivo == "principal") cout << "...\nParticion logica creada con exito "<< endl;
                    }
                }else{
                    while((extendedBoot.part_next != -1) && (ftell(fp) < (masterboot.mbr_partition[numExtendida].part_size + masterboot.mbr_partition[numExtendida].part_start))){
                        fseek(fp,extendedBoot.part_next,SEEK_SET);
                        fread(&extendedBoot,sizeof(EBR),1,fp);
                    }
                    int espacioNecesario = extendedBoot.part_start + extendedBoot.part_size + size_bytes;
                    if(espacioNecesario <= (masterboot.mbr_partition[numExtendida].part_size + masterboot.mbr_partition[numExtendida].part_start)){
                        extendedBoot.part_next = extendedBoot.part_start + extendedBoot.part_size;
                        //Escribimos el next del ultimo EBR
                        fseek(fp,ftell(fp) - sizeof (EBR),SEEK_SET);
                        fwrite(&extendedBoot, sizeof(EBR),1 ,fp);
                        //Escribimos el nuevo EBR
                        fseek(fp,extendedBoot.part_start + extendedBoot.part_size, SEEK_SET);
                        extendedBoot.part_status = 0;
                        extendedBoot.part_fit = auxFit;
                        extendedBoot.part_start = ftell(fp);
                        extendedBoot.part_size = size_bytes;
                        extendedBoot.part_next = -1;
                        strcpy(extendedBoot.part_name,nombre.c_str());
                        fwrite(&extendedBoot,sizeof(EBR),1,fp);
                        if(archivo == "principal") cout << "Particion logica creada con exito "<< endl;
                    }else{
                        cout << "ERROR la particion logica a crear excede el" << endl;
                        cout << "espacio disponible de la particion extendida" << endl;
                    }
                }
            }else{
                cout << "ERROR se necesita una particion extendida donde guardar la logica " << endl;
            }
        }else{
            cout << "ERROR ya existe una particion con ese nombre" << endl;
        }

    fclose(fp);
    }else{
        cout << "ERROR no existe el disco" << endl;
    }

}

int main(){




    //crearParticionPrimaria("urlnuevo.dk", "part3", 2, 'B', 'b', "principal");
   
    /*
    */
    //crearParticionPrimaria("urlPruebaParticion.dk", "parti", 50, 'B', 'b', "principal");
    //crearParticionExtendida("urlPruebaParticion.dk", "Extendida1", 200, 'W', 'b', "principal");
    crearParticionLogica("urlPruebaParticion.dk", "logic2", 50,'W', 'b', "principal");
    
/*
    Disk2.path="PruebaParticion.dk";

    Disk2.size=15;
    Disk2.unit=1;

    CrearDisco(Disk2);*/
    return 0;
}