#include<stdio.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<string.h>
#include<unistd.h> 
#include<getopt.h>
#include<stdlib.h>
#include<sys/file.h>
#include<sys/stat.h>
#include<time.h>
#include<fcntl.h>
#include<semaphore.h>
#include<errno.h>
#include <pthread.h>



int fdhw;
int fdst;
char homeworks[1000];
int totalBudget;
int hwsize;
int studentSize;
int head;
int speed[1000];
int price[1000];
int tail;
char queue[1000]={0};
int avaliable[1000];
char studentName[20][1000];
int quality[1000];
sem_t avaliableStu;
sem_t semArray[1000];
sem_t queueSemFull;
sem_t queueSemEmpty;
sem_t waitHomework;
int totalStudenthw[1000]={0};
int totalStudentPay[1000]={0};
int freeThread=0;
int firstTime=0;
int waithw=0;
int readedByte=0;
int finito=0;

char* substring(char *destination, const char *source, int beg, int n)
{
	while (n > 0)
    {
        *destination = *(source + beg);
        destination++;
        source++;
        n--;
    }
    *destination = '\0';
    return destination;
}

void *h (void *hwlist){
		
	while(totalBudget>0 && readedByte<hwsize){
		sem_post(&queueSemFull);
		sem_wait(&queueSemEmpty);
		if (freeThread==1)
		{
			break;
		}
		
		char den[1];
		read(fdhw,den,1);
		strcat(queue,den);
		readedByte++;
		tail++;
		printf("H has a new homework %c; remaining money is %d TL\n",den[0],totalBudget);
		if (waithw==1)
		{
			waithw=0;
			sem_post(&waitHomework);
		}
		if (firstTime==0)
		{
			for (int i = 0; i < studentSize; i++)
			{
				sem_post(&avaliableStu);
			}
			firstTime++;
		}
		
	}
	
	 return NULL;
}

void *studentWork(void *student)
{	

	while(1){
		if (finito!=1)
			printf("%s waiting for homework\n",studentName[(intptr_t)student]);
		sem_wait(&semArray[(intptr_t)student]);
		sem_post(&queueSemEmpty);
		sem_wait(&queueSemFull);
		
		if (freeThread==1 || totalBudget<price[(intptr_t)student])
		{	
			
			break;
		}
		int wt=6-speed[(intptr_t)student];
		totalBudget-=price[(intptr_t)student];
		totalStudenthw[(intptr_t)student]=totalStudenthw[(intptr_t)student]+1;
		totalStudentPay[(intptr_t )student]=totalStudentPay[(intptr_t)student]+price[(intptr_t)student];
	    printf("%s is solving homework for %d, H has %dTL left.\n",studentName[(intptr_t)student],price[(intptr_t)student],totalBudget); 
	    sleep(wt);
	    avaliable[(intptr_t)student]=1;
	    sem_post(&avaliableStu);
	}

     return NULL;
}

