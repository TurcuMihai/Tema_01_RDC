#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utmp.h>
#include <sys/socket.h>
#include <sys/wait.h>

#define client "FIFO_CLIENT"
#define server "FIFO_SERVER"

int main()

{
    struct utmp *data;
    pid_t pid, waitp;
    int fs, fc, fis, bytes, rv, connected = 0, index = 0;
    char comanda[100], temp1[100], temp2[100], username[100], mesaj[4000], val[100], line[100], pidul[100];

    if (mknod(client, S_IFIFO | 0777, 0) == -1)
    {
        if (errno != EEXIST)
        {
            printf("S:Eroare la crearea FIFO CLIENT!\n");
            exit(1);
        }
    }
    if (mknod(server, S_IFIFO | 0777, 0) == -1)
    {
        if (errno != EEXIST)
        {
            printf("S:Eroare la crearea FIFO SERVER!\n");
            exit(1);
        }
    }

    printf("Server pornit\n");

    while (1)
    {
        printf("[%d] Astept urmatoarea comanda.\n",index);
        index++;
        if ((fc = open(client, O_RDONLY)) == -1)
        {
            printf("S:Eroare la deschiderea FIFO CLIENT pentru citire!\n");
            exit(1);
        }

        if ((bytes = read(fc, comanda, 100)) == -1)
        {
            printf("S:Eroare la citirea comenzii din FIFO!\n");
        }

        comanda[bytes] = '\0';

        strncpy(temp1, comanda, 8);
        strncpy(temp2,comanda, 16);       // pregatesc mediul
        temp1[8] = '\0';
        temp2[16] = '\0';
        
        if (strcmp(comanda, "logout") == 0)                                                       // DACA COMANDA E LOGOUT
        {

            if ((fs = open(server, O_WRONLY)) == -1)
            {
                printf("S:Eroare la deschiderea FIFO SERVER pentru scriere!\n");
                exit(1);
            }
            if (connected == 1)
            {
                connected = 0;
                if ((bytes = write(fs, "Deconectare reusita.", strlen("Deconectare reusita."))) == -1)
                {
                    printf("S:Eroare la scrierea in FIFO!\n");
                    exit(1);
                }
            }
            else
            {
                if ((bytes = write(fs, "Nu este niciun utilizator conectat in acest moment.", strlen("Nu este niciun utilizator conectat in acest moment."))) == -1)
                {
                    printf("S:Eroare la scrierea in FIFO!\n");
                    exit(1);
                }
            }
            close(fs);
        }
        else 
            if (strcmp(comanda, "quit") == 0)                                                 // DACA COMANDA E QUIT
        {
            if ((fs = open(server, O_WRONLY)) == -1)
            {
                printf("S:Eroare la deschiderea FIFO SERVER pentru scriere!\n");
                exit(1);
            }
            connected = 0;
            if ((bytes = write(fs, "Stingere.", strlen("Stingere."))) == -1)
            {
                printf("S:Eroare la scrierea in FIFO!\n");
                exit(1);
            }
            close(fs);
        }
            else
                if (strcmp(temp1, "login : ") == 0)                                                      //// DACA COMANDA E LOGIN
              {
                
                if (connected == 0)
                {
                    strcpy(username, comanda + 8);

                    int login[2];
                    int gasit = 0;

                    if (pipe(login) == -1)
                    {
                        printf("Eroare la pipe!\n");
                        exit(1);
                    }
                    if ((pid = fork()) == -1)
                    {
                        printf("Eroare la fork!\n");
                        exit(1);
                    }
                    if (pid == 0) /// COPIL
                    {
                        close(login[0]);
                        char line[100];
                        FILE *file = fopen("users.txt", "r");
                        while (fgets(line, sizeof(line), file))
                        {
                            line[strlen(line) - 1] = '\0';
                            if (strcmp(line, username) == 0)
                            {
                                if (write(login[1], "Utilizator gasit.", strlen("Utilizator gasit.")) == -1)
                                {
                                    printf("Eroare la scrierea in pipe!\n");
                                    exit(1);
                                }
                                gasit = 1;
                            }
                        }
                        if (gasit == 0)
                        {
                            if (write(login[1], "Utilizator negasit.", strlen("Utilizator negasit.")) == -1)
                            {
                                printf("Eroare la scrierea in pipe!\n");
                                exit(1);
                            }
                        }
                        fclose(file);
                        close(login[1]);
                        exit(1); // omor procesul copil dupa ce acesta a scris in pipe informatia;
                    }
                    else
                    { // PARINTE
                        close(login[1]);
                        if ((bytes = read(login[0], mesaj, 100)) == -1)
                        {
                            printf("Eroare la citirea din pipe!\n");
                            exit(1);
                        }
                        mesaj[bytes] = '\0';
                        if (strcmp(mesaj, "Utilizator gasit.") == 0)
                        {
                            connected = 1;

                            if ((fs = open(server, O_WRONLY)) == -1)
                            {
                                printf("S:Eroare la deschiderea FIFO SERVER pentru scriere!\n");
                                exit(1);
                            }
                            if ((bytes = write(fs, "Conectare reusita.", strlen("Conectare reusita."))) == -1)
                            {
                                printf("S:Eroare la scrierea in FIFO!\n");
                                exit(1);
                            }
                            close(fs);
                        }
                        else
                        {
                            if ((fs = open(server, O_WRONLY)) == -1)
                            {
                                printf("S:Eroare la deschiderea FIFO SERVER pentru scriere!\n");
                                exit(1);
                            }
                            if ((bytes = write(fs, "Utilizator invalid.", strlen("Utilizator invalid."))) == -1)
                            {
                                printf("S:Eroare la scrierea in FIFO!\n");
                                exit(1);
                            }
                            close(fs);
                        }

                        close(login[0]);
                    }
                }
                else
                {
                    if ((fs = open(server, O_WRONLY)) == -1)
                    {
                        printf("S:Eroare la deschiderea FIFO SERVER pentru scriere!\n");
                        exit(1);
                    }
                    if ((bytes = write(fs, "Alt utilizator este deja conectat.", strlen("Alt utilizator este deja conectat."))) == -1)
                    {
                        printf("S:Eroare la scrierea in FIFO!\n");
                        exit(1);
                    }
                    close(fs);
                }
            }
             else                                                                /// DE AICI
                if (strcmp(comanda, "get-logged-users") == 0)
                {
                if (connected == 1)
                {                                                                    // daca esti sau nu esti conectat
                    int logged[2];
                    if (socketpair(AF_UNIX, SOCK_STREAM, 0, logged) == -1)
                    {                     
                        perror("Eroare la crearea socketpair.\n");
                        exit(1);
                    }
                    if ((pid = fork()) == -1)
                    {
                        printf("Eroare la fork!\n");
                        exit(1);
                    }
                    if (pid == 0)
                    {
                        close(logged[0]);

                        char aux[50];
                        data = getutent();
                        char ut_user[100] = "ut_user: ", ut_host[100] = "ut_host: ", ut_sec[100] = "ut_tv->ut_sec: ", ut_usec[100] = "ut_tv->ut_usec: ";
                        while (data != NULL)
                        {
                            char user[100], host[100], sec[100], usec[100];
                            // strncpy(user,data->ut_user,strlen(*(data->ut_user)));
                            // strncpy(host,data->ut_host,strlen(*(data->ut_host)));
                            strncpy(user, data->ut_user, 32);
                            user[32] = '\0';
                            strncpy(host, data->ut_host, 32);
                            host[32] = '\0';
                            strcat(ut_user, user);
                            strcat(ut_host, host);
                            sprintf(sec, "%d", data->ut_tv.tv_sec);
                            sprintf(usec, "%d", data->ut_tv.tv_sec);
                            strcat(ut_sec, sec);
                            strcat(ut_usec, usec);

                            strcat(ut_user, "\n");
                            strcat(ut_host, "\n");
                            strcat(ut_sec, "\n");
                            strcat(ut_usec, "\n");

                            if (write(logged[1], ut_user, strlen(ut_user)) == -1)
                            {
                                printf("Eroare la scrierea in fisier!\n");
                                exit(1);
                            }
                            if (write(logged[1], ut_host, strlen(ut_host)) == -1)
                            {
                                printf("Eroare la scrierea in fisier!\n");
                                exit(1);
                            }
                            if (write(logged[1], ut_sec, strlen(ut_sec)) == -1)
                            {
                                printf("Eroare la scrierea in fisier!\n");
                                exit(1);
                            }
                            if (write(logged[1], ut_usec, strlen(ut_usec)) == -1)
                            {
                                printf("Eroare la scrierea in fisier!\n");
                                exit(1);
                            }
                            if (write(logged[1], "\n", 1) == -1)
                            {
                                printf("Eroare la scrierea in fisier!\n");
                                exit(1);
                            }

                            ut_user[9] = '\0';
                            ut_host[9] = '\0';
                            ut_sec[15] = '\0';
                            ut_usec[16] = '\0';

                            data = getutent();
                        }

                        close(logged[1]);
                        exit(10);
                    }
                    else
                    {
                      //  waitp = wait(NULL);
                        sleep(1);
                        close(logged[1]);
                        if ((bytes = read(logged[0], mesaj, 4000)) == -1)
                        {
                            printf("Eroare la scrierea in pipe!\n");
                            exit(1);
                        }
                        mesaj[bytes] = '\0';

                        if ((fs = open(server, O_WRONLY)) == -1)
                        {
                            printf("S:Eroare la deschiderea FIFO SERVER pentru scriere!\n");
                            exit(1);
                        }
                        if ((bytes = write(fs, mesaj, strlen(mesaj))) == -1)
                        {
                            printf("S:Eroare la scrierea in FIFO!\n");
                            exit(1);
                        }
                        close(fs);
                        close(logged[0]);
                    }
                }
                else
                {
                    if ((fs = open(server, O_WRONLY)) == -1)
                    {
                        printf("S:Eroare la deschiderea FIFO SERVER pentru scriere!\n");
                        exit(1);
                    }
                    if ((bytes = write(fs, "Nu sunteti conectat.", strlen("Nu sunteti conectat."))) == -1)
                    {
                        printf("S:Eroare la scrierea in FIFO!\n");
                        exit(1);
                    }
                    close(fs);
                }
              }                                                                         //// PANA AICI 
              else 
                if(strcmp(temp2,"get-proc-info : ") == 0)
                {
                    if(connected == 1)
                    {
                        int proc[2];
                        strcpy(val, comanda + 16 );
                        if(socketpair(AF_UNIX, SOCK_STREAM, 0, proc) == -1)
                        {
                            printf("Eroare la creara socketpair.\n");
                            exit(1);
                        }
                        if((pid = fork()) == -1)
                        {
                            printf("Eroare la fork.\n");
                            exit(1);
                        }
                        if(pid == 0)
                        {
                            close(proc[0]);
                            if((bytes = read(proc[1], pidul,100)) == -1)
                            {
                                
                            }
                            pidul[bytes] = '\0';
                            char add[200]="/proc/";
                            strcat(add,pidul);
                            
                            strcat(add,"/status");
                            //printf("%s %ld\n",add,strlen(add));
                            FILE *fd;
                            int exist;
                            fd = fopen(add,"r");
                            if(open(add,O_RDONLY) == -1) 
                            {
                                if ((bytes = write(proc[1], "Proces inexistent.", strlen("Proces inexistent."))) == -1)
                                    {
                                        printf("Eroare la scrierea in socketpair!\n");
                                        exit(1);
                                    }
                                    exit(1);
                            }
                           // printf("%d\n", fd);
                            bytes = 0;
                            while (fgets(line, sizeof(line), fd))
                            {
                                
                               // printf("%s\n", line);
                               // line[strlen(line)] = '\0';                       // trebuie -1 ?
                                if (strstr(line, "Name:"))
                                {
                                    //printf("%s\n", line);
                                    if ((bytes = write(proc[1], line, strlen(line))) == -1)
                                    {
                                        printf("Eroare la scrierea in socketpair!\n");
                                        exit(1);
                                    }
                                }
                                if (strstr(line, "State:"))
                                {
                                    //printf("%s\n", line);
                                    if ((bytes = write(proc[1], line, strlen(line))) == -1)
                                    {
                                        printf("Eroare la scrierea in socketpair!\n");
                                        exit(1);
                                    }
                                }
                                if (strstr(line, "PPid:"))
                                {
                                  //printf("%s\n", line);
                                    if ((bytes = write(proc[1], line, strlen(line))) == -1)
                                    {
                                        printf("Eroare la scrierea in socketpair!\n");
                                        exit(1);
                                    }
                                }
                                if (strstr(line, "Uid:"))
                                {
                                     //printf("%s\n", line);
                                    if ((bytes = write(proc[1], line, strlen(line))) == -1)
                                    {
                                        printf("Eroare la scrierea in socketpair!\n");
                                        exit(1);
                                    }
                                }
                                if (strstr(line, "VmSize:"))
                                {
                                    //printf("%s\n", line);
                                    if ((bytes = write(proc[1], line, strlen(line))) == -1)
                                    {
                                        printf("Eroare la scrierea in socketpair!\n");
                                        exit(1);
                                    }
                                }
                            }
                           
                            fclose(fd);
                            close(proc[1]);
                            exit(1);
                        }
                        else
                        {
                            close(proc[1]);
                            strcpy(val, comanda + 16);
                            
                            if((bytes = write(proc[0],val,strlen(val))) == -1)
                            {
                                printf("Eroare la scrierea in socketpair\n");
                                exit(1);
                            }
                            sleep(1);
                              
                            if((bytes = read(proc[0], mesaj, 4000)) == -1)
                            {
                                printf("Eroare la citirea din socketpair!\n");
                                exit(1);
                            }
                            mesaj[bytes] = '\0';
                            
                            if ((fs = open(server, O_WRONLY)) == -1)
                            {
                                printf("S:Eroare la deschiderea FIFO SERVER pentru scriere!\n");
                                exit(1);
                            }
                            if ((bytes = write(fs, mesaj, strlen(mesaj))) == -1)
                            {
                                printf("S:Eroare la scrierea in FIFO!\n");
                                exit(1);
                            }
                                close(fs);
                            }
                           
                                close(proc[0]);
                        }
                    
                    else
                    {
                        if ((fs = open(server, O_WRONLY)) == -1)
                        {
                            printf("S:Eroare la deschiderea FIFO SERVER pentru scriere!\n");
                            exit(1);
                        }
                        if ((bytes = write(fs, "Nu sunteti conectat.", strlen("Nu sunteti conectat."))) == -1)
                        {
                            printf("S:Eroare la scrierea in FIFO!\n");
                            exit(1);
                        }
                        close(fs);
                    }
    
                }
                
                    else
                    if(strcmp(comanda,"manual") == 0)
                    {
                         if ((fs = open(server, O_WRONLY)) == -1)
                        {
                            printf("S:Eroare la deschiderea FIFO SERVER pentru scriere!\n");
                            exit(1);
                        }
                        if ((bytes = write(fs, "1. login : <nume_utilizator>\n2. get-logged-users\n3. get-proc-info : <pid>\n4. logout\n5. quit\n Va rugam sa respectati formatul!\n", strlen("1. login : <nume_utilizator>\n2. get-logged-users\n3. get-proc-info : <pid>\n4. logout\n5.quit\n Va rugam sa respectati formatul!\n"))) == -1)
                        {
                            printf("S:Eroare la scrierea in FIFO!\n");
                            exit(1);
                        }
                    }
                    else
                    {
                    if ((fs = open(server, O_WRONLY)) == -1)
                        {
                            printf("S:Eroare la deschiderea FIFO SERVER pentru scriere!\n");
                            exit(1);
                        }
                        if ((bytes = write(fs, "Comanda necunoscuta.", strlen("Comanda necunoscuta."))) == -1)
                        {
                            printf("S:Eroare la scrierea in FIFO!\n");
                            exit(1);
                        }
                }                                                                            
            
        close(fc);
    }
    return 0;
}

      