#include<stdio.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/stat.h>
#include<time.h>
#include <fcntl.h>
#include <semaphore.h>
#include<errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

   
int main(int argc, char *argv[])
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    
    struct stat st;
    int opt = 0;
    int port = 0;
    int id = 0;
    char pathToQueryFile[200] = {0};
    char adress[200] = {0};

    clock_t t;
    

  while ((opt = getopt(argc, argv, "i:a:o:p:")) != -1) {
    switch (opt) {
    case 'p':
      port = atoi(optarg);
      break;

    case 'i':
      id = atoi(optarg);
      break;

    case 'o':
      strcpy(pathToQueryFile, optarg);
      printf("Path to log dile : %s \n", pathToQueryFile);
      break;
    case 'a':
      strcpy(adress, optarg);
      printf("Dataset path : %s\n", adress);
      break;

    default:
      break;

    }
  }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
   
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
       
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, adress, &serv_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

        
        int fp = open(pathToQueryFile, O_RDONLY);
        int readedByte = 0;
        if (!fp)
            printf("Can't open file\n");
        if (stat(pathToQueryFile, & st) != 0) {
            perror("Input file error");
            exit(1);
        }
    

 
    time_t mytime = time(NULL);
    char * time_str = ctime(&mytime);
    time_str[strlen(time_str)-1] = '\0';
    printf("[%s] Client-%d connecting to %s:%d\n",time_str,id,adress,port );


    char sendedFile[1024]={0};
    for (int i = 0; i < 1024; ++i)
    {
        sendedFile[i]=0;
    }
    
    char c[2];
    
    int fileSize=st.st_size;
    
    while(1){
        read(fp, c, 1);
        readedByte++;
        
        while(strcmp(c,"\n")!=0 && strcmp(c,"\0")!=0){
            
            strcat(sendedFile,c);
            strcpy(c,"");
            read(fp, c, 1);
            readedByte++;
        }
        strcat(sendedFile,c);
        time_str = ctime(&mytime);
        time_str[strlen(time_str)-1] = '\0';
        printf("[%s] Client-%d connected and sending query  '%s'\n",time_str,id,sendedFile);
        t = clock();
        send(sock , sendedFile , 1024 , 0 );
        strcpy(sendedFile,"");
        int totalRecord=0;
        while(1){
            char buffer[1024] = {0};
            for (int i = 0; i < 1024; i++)
            {
                buffer[i]=0;
            }

            read( sock , buffer, 1024);

            if (strcmp(buffer,"OK")!=0)
            {
                printf("%s",buffer );
                strcpy(buffer,"");
                totalRecord++;
            }
            else{
                t = clock() - t;
                double time_taken = ((double)t)/CLOCKS_PER_SEC;
                time_str = ctime(&mytime);
                time_str[strlen(time_str)-1] = '\0';
                printf("[%s] Server’s response to Client-%d is %d records, and arrived in %f seconds.\n",time_str,id,totalRecord,time_taken);
                totalRecord=0;
                break;
            }

        }
        if (readedByte==fileSize)
        {   
            char buffer[1024] = {0};
            for (int i = 0; i < 1024; i++)
            {
                buffer[i]=0;
            }
            for (int i = 0; i < 1024; i++)
            {
                sendedFile[i]=0;
            }
            
            strcpy(sendedFile,"exit");
            send(sock , sendedFile , 1024 , 0 );
            read( sock , buffer, 1024);
            if (strcmp(buffer,"exit")==0)
            {
                break;
            }
        }
    }

    

    return 0;
        
}