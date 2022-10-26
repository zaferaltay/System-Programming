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
#include<signal.h>
#include <time.h>


struct {
  int row;
  int column;
  char data[2000];
}
typedef Element;

struct{
  int acceptNum;
  int whoAmI;
  Element* den;
  int addedinLine;
  FILE *f;
  
}typedef sqlCommand;

int finishedThread=0;
int createdThread=0;
Element * oneLine;
int counter=0;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond3 = PTHREAD_COND_INITIALIZER;
pthread_cond_t reader = PTHREAD_COND_INITIALIZER;
pthread_cond_t writer = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;
int freeAllThread=0;
int busy=0;
int wakeup=0;
int head=0;
int tail=0;
int poolSize = 0;


void handle_sigint()
{
    free(oneLine);
    kill(getpid(),SIGINT);
}
int writerCount=0;
int readerCOunt=0;
int waitedReaderCOunt=0;
int waitedWriterCount=0;



void* serverThreads(void *arg){

     sqlCommand* command=(sqlCommand *)arg;
     int addedinLine=0;
     int imReader=0;
     int imWriter=0;
     int aNum=0;
     int myHead=0;
     pthread_mutex_lock(&lock2); //
     int me=wakeup;
     wakeup++;
     pthread_cond_signal(&cond3);// oluştum sinyali yolla,boşa giderse gitsin uyumuyordur zaten
     pthread_mutex_unlock(&lock2); 
      

    pthread_mutex_lock(&lock);
    FILE *f=(*(command+head)).f;
    //printf("Thread #%d waiting connection\n",me);
    fprintf(f,"Thread #%d waiting connection\n",me);

  while(freeAllThread!=1){
    
    //int valread=0;

    //printf("#%d uyuyor \n", command[head]->whoAmI);
    pthread_cond_wait(&cond, &lock);
    if (freeAllThread==1 && (*(command+head)).acceptNum==-1)
    {
      break; 
    }


    aNum=(*(command+head)).acceptNum;

    addedinLine=(*(command+head)).addedinLine;
    (*(command+head)).acceptNum=-1;
    //printf("A connection has been delegated to thread id #%d\n",me);
    fprintf(f,"A connection has been delegated to thread id #%d\n",me);
    int myHead=head;
    head++;
    head%=poolSize;

    sleep(0.5);

    pthread_mutex_unlock(&lock);
//------------------------PARSE İŞLEMİNİ YAP---------------------------------------------------------------------------------------------------------------------------------
    while(1){
    		char query[1024];
    		strcpy(query,"");
    		
    		int backRecord=0;

		    read( aNum , query, 1024);
		    //printf("Thread #%d: received query ‘%s‘\n",me,query);
	    	 fprintf(f,"Thread #%d: received query ‘%s‘\n",me,query);
		    if (strcmp(query,"exit")==0)
		    {
		    	send(aNum , "exit" , 1024 , 0 );
		    	//printf("Thread #%d waiting for connection\n",me);
		    	fprintf(f,"Thread #%d waiting for connection\n",me);
		    	break;
		    }

		    Element* oneLine=(*(command+myHead)).den;
		    char id[25]={0};
		    
		    
		    char columnList[1000][1000]={0};

		    int size=strlen(query);
		    int start=0;
		    
		    int index=0;
		    int col=0;
		    while(query[start]!= ' '){
		        id[start]=query[start];
		        start++;
		    }

		    start++;    
		    
		    while(start<size){   // 1 sorguyu boşluklara ve virgüllere göre ayır   
		        while(query[start]!= ' ' && query[start]!= ',' && query[start]!= ';'  && start<size){       
		          columnList[index][col]=query[start];
		          start++;      
		          col++;
		          if (query[start]==' ' || start==size ||query[start]== ',' || query[start]== ';')
		          {      
		            columnList[index][col]='\0';
		            col=0;
		            index++;
		          }
		        }
		      start++;
		    }
		    

		    if (strcmp(columnList[0],"UPDATE")==0)
		    {
		    	pthread_mutex_lock(&lock);
		    	imWriter=1;
		    	imReader=0;


        		if (writerCount == 1 || readerCOunt > 0) {
            		++waitedWriterCount;
            		pthread_cond_wait(&writer, &lock);
            		--waitedWriterCount;
        		}
        		writerCount = 1;
	
		    }else if (strcmp(columnList[0],"SELECT")==0)
		    {	
		    	pthread_mutex_lock(&lock);
		    	imReader=1;
		    	imWriter=0;
		    	if (writerCount == 1 || waitedWriterCount > 0) {
           
            		waitedReaderCOunt++;
            		pthread_cond_wait(&reader, &lock);
            		waitedReaderCOunt--;
        		}
            		readerCOunt++;
            	pthread_mutex_unlock(&lock);	
            	pthread_cond_broadcast(&reader);
		    }


		    int columnNum=0;

		    while((*(oneLine+columnNum)).row==0){
		      columnNum++;
		    }

		  
		    index=0;
		    if (strcmp(columnList[index],"SELECT")==0) //ilk kelime select
		    {
		      index++;
		      
		      if (strcmp(columnList[index],"*")==0) //selectten sonra *    ---------------------------------------OK
		      {
		        
		        if (strcmp(columnList[index+1],"FROM")==0 && strcmp(columnList[index+2],"TABLE")==0)
		        {
		            int qz = 0;
		            char sender[1024]={0};
		            strcpy(sender,"");

		            for (int i = 0; i < addedinLine; i++) {
		                strcat(sender, (*(oneLine+i)).data);
		               qz++;
		              if (qz == columnNum ) {
		                strcat(sender,"\n");
		                qz = 0;
		                send(aNum , sender ,1024 , 0 );
		                backRecord++;
		                for (int i = 0; i < 1024; i++)
		                {
		                	sender[i]=0;
		                }
		                
		              }else{
		              	strcat(sender,",");
		              }
		            }
		            
		            
		            for (int i = 0; i < 1024; i++)
		                {
		                	sender[i]=0;
		                }
		            strcat(sender,"OK");
		            //printf("Thread #%d: query completed, %d records have been returned.\n",me,backRecord);
		            fprintf(f,"Thread #%d: query completed, %d records have been returned.\n",me,backRecord);
		            send(aNum,sender,1024,0);
		        }

		        else{
		        	char sender[1024]={0};
		            strcpy(sender,"");
		          	strcpy(sender,"Invalid Input\n");
		          	//printf("Thread #%d: query completed, %d records have been returned.\n",me,backRecord);
		          	fprintf(f,"Thread #%d: query completed, %d records have been returned.\n",me,backRecord);
		          	send(aNum,sender,1024,0);
		        }
		      }
		      else if(strcmp(columnList[index],"DISTINCT")==0){ //selectten sonra distinct
		        if (strcmp(columnList[index+1],"*")==0)  // distincten sonra yıldız --------------------------------OK
		        {
		          if (strcmp(columnList[index+2],"FROM")==0 && strcmp(columnList[index+3],"TABLE")==0)
		          {
		            int qz = 0;
		            char sender[1024]={0};
		            strcpy(sender,"");

		            for (int i = 0; i < addedinLine; i++) {
		                strcat(sender, (*(oneLine+i)).data);
		               qz++;
		              if (qz == columnNum ) {
		                strcat(sender,"\n");
		                qz = 0;
		                send(aNum , sender ,1024 , 0 );
		                backRecord++;
		                for (int i = 0; i < 1024; i++)
		                {
		                	sender[i]=0;
		                }
		                
		              }else{
		              	strcat(sender,",");
		              }
		            }

		            for (int i = 0; i < 1024; i++)
		                {
		                	sender[i]=0;
		                }
		            strcat(sender,"OK");
		            //printf("Thread #%d: query completed, %d records have been returned.\n",me,backRecord);
		            fprintf(f,"Thread #%d: query completed, %d records have been returned.\n",me,backRecord);
		            send(aNum,sender,1024,0);
		          }
		           else{
		           	char sender[1024]={0};
		            strcpy(sender,"");
		            strcpy(sender,"Incorrect Input\n");
		            //printf("Thread #%d: query completed, %d records have been returned.\n",me,backRecord);
		            fprintf(f,"Thread #%d: query completed, %d records have been returned.\n",me,backRecord);
		            send(aNum,sender,1024,0);
		            
		           }
		        }
		        else{ //----------------OK //distincten sonra kolon      
		            int *columns;
		            columns = (int *)malloc(1* sizeof(int));
		            int columnsIndex=0;
		            int tempIndex=index+1;
		            int i=0;
		            int isValid=1;
		            int pass=0;
		            while(strcmp(columnList[tempIndex],"FROM")!=0){
		              if (isValid==0)
		              {
		                pass=1;
		                //printf("The column not found\n");
		                //printf("Thread #%d: query completed, %d records have been returned.\n",me,backRecord);
		                fprintf(f,"Thread #%d: query completed, %d records have been returned.\n",me,backRecord);
		                break;
		              }
		              isValid=0;
		              while((*(oneLine+i)).row==0){

		                if (strcmp(columnList[tempIndex],(*(oneLine+i)).data)==0)
		                { 
		                  int col=(*(oneLine+i)).column;
		                  columns[columnsIndex]=col;
		                  tempIndex++;
		                  columnsIndex++;
		                  columns =(int *) realloc(columns,sizeof(int) * (columnsIndex+1));
		                  i=0;
		                  isValid=1;
		                  break;
		                }
		                i++;
		                isValid=0;
		              }

		            }
		            if (pass!=1)
		            {
		              

		              Element* forDist;
		              int indexDist=0;
		              int drow=0;
		              forDist=(Element*) malloc ((1+indexDist)*sizeof(Element));

		              char willAdd[1000]={0};
		              for(int i=0;i<columnsIndex;i++){
		                strcat(willAdd,(*(oneLine+columns[i])).data);
		                if (i+1<columnsIndex)
		                {
		                  strcat(willAdd,",");
		                }
		              }
		              int col=0;
		              strcpy(forDist[indexDist].data,willAdd);
		              strcpy(willAdd,"");
		              indexDist++;
		              drow++;

		              while((*(oneLine+col)).row==0){   
		                col++;
		              }
		              int addedIndex=0;
		              int equalFlag=0;
		              while(addedIndex<addedinLine){  // Liste bitene kadar seçili kolonalrı ekle eklediğini önceden eklendi mi diye kontrol et
		                for(int i=0;i<columnsIndex;i++){
		                  addedIndex=(drow*col)+columns[i];
		                  strcat(willAdd,(*(oneLine+addedIndex)).data);
		                  if (i+1<columnsIndex)
		                  {
		                    strcat(willAdd,",");
		                  }
		                }
		                for (int i = 0; i < indexDist; i++)
		                {
		                  if (strcmp(willAdd,forDist[i].data)==0)
		                  {
		                    equalFlag=1;
		                    break;
		                  }
		                }
		                if (equalFlag==1)
		                {
		                  equalFlag=0;
		                  drow++;
		                  addedIndex=(drow*col)+columns[0];
		                  strcpy(willAdd,"");  

		                  continue;
		                }
		                forDist=realloc(forDist,sizeof(Element )*(indexDist+1));
		                strcpy(forDist[indexDist].data,willAdd);
		                strcpy(willAdd,"");
		                indexDist++;
		                drow++;
		                addedIndex=(drow*col)+columns[0];
		              }
		              for (int i = 0; i < indexDist; ++i)
		              {	
		              	char sender[1024]={0};
		              	for (int i = 0; i < 1024; ++i)
		              	{
		              		sender[i]=0;
		              	}
		                strcpy(sender,forDist[i].data);
		                strcat(sender,"\n");
		              	send(aNum , sender , 1024 , 0 );
		              	backRecord++;

		              }
		              char sender[1024]={0};
		              	for (int i = 0; i < 1024; ++i)
		              	{
		              		sender[i]=0;
		              	}
		              	strcpy(sender,"OK");
		              	//printf("Thread #%d: query completed, %d records have been returned.\n",me,backRecord);
		              	fprintf(f,"Thread #%d: query completed, %d records have been returned.\n",me,backRecord);
		              	send(aNum , sender , 1024 , 0 );
		              free(forDist);
		            }
		            free(columns);
		          }
		      }
		      else{ //selectten sonra seçim kolonu
		            int *columns;
		            columns = (int *)malloc(1* sizeof(int));		        
		            int columnsIndex=0;
		            int tempIndex=index;
		            int i=0;
		            int isValid=1;
		            int pass=0;
		            while(strcmp(columnList[tempIndex],"FROM")!=0){
		              if (isValid==0)
		              {
		                pass=1;
		                char sender[1024]={0};
		                strcpy(sender,"");
		                strcpy(sender,"The column not found\n");
		                //printf("Thread #%d: query completed, %d records have been returned.\n",me,backRecord);
		                fprintf(f,"Thread #%d: query completed, %d records have been returned.\n",me,backRecord);
		                send(aNum , sender , 1024 , 0 );
		                break;
		              }
		              isValid=0;
		              while((*(oneLine+i)).row==0){
		        
		                if (strcmp(columnList[tempIndex],(*(oneLine+i)).data)==0)
		                { 
		                  int col=(*(oneLine+i)).column;
		                  columns[columnsIndex]=col;
		                  tempIndex++;
		                  columnsIndex++;
		                  columns =(int *) realloc(columns,sizeof(int) * (columnsIndex+1));
		                  i=0;
		                  isValid=1;
		                  break;
		                }
		                i++;
		                isValid=0;
		              }

		            }

		            if (pass!=1)
		            {
		              Element* forDist;
		              int indexDist=0;
		              int drow=0;
		              forDist=(Element*) malloc ((1+indexDist)*sizeof(Element));

		              char willAdd[1000]={0};
		              for(int i=0;i<columnsIndex;i++){
		                strcat(willAdd,(*(oneLine+columns[i])).data);
		                if (i+1<columnsIndex)
		                {
		                  strcat(willAdd,",");
		                }
		              }
		              int col=0;

		              strcpy(forDist[indexDist].data,willAdd);
		              strcpy(willAdd,"");
		              indexDist++;
		              drow++;

		              while((*(oneLine+col)).row==0){   
		                col++;
		              }
		              int addedIndex=0;
		             
		              while(addedIndex<addedinLine){  // Liste bitene kadar seçili kolonalrı ekle eklediğini önceden eklendi mi diye kontrol et
		                for(int i=0;i<columnsIndex;i++){
		                  addedIndex=(drow*col)+columns[i];
		                  strcat(willAdd,(*(oneLine+addedIndex)).data);
		                  if (i+1<columnsIndex)
		                  {
		                    strcat(willAdd,",");
		                  }
		                }
		                forDist=realloc(forDist,sizeof(Element )*(indexDist+1));
		                strcpy(forDist[indexDist].data,willAdd);
		                strcpy(willAdd,"");
		                indexDist++;
		                drow++;
		                addedIndex=(drow*col)+columns[0];
		              }


		              for (int i = 0; i < indexDist; ++i)
		              {
		              	char sender[1024]={0};
		              	for (int i = 0; i < 1024; i++)
		              	{
		              		sender[i]=0;
		              	}
		              	strcpy(sender,forDist[i].data);
		              	strcat(sender,"\n");

		                send(aNum , sender , 1024 , 0 );
		                backRecord++;
		              }
		              	char sender2[1024]={0};
		              	for (int i = 0; i < 1024; ++i)
		              	{
		              		sender2[i]=0;
		              	}
		              	strcpy(sender2,"OK");
		              	//printf("Thread #%d: query completed, %d records have been returned.\n",me,backRecord);
		              	fprintf(f,"Thread #%d: query completed, %d records have been returned.\n",me,backRecord);
		              	send(aNum , sender2 , 1024 , 0 );
		              free(forDist);
		            }
		            free(columns);
		      }

		    }
		    else if(strcmp(columnList[index],"UPDATE")==0){//ilk kelime update
		      index++;
		      int tempIndex=index+2;
		      int isValid=1;
		      int *columns;
		      columns = (int *)malloc(1* sizeof(int));
		      int pass=0;      
		      char valueList[1000][1000]={0};
		      int valueNum=0;
		      if (strcmp(columnList[index],"TABLE")==0 && strcmp(columnList[index+1],"SET")==0)
		      { 
		        //columns = (int *)malloc(1* sizeof(int));
		        int columnsIndex=0;
		          char searchedColumn[100]={0};
		          int scount=0;
		          int vcount=0;
		        while(strcmp(columnList[tempIndex],"WHERE")!=0){
		          scount=0;
		          while(columnList[tempIndex][scount]!='='){
		            searchedColumn[scount]=columnList[tempIndex][scount];
		            scount++;
		          }
		          searchedColumn[scount]='\0';
		          scount+=2;
		          vcount=0;
		          while(columnList[tempIndex][scount]!='\''){
		            valueList[valueNum][vcount]=columnList[tempIndex][scount];
		            scount++;
		            vcount++;
		          }
		          valueList[valueNum][vcount]='\0';
		          valueNum++;
		          if (isValid==0)
		              {
		                pass=1;
		                //printf("The column not found\n");
		                fprintf(f, "The column not found\n");
		                //printf("Thread #%d: query completed, %d records have been returned.\n",me,backRecord);
		                fprintf(f,"Thread #%d: query completed, %d records have been returned.\n",me,backRecord);
		                break;
		              }
		              isValid=0;
		              int i=0;
		          while((*(oneLine+i)).row==0){

		                if (strcmp(searchedColumn,(*(oneLine+i)).data)==0)
		                { 
		                  int col=(*(oneLine+i)).column;
		                  columns[columnsIndex]=col;
		                  tempIndex++;
		                  columnsIndex++;
		                  columns =(int *)realloc(columns,sizeof(int)*(columnsIndex+1));
		                  i=0;
		                  isValid=1;
		                  break;
		                }
		                i++;
		                isValid=0;
		          }
		        }
		        tempIndex++;
		        scount=0;
		        vcount=0;
		        char targetCol[100];
		        char targetVal[100];
		        while(columnList[tempIndex][scount]!='='){
		          targetCol[scount]=columnList[tempIndex][scount];
		          scount++;
		        }
		        targetCol[scount]='\0';
		        scount+=2;

		        while(columnList[tempIndex][scount]!='\''){
		          targetVal[vcount]=columnList[tempIndex][scount];
		          scount++;
		          vcount++;
		        }
		        targetVal[vcount]='\0';

		        if (pass!=1)
		            {

		              int sercount=0;
		              int drow=0;
		              int col=0;
		              while((*(oneLine+col)).row==0){   
		                col++;
		              }
		              int outflag=0;
		              int targetColIndex=0;
		              while(strcmp((*(oneLine+targetColIndex)).data,targetCol)!=0){   
		                targetColIndex++;
		                if (targetColIndex>col)
		                { 
		                  char sender[1024]={0};
		                  strcpy(sender,"");
		                  strcpy(sender,"The target not found\n");
		                  //printf("Thread #%d: query completed, %d records have been returned.\n",me,backRecord);
		                  fprintf(stderr, "Thread #%d: query completed, %d records have been returned.\n",me,backRecord );
		                  send(aNum , sender , 1024 , 0 );
		                  outflag=1;
		                  break;
		                }
		              }
		              if (outflag!=1)
		              {
		                  sercount=drow*col+targetColIndex;
		                  int affectedCount=0;
		                  while(sercount<addedinLine){

		                    if (strcmp((*(oneLine+sercount)).data,targetVal)==0)
		                    {
		                      for (int i = 0; i < columnsIndex; ++i)
		                      { 
		                        int temp=drow*col+columns[i];
		                        strcpy((*(oneLine+temp)).data,"");
		                        strcpy((*(oneLine+temp)).data,valueList[i]);
		                        affectedCount++;

		                      }
		                      int xx=0;
		                      char sender[1024]={0};
		                  	  strcpy(sender,"");
		                      while(xx<col){
		                      	strcat(sender,(*(oneLine+(drow*col+xx))).data);
		                      	xx++;
		                      	if (xx!=col)
		                      	{
		                      		strcat(sender,",");
		                      	}
		                      }
		                      strcat(sender,"\n");
		                      send(aNum , sender , 1024 , 0 );
		                      backRecord++;

		                    }
		                    drow++;
		                    sercount=drow*col+targetColIndex;
		                  }

		                  //printf("Affected count= %d \n",affectedCount );
		                  fprintf(f, "Affected count= %d \n",affectedCount );
		                  char sender2[1024]={0};
		              	  for (int i = 0; i < 1024; ++i)
		              	  {
		              		sender2[i]=0;
		              	  }
		              	  strcpy(sender2,"OK");
		              	  //printf("Thread #%d: query completed, %d records have been returned.\n",me,backRecord);
		              	  fprintf(f, "Thread #%d: query completed, %d records have been returned.\n",me,backRecord );
		              	  send(aNum , sender2 , 1024 , 0 );              
		              }
		            }
		      }
		      else{
		      	char sender[1024]={0};
		      	strcpy(sender,"");
		      	strcpy(sender,"Invalid Input\n");
		        send(aNum , sender , 1024 , 0 );
		      }
		      free(columns);    
		    }
		    else{
		      char sender[1024]={0};
		      	strcpy(sender,"");
		      	strcpy(sender,"Invalid Input\n");
		        send(aNum , sender , 1024 , 0 );
		    }
		    if (imReader==1)
		    {
		    	pthread_mutex_lock(&lock);  
		        if (--readerCOunt == 0)
		            pthread_cond_signal(&writer);
		        pthread_mutex_unlock(&lock);
		    }
		    else if(imWriter==1){
		          writerCount = 0;
		        if (waitedReaderCOunt > 0)
		            pthread_cond_signal(&reader);
		        else
		            pthread_cond_signal(&writer);
		        pthread_mutex_unlock(&lock);
		    }
		    free(oneLine);
    }

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    counter++;
    busy--;
    pthread_cond_signal(&cond2);
  }
  
  
  free((*(command+myHead)).den);
  free(command);
  finishedThread++;
  if (finishedThread==3)
  {
  	pthread_cond_signal(&cond3);
  }
  return NULL;
}



