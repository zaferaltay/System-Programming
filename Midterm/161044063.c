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
#include <sys/mman.h>
#include<signal.h>
#include <sys/wait.h>

#define NURSE 0
#define VACCINATOR 1
#define CITIZEN 2


#define SHAREDKEY 1903


struct{
	sem_t nurseMutex;
	sem_t vaccinatorsMutex;
	sem_t citizensMutex;
	sem_t empty;
	sem_t full;
	sem_t bufferAccess;
	sem_t denem;

	int temppid;
	int readedByte;
	int filesize;
	int citizensAge[2000];
	int citizenTotalVac[2000];
	int citizenTotalT[2000];
	int totalShot[2000];
	int writedByte;
	int buffer[2000];
	int vacinatorsPid[2000];
	int vacinatorsVacinateCounter[2000];
	int nursePid[2000];

	int createdHuman;
	int vac1count;
	int vac2count;

	int totalNurse;
	int finishedNurse;
	int catchSignal;
	

}typedef sharedMemory;


void nurseWork(int i,int fd,int buffersize);
void citizensWork();
void vaccinatorsWork(int i);
void signalHandler(int signum){
   if(signum==SIGUSR1){
      //CITIZEN İÇİN SIGSUSPEND YAKALAMA
   }
}
void signalHandler2(int signum){
   if(signum==SIGUSR2){
      //CITIZEN İÇİN SIGSUSPEND YAKALAMA
   }
}

sharedMemory *sm;

int main(int argc, char *argv[])
{
	
	int nurse,vaccinators,citizens,buffersize,shotTime;
	char filename[200];
	int opt;
	int whoAmI;
	int motherpid=getpid();
	int me=0;
	int i=0;
	int fd=0;
	struct stat st;
	

	while((opt = getopt(argc, argv, "n:v:b:c:t:i:"))!=-1)
	{ 
        switch(opt) 
        { 
		    case 'n':
		    nurse=atoi(optarg);
		
		    break;

		    case 'v':
        	vaccinators=atoi(optarg);
		  
		    break;

		    case 'c':         
		    citizens=atoi(optarg);
		   
		    break;

		    case 'b':
		    buffersize=atoi(optarg);
		   
		    break;

		    case 't':
		    shotTime=atoi(optarg);
		    
		    break;

		    case 'i':
		    strcpy(filename,optarg);
		  
		    break;

		    default :
		    break;
	    } 
	}
	if(nurse<2){
		fprintf(stderr, "%s\n","Invalid Nurse Number");
		exit(0);
	}
	if(vaccinators<2){
		fprintf(stderr, "%s\n","Invalid Vaccinators Number");
		exit(0);
	}
	if(citizens<3){
		fprintf(stderr, "%s\n","Invalid Citizens Number");
		exit(0);
	}	
	if(buffersize<((shotTime*citizens)+1)){
		fprintf(stderr, "%s\n","Invalid Buffer Size");
		exit(0);
	}
	if(shotTime<1){
		fprintf(stderr, "%s\n","Invalid T Number");
		exit(0);
	}

//--------------------------------------------------------------------------------------
	int shmid= shmget(SHAREDKEY,100000000,0666|IPC_CREAT); 	//Shared Mmeory init
	sm=( sharedMemory *)shmat(shmid,(void*)0,0);
	
	sm->readedByte=0;
	sm->temppid=0;
	sm->writedByte=0;
	sm->vac1count=0;
	sm->vac2count=0;
	sm->createdHuman=0;
	sm->totalNurse=nurse;
	sm->finishedNurse=0;
	sm->catchSignal=0;
	fd = open(filename,O_RDONLY,0666);
	if(stat(filename,&st)!=0){
		perror("Input file error");
	}
    sm->filesize=st.st_size;
	printf("Welcome to the GTU344 clinic. Number of citizens to vaccinate c=%d with t=%d doses\n",citizens,shotTime);
	
//-----------------------------------------------------------------------------------

	if (sem_init(&sm->citizensMutex,1,1)<0 || sem_init(&sm->vaccinatorsMutex,1,1)<0 || sem_init(&sm->bufferAccess,1,1)<0 ||sem_init(&sm->nurseMutex,1,1)<0 || sem_init(&sm->full,1,0)<0 
		|| sem_init(&sm->empty,1,buffersize)<0 || sem_init(&sm->denem,1,0)<0)
	{
		perror("Semaphore initialize error.");
        exit(EXIT_FAILURE);
	}

	for (i = 0; i < nurse+citizens+vaccinators ; i++)
	{
		int pid;

		pid=fork();
		if (pid==0)
		{

			if (i<nurse)
			{
				whoAmI=NURSE;
				//nurseWork(i,fd,buffersize);
			}
			else if(i>=nurse && i< nurse+vaccinators){

				whoAmI=VACCINATOR;
				//sm->vacinatorsPid[i-nurse]=getpid();
			}
			else if(i>= nurse+vaccinators){
				whoAmI=CITIZEN;
				sm->citizensAge[i-(nurse+vaccinators)]=getpid();
				sm->totalShot[i-(nurse+vaccinators)]=0;
				
				me=i-(nurse+vaccinators);
			}

			break;
		}
		else if (pid!=0)
		{
			//printf("---------------------------------mom for %d \n",i);

		}
	}


	if (motherpid==getpid())
	{	
		
	
		sem_wait(&sm->denem);

		int k=0;
		while(sm->vacinatorsPid[k]!=0){
			kill(sm->vacinatorsPid[k],SIGUSR1);
			k++;
		}

		sem_post(&sm->denem);

		k=0;
		while(sm->vacinatorsPid[k]!=0){
			waitpid(sm->vacinatorsPid[k], NULL, 0);
			k++;
		}
		k=0;
		while(sm->citizensAge[k]!=0){
			waitpid(sm->vacinatorsPid[k], NULL, 0);
			k++;
		}
		k=0;
		while(sm->nursePid[k]!=0){
			waitpid(sm->vacinatorsPid[k], NULL, 0);
			k++;
		}


	}
	else if(whoAmI==0){
		//printf("Hemşireyim\n");
		sm->createdHuman=sm->createdHuman+1;
		//printf("sm created huma: %d olması gerken %d \n",sm->createdHuman,(nurse+citizens+vaccinators) );
		int k=0;
		//sem_wait(&sm->vacinatoraccess);
		
		while(sm->nursePid[k]!=0){
			k++;
		}
		sm->nursePid[k]=getpid();
		if (sm->createdHuman==(nurse+citizens+vaccinators))
		{
			sem_post(&sm->denem);
		}
		nurseWork(i,fd,buffersize);
		
	}
	else if(whoAmI==1){
		int k=0;
		//sem_wait(&sm->vacinatoraccess);
		sm->createdHuman=sm->createdHuman+1;
		while(sm->vacinatorsPid[k]!=0){
			k++;
		}
		sm->vacinatorsPid[k]=getpid();
		//sem_post(&sm->vacinatoraccess);
		//printf("sm created huma: %d olması gerken %d \n",sm->createdHuman,(nurse+citizens+vaccinators) );
		if (sm->createdHuman==(nurse+citizens+vaccinators))
		{
			sem_post(&sm->denem);
		}
		vaccinatorsWork(i);
	}
	else if (whoAmI==2)
	{	
		sm->createdHuman=sm->createdHuman+1;
		//printf("sm created huma: %d olması gerken %d \n",sm->createdHuman,(nurse+citizens+vaccinators) );
		if (sm->createdHuman==(nurse+citizens+vaccinators))
		{
			sem_post(&sm->denem);
		}
		citizensWork(me,shotTime);
		
	}

	shmdt(sm);
    shmctl(shmid,IPC_RMID,NULL);

	return 0;
}

