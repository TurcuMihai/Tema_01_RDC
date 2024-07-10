
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utmp.h>

#define client "FIFO_CLIENT"
#define server "FIFO_SERVER"

int main()

{
    int fs, fc, bytes;
    char *comanda = malloc(100), mesaj[256];
    
    if (mknod(client, S_IFIFO | 0777, 0) == -1) 
    {
        if(errno != EEXIST ) {
        printf("S:Eroare la crearea FIFO CLIENT!\n");
        exit(1); }
    }
    if (mknod(server, S_IFIFO | 0777, 0) == -1)
    {
        if(errno != EEXIST ) {
        printf("S:Eroare la crearea FIFO SERVER!\n");
        exit(1); }
    }
    printf("C: Bine ai venit!\nC: Poti consulta manualul introducand comanda 'manual'\n");

    while (1)
    {
        printf("\nC: Introduceti o comanda.\n");
        
        fgets(comanda, 100, stdin);
        
        comanda[strlen (comanda)-1] = '\0';

        if((fs = open(client, O_WRONLY)) == -1)
        {
            printf("C:Eroare la deschiderea FIFO CLIENT pentru scriere!\n");
        } 
        if((bytes = write(fs,comanda,strlen(comanda))) == -1)
        {
            
            printf("C:Eroare la scrierea in FIFO!\n");
            exit(1);
        }
     
        close(fs);

        if((fc = open(server, O_RDONLY)) == -1)
        {
            printf("C:Eroare la deschiderea FIFO CLIENT pentru citire!\n");
            exit(1);
        }
        if((bytes = read(fc,mesaj,4000)) == -1)
        {
            
            printf("C:Eroare la citirea din FIFO!\n");
            exit(1);
        }
        close(fc);
        mesaj[bytes] = '\0';
        
        if(strcmp(mesaj,"Stingere.") == 0)
        {
            exit(3);
        }
        
            printf("\n%ld\n%s\n",strlen(mesaj),mesaj); 
            
    }   
    
    return 0;
}