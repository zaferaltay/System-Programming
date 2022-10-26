#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include<signal.h>

#define TOTALCHILD 8
#define MAXROUND 2

pid_t childPids[TOTALCHILD]={0};
sig_atomic_t totalSuspendfromChild=0;
sig_atomic_t roundNum=1;
int elementNumforRound=6;
int currentOffset=0;

double interpolate(double x[],double y[], double xi, int n)
{
    double result = 0; // Initialize result
  
    for (int i=0; i<n; i++)
    {
        // Compute individual terms of above formula
        double term = y[i];
        for (int j=0;j<n;j++)
        {
            if (j!=i)
                term = term*(xi - x[j])/ (double) (x[i] - x[j]);
        }
  
        // Add current term to result
        result += term;
    }
  
    return result;
}
void signalHandler(int signum){

   if(signum==SIGUSR1){
      //fprintf(stderr, "%s\n","geldi");

   }
   if(signum==SIGUSR2){
          
   }


}
void parentp(int fd,const char *arr){
		
		
		
		printf("I didnt found polynomial errors but please check your text file\n");

		for (int p = 0; p < TOTALCHILD; p++)
		{
			kill(childPids[p], SIGUSR1);
		}


}
void childp(int fd,int i,int rn,const char *arr){
		char buffer[2000]={0};
		char buffer2[1000]={0};
		double round1arrayx[6];
		double round1arrayy[6];
		double round2arrayx[7];
		double round2arrayy[7];
		double fx=0;
		double predict=0;
		int detectNewLine=0;
		int detectComma=0;
		int firstNewLine=0;
		int secondCounter=0;
		int numcounter=0;
		int writeDetect=0;
		int j=0;
		struct stat st;
		int sz;

		
		if(rn==1){
				
				lockf(fd,F_LOCK,0);

				lseek(fd, 0, SEEK_SET);
				stat(arr, &st);
				sz = st.st_size;
				read(fd,buffer,sz);

				
				if(i!=TOTALCHILD-1){
					j=0;
					detectNewLine=0;
					while(j<i){
							while(buffer[detectNewLine]!='\n'){
								detectNewLine++;
							}
							if(j<(i-1) && buffer[detectNewLine]=='\n' )
								detectNewLine++;
							j++;
			
					}
					if (i!=0)
						detectNewLine++;
					firstNewLine=detectNewLine;
					while(buffer[firstNewLine]!='\n'){
						buffer2[secondCounter]=buffer[firstNewLine];
						firstNewLine++;
						secondCounter++;
					}
					buffer2[secondCounter]='\0'; //DOSYA OKUMASI BAŞLADI
					writeDetect=secondCounter;
				}
				else{
					j=0;
					detectNewLine=0;
					while(j<i){
							while(buffer[detectNewLine]!='\n'){
								detectNewLine++;
							}
							if(j<(i-1) && buffer[detectNewLine]=='\n' )
								detectNewLine++;
							j++;
					}
					detectNewLine++;
					while(buffer[detectNewLine]!='\n' && buffer[detectNewLine]!='\0' ){
						buffer2[secondCounter]=buffer[detectNewLine];
						detectNewLine++;
						secondCounter++;						
					}
					buffer2[secondCounter]='\0';
					writeDetect=secondCounter;				
				}	//DOSYA OKUMSI BİTTTT
				//printf("%d. th child buffer = %s\n",i,buffer2 );
				//printf("%d. th  buffer = \n%s\n",i,buffer );

				
				
					secondCounter=0;
					detectComma=0;
					while(secondCounter<12){
						char num[5]={0};
						j=0;
						while(buffer2[detectComma]!=','){
							num[j]=buffer2[detectComma];
							detectComma++;
							j++;
						}
						if (secondCounter%2==0)
						{
							round1arrayx[numcounter]=atoi(num);
							secondCounter++;
						}
						else{
							round1arrayy[numcounter]=atoi(num);
							numcounter++;	
							secondCounter++;
						}
						detectComma++;

					}
					int secondCounter2=0;
					int detectComma2=0;
					int numcounter2=0;
					while(secondCounter2<14){
						char num[5]={0};
						j=0;
						while(buffer2[detectComma2]!=','){
							num[j]=buffer2[detectComma2];
							detectComma2++;
							j++;
						}
						if (secondCounter2%2==0)
						{
							round2arrayx[numcounter2]=atoi(num);
							secondCounter2++;
						}
						else{
							round2arrayy[numcounter2]=atoi(num);
							numcounter2++;	
							secondCounter2++;
						}
						detectComma2++;

					}

					char numfx[5]={0};
					j=0;
					while(buffer2[detectComma]!=','){
							numfx[j]=buffer2[detectComma];
							detectComma++;
							j++;
						}
					fx=atoi(numfx);

					predict=interpolate(round1arrayx,round1arrayy,fx, 6);
					//printf("The interpolated value is %lf \n",predict);
					char numf2x[5]={0};
					j=0;
					while(buffer2[detectComma2]!=','){
							numf2x[j]=buffer2[detectComma2];
							detectComma2++;
							j++;
						}
					double f2x=atoi(numf2x);

					double predict2=interpolate(round2arrayx,round2arrayy,f2x, 7);
					//printf("The interpolated value is %lf \n",predict2);

					char numtostring[20];
					sprintf(numtostring,"%lf",predict);
					int z=0;
					buffer2[writeDetect]=',';
					writeDetect++;
					while(numtostring[z]!='\0'){
						buffer2[writeDetect]=numtostring[z];
						writeDetect++;
						z++;
					}
					buffer2[writeDetect]='\n';

					char numtostring2[20];
					sprintf(numtostring2,"%lf",predict2);
					int z2=0;
					buffer2[writeDetect]=',';
					writeDetect++;
					while(numtostring2[z2]!='\0'){
						buffer2[writeDetect]=numtostring2[z2];
						writeDetect++;
						z2++;
					}
					buffer2[writeDetect]='\n';

					//printf("%d.th new buffer %s---",i,buffer2 );

					char str1[5000];
					char str2[5000];
					writeDetect=0;
					z=0;j=0;
					
					while(z<i){
						str1[j]=buffer[writeDetect];
						j++;
						writeDetect++;
						if (buffer[writeDetect-1]=='\n')
						{
								z++;
						}
					}
					strcat(str1,buffer2);
					//printf("str1(%d): \n%s\n",i,str1 );
					if (i!=TOTALCHILD-1){
						
						j=0;
						while(buffer[writeDetect]!='\n')writeDetect++;
						writeDetect++;
						while(buffer[writeDetect]!= '\0' && z<TOTALCHILD-1){
							str2[j]=buffer[writeDetect];
							j++;writeDetect++;
							if (buffer[writeDetect-1]=='\n')
							{
								z++;
							}
						}

						strcat(str1,str2);
					}
						
					//printf("%d th için str1 \n%s\n\n",i,str1 );
				
					lseek(fd,0,SEEK_SET);
					write(fd,str1,strlen(str1));
					lockf(fd,F_ULOCK,0);	 		
		}



}