void nurseWork(int i,int fd,int buffersize){
	
	while(1){
		char c[2];
		sem_wait(&sm->empty);
		sem_wait(&sm->nurseMutex);
		sem_wait(&sm->bufferAccess);
		if (sm->readedByte==sm->filesize)
		{
			
			sm->finishedNurse=sm->finishedNurse+1;
			if (sm->finishedNurse==1)
			{//sm->totalNurse
				printf("Nurses have carried all vaccines to the buffer, terminating.\n");
			}
			sem_post(&sm->bufferAccess);
			sem_post(&sm->nurseMutex);
			break;
		}
		lseek(fd,sm->readedByte,SEEK_SET);
		read(fd,c,1);
		int x=atoi(c);
		sm->buffer[sm->writedByte]=x;
		if (x==1)
		{
			sm->vac1count=sm->vac1count+1;
		}
		else if(x==2){
			sm->vac2count=sm->vac2count+1;
		}
		
		printf("Nurse %d (pid %d) has brought vaccine %d : Clinic has %d vaccine1 and %d vaccine2 now \n",i,getpid(),x,sm->vac1count,sm->vac2count);
		sm->readedByte=sm->readedByte+1;
		sm->writedByte=sm->writedByte+1;
		sem_post(&sm->bufferAccess);
		sem_post(&sm->nurseMutex);
		sem_post(&sm->full);
		
	}
	exit(5);
}