int main(int argc, char const *argv[])
{	

	char hwpath[500];
	char studentpath[500];
	struct stat st;
	struct stat st2;
	strcpy(hwpath,argv[1]);
	strcpy(studentpath,argv[2]);
	totalBudget=atoi(argv[3]);

	fdst=open(studentpath,O_RDONLY,0666);
	if(stat(studentpath,&st2)!=0){
		perror("Input file error");
	}
	studentSize=st2.st_size;

	fdhw=open(hwpath,O_RDONLY,0666);
	if(stat(hwpath,&st)!=0){
		perror("Input file error");
	}
	hwsize=st.st_size;


	printf("hwpath:   %s\n",hwpath );
	printf("studentpath:   %s\n",studentpath );
	printf("totalBudget:   %d\n",totalBudget);

	char students[2000]={0};
	read(fdst,students,studentSize);
	printf("%d \n",studentSize );

	// OKUNULAN ÖĞRENCİYİ PARSE ETME BAŞLANGIÇ--------------------------------------------------------------
	int j=0;int x=0;
	int oldCount2=0;
	int count3=0;
	while(students[j]!=0){
		char yedek1[20]={0};
		if(students[j]==' ' || students[j]=='\n' || students[j]==0){
			substring(yedek1, students, oldCount2, j-oldCount2);
			oldCount2=j+1;
			if (count3==0)
			{
				strcpy(studentName[x],yedek1);
				count3++;
			}
			else if(count3==1){
				
				quality[x]=atoi(yedek1);
				count3++;
			}
			else if(count3==2){
			
				speed[x]=atoi(yedek1);
				count3++;
			}
			else if (count3==3)
			{
				price[x]=atoi(yedek1);
				x++;
				count3=0;
			}		
		}
		j++;
	}
	char yedek1[20]={0};
	substring(yedek1, students, oldCount2, j-oldCount2);
	price[x]=atoi(yedek1);
	x++;
	studentSize=x;
	for (int i = 0; i < studentSize; i++)
	{
		avaliable[i]=1;
	}
	if (sem_init(&avaliableStu,1,0)<0)
	{
		perror("Semaphore initialize error.");
        exit(EXIT_FAILURE);
	}
	if (sem_init(&queueSemEmpty,1,studentSize)<0)
	{
		perror("Semaphore initialize error.");
        exit(EXIT_FAILURE);
	}
	if (sem_init(&waitHomework,1,0)<0)
	{
		perror("Semaphore initialize error.");
        exit(EXIT_FAILURE);
	}
	if (sem_init(&queueSemFull,1,0)<0)
	{
		perror("Semaphore initialize error.");
        exit(EXIT_FAILURE);
	}

//OKUNULAN ÖĞRENCİYİ PARSE ETME BİTİŞ-------------------------------------------------------------------
	pthread_t tid;
	pthread_create(&tid, NULL, h, NULL);
	pthread_detach(tid);
	freeThread=0;
	for (int i = 0; i <studentSize; i++)
	{
		sem_init(&semArray[i],1,0);
		pthread_t tid;
		pthread_create(&tid, NULL, studentWork, (void*)(intptr_t)i);
		pthread_detach(tid);
	}
	
	char now;
	
	while(totalBudget>0){
		sem_wait(&avaliableStu);
		now=queue[head];
		head++;
		if(now== ' ' || now=='\n' || (now=='\0' && readedByte==hwsize)){
			freeThread=1;
			
			break;
		}
		if (now==0 || now=='\0')
		{	
			waithw=1;
			
			head--;
			sem_wait(&waitHomework);
			continue;
		}
		
		if (now=='C')
		{
			int min=0;
			int minCount=100000;
			int c=0;
			while(c<studentSize){
				if (avaliable[c]==1 && price[c]<minCount)
				{
					min=c;
					minCount=price[c];
				}
				c++;
			}

			if (totalBudget<price[min])
			{
				printf("H has no more money for homeworks, terminating.  \n");
				freeThread=1;
				break;
			}else{
				
				avaliable[min]=0;
				sem_post(&semArray[min]);
			}
		}
		else if(now=='Q'){
			int max=0;
			int maxCount=-1;
			int c=0;
			while(c<studentSize){
				if (avaliable[c]==1 && quality[c]>maxCount && totalBudget>price[c])
				{
					max=c;
					maxCount=quality[c];
				}
				c++;
			}

			if (maxCount==-1)
			{
				printf("H has no more money for homeworks, terminating.  \n");
				freeThread=1;
				break;
			}
			else{
				
				avaliable[max]=0;
				sem_post(&semArray[max]);
			}

		}
		else if(now=='S'){
			int max=0;
			int maxCount=-1;
			int c=0;
			while(c<studentSize){
				if (avaliable[c]==1 && speed[c]>maxCount && totalBudget>price[c] )
				{
					max=c;
					maxCount=speed[c];
				}
				c++;
			}
			if (maxCount==-1)
			{
				printf("H has no more money for homeworks, terminating.  \n");
				freeThread=1;
				break;
			}else{
				
				avaliable[max]=0;
				sem_post(&semArray[max]);
			}	
		}
	}



	if (freeThread==1)
	{
		for (int i = 0; i < studentSize; i++)
		{
			sem_post(&semArray[i]);
			sem_post(&queueSemEmpty);
			sem_post(&queueSemFull);
		}
		printf("H has no more homeworks, terminating. \n");
	}
	printf("Homeworks solved and money made by the students:\n");
	int totPay=0;
	int totHw=0;
	for (int i = 0; i < studentSize; i++)
	{
		printf("%s %d %d \n",studentName[i],totalStudenthw[i],totalStudentPay[i] );
		totPay+=totalStudentPay[i];
		totHw+=totalStudenthw[i];
		sem_close(&semArray[i]);
	}
	printf("Total cost for %d homeworks %dTL\n",totHw,totPay);
	printf("Money left at G's account: %dTL\n",totalBudget);


	finito=1;

	pthread_exit(0);
	close(fdhw);
	close(fdst);
    sem_close(&avaliableStu);
    sem_close(&queueSemFull);
    sem_close(&queueSemEmpty);
    sem_close(&waitHomework);
	


	return 0;
}