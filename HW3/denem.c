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

#define N 6
#define SHAREDKEY 1909

struct
{
	int patatoId[N];
	int patatoSwitch[N];
	char fifosnames[N][100];
	int totalPatato;	
}typedef Shared;

int main(int argc, char *argv[])
{
	int opt=0;
	char shmem[30]={0};
	char fname[30]={0};
	char semapname[50];
	int byte=0;
	char buffer[1000];
	char tempbuf[1000][100]={0};
	int fd=0;
	int random=0;
	struct stat st;
	int sz;
	int me=0;
	srand(time(NULL));
	
	
//--------------------------------------------------------------------------------------------------
    	

//--------------------------------------------------------------------------------------------------

	int shmid= shmget(SHAREDKEY,100000000,0666|IPC_CREAT);

	while((opt = getopt(argc, argv, "f:b:s:m:"))!=-1)  // Hepsi girildi mi kontrol et
	{ 
        switch(opt) 
        { 
		    case 'b':
		    byte=atoi(optarg);
		    //printf("Total patato %d\n",byte );
		    break;

		    case 's':
        	strcpy(shmem,optarg);
		   // printf("Shared Memory: %s \n",shmem);
		    break;

		    case 'f':         
		    strcpy(fname,optarg);
		   // printf("Fifos Name: %s \n",fname);
		    break;

		    case 'm':
		    strcpy(semapname,optarg);
		    break;

		    default :
		    break;
	    } 
	}

	sem_t *sem; 		//SEMAFOR
    sem=sem_open (semapname, O_CREAT, 0644, 1);

	Shared *sm=( Shared *)shmat(shmid,(void*)0,0);
	fd = open(fname,O_CREAT | O_RDWR,0666);
	lseek(fd, 0, SEEK_SET);
	stat(fname, &st);
	sz = st.st_size;
	read(fd,buffer,sz);
	close(fd);
	int j=0;
	int i=0;
	int i2=0;

	
	//-----------------------------------------FIFOLARI OKU-------------------------------
	while(j<sz){
		if (buffer[j]!='\n')
		{
			tempbuf[i][i2]=buffer[j];
			i2++;
		}
		else{
			tempbuf[i][i2]='\0';
			i++;
			i2=0;

		}
		j++;   //tempbuff a isimleri aldık
	}
	
	   //---------------------------------------------------------------------------------
    for(int i=0;i<N;i++){				//fifo oluşturuldu
    	char myfifo[100];
    	strcpy(myfifo,tempbuf[i]);
    	mkfifo(myfifo,0666);
    }	
    //------------------------------------------------------------------------------------
	sem_wait(sem);
    for (int i = 0; i < N; i++)    //kendini sharedde boş bir yere kaydet 
    {
    	if (sm->patatoId[i]==0)
    	{
     		sm->patatoId[i]=getpid();
     		sm->patatoSwitch[i]=byte;
			while(1){
				random=rand() % N;
				char fif[100]={0};
				int flag=0;
				strcpy(fif,tempbuf[random]);
				for (int j = 0; j < i; j++)
				{
					if (!strcmp(fif,sm->fifosnames[j]))
					{
						flag=1;
						break;
					}
				}
				if (flag!=1)
				{
					strcpy(sm->fifosnames[i],fif);
					sm->totalPatato=sm->totalPatato+1;
					me=i;
					break;
				}
			}
     		break;
    	}
    }
    
    sem_post(sem);


    //------------------------------------------------------------------------------------

   	/*shmdt(sm);
    sem_close(sem);
	sem_unlink(semapname);
    shmctl(shmid,IPC_RMID,NULL);
    //*/
    int myPatatoesId[N]={0};
    if(byte!=0){
    	myPatatoesId[0]=getpid();
    }
	j=0;
	int randd;
	int fw;
	int meInBuf;
	for (int i = 0; i < N; i++)
	{
		if(strcmp(tempbuf[i],sm->fifosnames[me])==0)
		{
			meInBuf=i;
			break;
		}
	}
	char myfifo2[100];
	strcpy(myfifo2,tempbuf[meInBuf]);
    fw = open(myfifo2,O_RDWR);
    char totcounter[20];
    sprintf(totcounter,"%d",getpid());
	int tot=strlen(totcounter);
    
    while(1){
    		  	
		while(1){
			randd= rand() % N;
			if(randd!=meInBuf){
				break;
			}
		}
		int p=0;
		if(myPatatoesId[p]!= 0){
			char myfifo1[100];
    		strcpy(myfifo1,tempbuf[randd]);
			int fw2=open(myfifo1,O_WRONLY);
			int find=myPatatoesId[0];
			int pr=0;
			for (int i = 0; i < N; i++)
			{
				if(find==sm->patatoId[i])
				{
					pr=i;
					break;
				}
			}
			printf("Pid= %d; Sending patato %d number to %s ; This is switch number %d \n",getpid(),myPatatoesId[0],tempbuf[randd],sm->patatoSwitch[pr]);
			char pid[20];
			sprintf(pid,"%d",myPatatoesId[0]);
			tot=strlen(pid);
			sem_wait(sem);
			write(fw2,pid,strlen(pid)+1);
			close(fw2);
			myPatatoesId[0]=0;
			p++;
			while(myPatatoesId[p]!=0 && p<N){
				myPatatoesId[p-1]=myPatatoesId[p];
			}
    		sem_post(sem);
		}
		
	    char buf[20];
	    
	    
	    read(fw,buf,tot+1);
	    int find=atoi(buf);
	    int pr=0;
	    int prCounter=0;
	    
	    for (int i = 0; i < N; i++)
		{
			if(find==sm->patatoId[i])
			{
				pr=i;
				
				break;
			}
			prCounter++;
		}
		if (prCounter==N)
		{	
			break;
		}
		sm->patatoSwitch[pr]=sm->patatoSwitch[pr]-1;
	    printf("pid= %d ; Recieving patato number %s from  %s \n",getpid(),buf,sm->fifosnames[pr]);
	    if (sm->patatoSwitch[pr]<=0)
	    {
	    	printf("pid = %d ;The patato %d is cooled down \n",getpid(),sm->patatoId[pr]);
	    	myPatatoesId[0]=0;
	    	sm->patatoSwitch[pr]=0;
	    }
	    p=0;
	    while(myPatatoesId[p]!=0 && p<N){
	    	p++;
	    }
	    if (sm->patatoSwitch[pr]!=0){
	    	myPatatoesId[p]=find;
	    }
    	
    	sem_wait(sem);
    	int q=0,qcount=0;
    	while(q<N){
    		if (sm->patatoSwitch[q]<=0)
    			qcount++;
    		q++;
    	}
    	if (qcount==N)
    	{
    		for (int i = 0; i < N; i++)
    		{
    			if (sm->patatoId[i]!=getpid())
    			{
    				int fw2=open(sm->fifosnames[i],O_WRONLY);
    				write(fw2,"999999",7);
					close(fw2);

    			}
    		}
    		sem_post(sem);
    		break;
    	}
    	sem_post(sem);
    	

  	 	j++;
    }    
   


   
    close(fw);
    //-----------------------------------------------------------------------------------
    shmdt(sm);
    sem_close(sem);
	sem_unlink(semapname);
    shmctl(shmid,IPC_RMID,NULL);
	
	return 0;
}