int main(int argc, char * argv[]) {

  signal(SIGINT, handle_sigint);
  FILE *f;
  int opt = 0;
  int port = 0;
  int portFlag = 0;
  char pathToLogFile[200] = {0};
  int pathFlag = 0;

  int poolflag = 0;
  char datasetPath[200] = {0};
  int datasetflag = 0;
  struct stat st;

  while ((opt = getopt(argc, argv, "p:l:o:d:")) != -1) {
    switch (opt) {
    case 'p':
      port = atoi(optarg);
      portFlag = 1;
      printf("Port num: %d\n", port);
      break;

    case 'l':
      poolSize = atoi(optarg);
      poolflag = 1;
      printf("Pool Size %d\n", poolSize);
      break;

    case 'o':
      strcpy(pathToLogFile, optarg);
      pathFlag = 1;
      printf("Path to log dile : %s \n", pathToLogFile);
      break;
    case 'd':
      strcpy(datasetPath, optarg);
      printf("Dataset path : %s\n", datasetPath);
      datasetflag = 1;
      break;

    default:
      break;

    }
  }

  if ((poolflag && portFlag && pathFlag && datasetflag) == 0) {
    perror("Invalid Command Line Input");
    exit(EXIT_FAILURE);
  }

	f = fopen(pathToLogFile, "w");
	if (f == NULL) { 
		perror("Log File Creating Error");
	}
	fprintf(f, "Executing with parameters:\n");
	fprintf(f, "-p: %d \n",port);
	fprintf(f, "-o: %s\n",pathToLogFile );
	fprintf(f, "-l: %d \n",poolSize );
	fprintf(f, "-d: %s \n",datasetPath );
	fprintf(f, "Loading dataset...");



 



//-----------------------------------------------READ AND SAVE FROM DATABASE --------------------------------------------------------------------------------------
 
  int fp = open(datasetPath, O_RDONLY);
  int readedByte = 0;
  if (!fp)
    printf("Can't open file\n");
  if (stat(datasetPath, & st) != 0) {
    perror("Input file error");
    exit(1);
  }
  int filesize = st.st_size;
  int openQuotes = 0;
  char temp[2000];
  int column = 0;
  int row = 0;
  clock_t t;
  t = clock();

  Element * oneLine;
  oneLine = (Element * ) malloc(sizeof(Element) * 1);
  int addedinLine = 0;
  

   for (int i = 0; i < 2000; ++i) {
        temp[i] = 0;
      }

  while (readedByte < filesize) {
    char c[2];
    read(fp, c, 1);
    if (strcmp(c, "\n") == 0 && openQuotes == 0)
    {

      if (row == 0) {
        //columnNum = column;
        oneLine = (Element * ) realloc(oneLine, sizeof(Element) * (addedinLine + 1));
        oneLine[addedinLine].row = row;
        oneLine[addedinLine].column = column;
        strcpy(oneLine[addedinLine].data, temp);
        addedinLine++;

        for (int i = 0; i < 2000; ++i) {
          temp[i] = 0;
        }

      } 
      else
      {

        oneLine =(Element * ) realloc(oneLine,sizeof(Element) * (addedinLine + 1));
        oneLine[addedinLine].row = row;
        oneLine[addedinLine].column = column;
        strcpy(oneLine[addedinLine].data, temp);
        addedinLine++;

        for (int i = 0; i < 2000; ++i)
        {
          temp[i] = 0;
        }

        
      }
      column=0;
      row++;
    } else if (strcmp(c, "\n") == 0 && openQuotes == 1){
      strcat(temp, "2");

    } else if (strcmp(c, ",") == 0 && openQuotes == 0) {
      int i=0;
      while(temp[i] != 0){
        i++;
      }
      temp[i]='\0';
      if (addedinLine == 0)
      {
        oneLine[0].row = row;
        oneLine[0].column = 0;
        strcpy(oneLine[0].data, temp);
        

        addedinLine++;

      } 
      else
      {
        oneLine =(Element * ) realloc(oneLine,sizeof(Element) * (addedinLine + 1));
        oneLine[addedinLine].row = row;
        oneLine[addedinLine].column = column;
        strcpy(oneLine[addedinLine].data, temp);
        addedinLine++;
      }
      for (int i = 0; i < 2000; ++i) {
        temp[i] = 0;
      }
      column++;
    }
    else if (strcmp(c, ",") == 0 && openQuotes == 1){
      strcat(temp, ",");
    }
    else if (strcmp(c, "\"") == 0) {
      if (openQuotes == 1) {
        openQuotes = 0;
      } else {
        openQuotes = 1;
      }
      //strcat(temp, c);
    } 
    else {
      strcat(temp, c);
    }
    readedByte++;
  }
  close(fp);
  //----------------------------------------------PRINT DATABASE------------------------------------------------------------------------------------------------------------------
    t = clock() - t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC;
  fprintf(f, "Dataset loaded in %f seconds with %d records. \n",time_taken,row );



 //----------------------------------CREATE THREAD POOL------------------------------------------------------------------------------------------------


  

  pthread_t myThreadPool[poolSize];      //poolsize kadar thread
  sqlCommand commands[poolSize];         //sql komutu için her threade 1 tane

  for (int i = 0; i < poolSize; i++)
  {
    
    
    commands[i].whoAmI=i;
    commands[i].den=oneLine;
    commands[i].addedinLine=addedinLine;
    commands[i].acceptNum=-1;
    commands[i].f=f;
  }
  
  for (int i = 0; i < poolSize; i++)
  {
    pthread_create(&myThreadPool[i], NULL, serverThreads, (void*)&commands[0]);  //threadleri oluşturma
    
  }
    

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt2 = 1;
    int addrlen = sizeof(address);
    
    
       
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }


    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR ,&opt2, sizeof(opt2)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( port );

  if (bind(server_fd, (struct sockaddr *)&address,sizeof(address))<0){
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

  if (listen(server_fd, 3) < 0){
        perror("listen");
        exit(EXIT_FAILURE);
    }





  pthread_mutex_lock(&lock2);    // sinyal kaybolmasın diye tüm threadlerin oluşmasını bekleme
  while(wakeup<poolSize){
    pthread_cond_wait(&cond3, &lock2);  //tüm threadşeri bekleye n koşul
  }
  pthread_mutex_unlock(&lock2);    // hepsi oluştu devam et

  
  
  while(1){
      
      if (busy<poolSize)  //eğer tüm threadler doluysa bekle
      {
        busy++;
      if ((new_socket = accept(server_fd, (struct sockaddr *)&address,(socklen_t*)&addrlen))<0)
      {
        perror("accept");
        exit(EXIT_FAILURE);
      }
        commands[tail].acceptNum=new_socket;
        createdThread++;
        tail++;
        tail%=poolSize;
        busy++;
        pthread_cond_signal(&cond); //threadler dolu değilse çalışması için sinyal yolla

      }
      else{
        fprintf(f, "No thread is available! Waiting… \n");
        pthread_cond_wait(&cond2, &lock); //threadler doluysa çalışması için sinyal bekle
        
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,(socklen_t*)&addrlen))<0)
        {
          perror("accept");
          exit(EXIT_FAILURE);
        }
        commands[tail].acceptNum=new_socket;
        createdThread++;
        tail++;
        tail%=poolSize;
        busy++;
        pthread_cond_signal(&cond);  //1 thread boşalırsa çalış,sinyal yolla
      }
    
    
  }


  

  for (int i = 0; i < poolSize; i++)
  {
  	commands[i].acceptNum=-1;
  }

 
   pthread_cond_broadcast(&cond);   //tüm threadleri serbest bırak
  

  free(oneLine);


//------------------------------------------------------------------------------------------------------------------------------------------------------------


  return 0;
}