void vaccinatorsWork(int i){
	if(signal(SIGUSR1,signalHandler)<0){
		fprintf(stderr, "%s\n","SIGUSR1 REGISTER ERROOR" );
		exit(1);
	}
	sigset_t mymask;
	sigfillset(&mymask);
	sigdelset(&mymask,SIGUSR1);
	sigsuspend(&mymask);

	
	while(1){
		
		sem_wait(&sm->full);
		sem_wait(&sm->vaccinatorsMutex);
		sem_wait(&sm->bufferAccess);

		int k=0;
		if (sm->buffer[0]==0)
		{
		
			sem_post(&sm->bufferAccess);
			sem_post(&sm->vaccinatorsMutex);
			break;
		}
		//----------------------------------KONTROL BYTELARI---------------------------------------
		int t=0;
		int temp=0;
		int count=0;
		int flag=0;
		int q=0;
		while(1){
			t=0;
			temp=0;
			q=sm->buffer[count];
			if (q==1)
			{
				while(sm->citizensAge[t]!= 0){
					if (sm->citizenTotalVac[t]==0)
					{	
						k=count;temp=t;
						flag=1;
						break;
					}
					t++;
				}
			}
			if (q==2)
			{
				while(sm->citizensAge[t]!= 0){
					if (sm->citizenTotalVac[t]==1)
					{	
						k=count;flag=1;temp=t;
						break;
					}
					t++;
				}
			}
			if (flag==1)
			{
				break;
			}
			else{
				count++;
			}

		}
		int x=sm->buffer[k];
		if (x==1)
		{
			sm->vac1count=sm->vac1count-1;
		}
		else if(x==2){
			sm->vac2count=sm->vac2count-1;
		}
		while(sm->buffer[k]!=0){
			sm->buffer[k]=sm->buffer[k+1];
			k++;
		}
		sm->writedByte=sm->writedByte-1;
		int tempT=10000;
		if(q==1){
			while(sm->citizensAge[t]!=0){
				if (sm->citizenTotalVac[t]==0)
				{
					if (tempT>sm->citizenTotalT[t])
					{
						tempT=sm->citizenTotalT[t];
						temp=t;
					}
				}

				t++;
			}
		}
		if (q==2)
		{
			while(sm->citizensAge[t]!=0){
				if (sm->citizenTotalVac[t]==1)
				{
					if (tempT>sm->citizenTotalT[t])
					{
						tempT=sm->citizenTotalT[t];
						temp=t;
					}
				}

				t++;
			}
		}
		sm->temppid=getpid();						

		int who=0;
		while(sm->vacinatorsPid[who]!=getpid()){
			who++;
		}
		sm->vacinatorsVacinateCounter[who]=sm->vacinatorsVacinateCounter[who]+1;
		printf("Vaccinator %d is inviting citizen pid=%d to the clinic\n", who,sm->citizensAge[temp]);
	
		

		kill(sm->citizensAge[temp],SIGUSR1);
	
		
		sigsuspend(&mymask);

		
		
		sem_post(&sm->bufferAccess);
		sem_post(&sm->vaccinatorsMutex);
		sem_post(&sm->empty);

	}
		exit(5);
}
void citizensWork(int me,int shotTime){
	if(signal(SIGUSR1,signalHandler)<0){
		fprintf(stderr, "%s\n","SIGUSR1 REGISTER ERROOR" );
		exit(1);
	}
	sigset_t mymask;
	sigfillset(&mymask);
	sigdelset(&mymask,SIGUSR1);
	
	while(1){  //112122112122112122
		int k=0;
		int count1=0;
		int count2=0;
	
		sigsuspend(&mymask);

		sm->citizenTotalVac[me]=sm->citizenTotalVac[me]+1;
		sm->totalShot[me]=sm->totalShot[me]+1;
		if (sm->citizenTotalVac[me]==2)
		{
			sm->citizenTotalVac[me]=0;
			sm->citizenTotalT[me]=sm->citizenTotalT[me]+1;
		}
		while(sm->buffer[k]!=0){
			
			if (sm->buffer[k]==1)
			{
				count1++;
			}
			else if(sm->buffer[k]==2){
				count2++;
			}
			k++;
		}

		printf("Citizen %d (pid=%d) is vacineted for the %dth time : the clinic has %d vaccine1 and %d vaccine2 \n",me,getpid(),sm->totalShot[me],count1,count2);
		if (sm->totalShot[me]==(shotTime*2))
		{
			sm->citizenTotalVac[me]=-1;
			int remaining=0;
			count2=0;
			while(sm->citizensAge[count2]!=0){
				if (sm->citizenTotalVac[count2]!=-1)
				{
					remaining++;
				}
				count2++;
			}

			printf("Citizen %d is leaving.Remaining citizens to vaccinate: %d \n",me,remaining);
			if (remaining==0)
			{
				printf("All citizens have been vaccinated.\n");

				int a=0;
				while(sm->vacinatorsPid[a]!=0){
					printf("Vaccinator %d (pid=%d) vaccinated %d doses. ",a,sm->vacinatorsPid[a],sm->vacinatorsVacinateCounter[a]);
					kill(sm->vacinatorsPid[a],SIGKILL);
					a++;
				}
				printf(" The clinic is now closed. Stay healthy.\n");
				break;
			}
			kill(sm->temppid,SIGUSR1);
			break;
		}
		
		kill(sm->temppid,SIGUSR1);
		
	}



	exit(5);

}