int main(int argc, char const *argv[])
{


	int fd = open(argv[1],O_CREAT | O_RDWR,0666);
	pid_t motherpid=getpid();
	pid_t pid;
	int i=0;	
	if(signal(SIGUSR1,signalHandler)<0){
		fprintf(stderr, "%s\n","SIGUSR1 REGISTER ERROOR" );
		exit(1);
	}
	if(signal(SIGUSR2,signalHandler)<0){
		fprintf(stderr, "%s\n","SIGUSR1 REGISTER ERROOR" );
	}

//---------------------------------------------------------------------------------------------------------------------	
	
	for (i = 0; i < TOTALCHILD; i++)    //CHILD OLUŞTURMA
	{	
		pid=fork();
		if (pid==0)
			break;
		else if (pid>0)
		{
			childPids[i]=pid;
		}
		else{
			fprintf(stderr, "%s\n","FORKING ERROR");
			exit(1);
		}
	}

	if (motherpid!=getpid())
	{
		sigset_t mymask;
		sigfillset(&mymask);
		sigdelset(&mymask,SIGUSR1);

		sigsuspend(&mymask);

		childp(fd,i,roundNum,argv[1]);

		exit(1);

	}
//---------------------------------------------------------------------------------------------------------------------
	if (motherpid==getpid())
	{	


		parentp(fd,argv[1]);


		for (int p = 0; p < 8; p++)
		{
			waitpid(childPids[p],NULL,0);
		}

		
	}


	
	return 0;
}
