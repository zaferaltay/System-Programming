#include <stdio.h> 
#include <unistd.h> 
#include <getopt.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include  <signal.h>
char **stringStore;


void  INThandler(int sig)
{   
    int i=0;
    signal(sig, SIG_IGN);
    fprintf(stderr, "Memory cleared and program will terminated");
    while(stringStore[i]!=0)
     {
         free(stringStore[i]);
         i++;
     } 
     free(stringStore);
    exit(0);
}




char *strrev(char *str)
{
      char *p1, *p2;

      if (! str || ! *str)
            return str;
      for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
      {
            *p1 ^= *p2;
            *p2 ^= *p1;
            *p1 ^= *p2;
      }
      
      return str;
}

int regexpMatch(char *target1,char *target2){
    int i=0;
    int j=0;

    while(i<strlen(target2) && j< strlen(target1)){
        if (target2[i]==target1[j]){
            i++;
            j++;
        }
        else if(target2[i]=='+'){
            i++;
            while(target1[j-1]==target1[j])
                j++;
        }
        else
           return 0;    
        
    }

    if (i==strlen(target2) && j==strlen(target1))
        return 1;
    
    else
        return 0;
}

void getSubstring(char *target,char *sub,int start,int end){
    int j=0,i;
    for(i=start;i<end;i++){
        sub[j]=target[i];
        j++;
    }
    return;
}

void stringUpperCase(char string[]){
    int i=0;
    while(i<strlen(string)){
                    if (string[i]>='a' && string[i]<='z')
                    {
                        string[i]-=32;
                    }
                   i++; 
    }
    
}
void add2StringStrore(char *name,int space,char yn){

    int i=0,j=0;
    char num[10];
    char oneElemenStringStore[102];

    while(stringStore[i][0]!=0){
        i++;
    }
    
    sprintf(num,"%d",space);
    snprintf(oneElemenStringStore, sizeof(oneElemenStringStore), "%s-%s%c%c", name, num,yn,'x');
    while(j<strlen(oneElemenStringStore)){
        stringStore[i][j]=oneElemenStringStore[j];
        j++;  
    }

}
void printFileTree(const char *name, int space,char filename[],char perm[],char type,int byte,int link){

    DIR *dir;
    struct dirent *start;
    struct stat fileStat;
    int filesize,linksize;
    int count=0,totalcount=0,permCounter=0,i;



    if (!(dir=opendir(name)))
    {
        return;
    }

    while ((start=readdir(dir))!=NULL)
    {   

        if (start->d_type == DT_DIR)
         {  
        
            char path[1200];
            char permissions[9];
            char temp[100];
            if (strcmp(start->d_name,".")==0 || strcmp(start->d_name,"..")==0)
                continue;

            snprintf(path, sizeof(path), "%s/%s", name, start->d_name);
            printFileTree(path,space+4,filename,perm,type,byte,link);
            totalcount=0;
            stat(path, &fileStat);
                
              
            if (filename[0] != '"'){   
                totalcount++;
                stringUpperCase(filename);
                strcpy(temp,start->d_name);
                stringUpperCase(temp);
                if(regexpMatch(temp,filename))
                    count++;
            }
            if (perm[0]!= '"'){
                permCounter=0;
                totalcount++;
                permissions[0]=( (fileStat.st_mode & S_IRUSR) ? 'r' : '-');
                permissions[1]=( (fileStat.st_mode & S_IWUSR) ? 'w' : '-');
                permissions[2]=( (fileStat.st_mode & S_IXUSR) ? 'x' : '-');
                permissions[3]=( (fileStat.st_mode & S_IRGRP) ? 'r' : '-');
                permissions[4]=( (fileStat.st_mode & S_IWGRP) ? 'w' : '-');
                permissions[5]=( (fileStat.st_mode & S_IXGRP) ? 'x' : '-');
                permissions[6]=( (fileStat.st_mode & S_IROTH) ? 'r' : '-');
                permissions[7]=( (fileStat.st_mode & S_IWOTH) ? 'w' : '-');
                permissions[8]=( (fileStat.st_mode & S_IXOTH) ? 'x' : '-');

                for (i = 0; i < 9; i++)
                {   
                    if (perm[i]==permissions[i]){
                        permCounter++;
                    }
                }
                if (permCounter==9)
                    count++;
            }

            if (type!='"')
            {
                totalcount++;

                if (S_ISDIR(fileStat.st_mode)){
                    if (type=='d')
                        count++;
                }

                else if (S_ISREG(fileStat.st_mode)){
                    if (type=='f')
                        count++;
                }

                else if (S_ISCHR(fileStat.st_mode)){
                    if (type=='c')
                        count++;
                }

                else if (S_ISBLK(fileStat.st_mode)){

                    if (type=='b')
                        count++;
                }

                else if (S_ISFIFO(fileStat.st_mode)){
                    if (type=='p')
                        count++;
                }

                else if (S_ISLNK(fileStat.st_mode)){
                    if (type=='l')
                        count++;
                }

                else if (S_ISSOCK(fileStat.st_mode)){
                    if (type=='s')
                        count++;
                }

            }
            if (byte!=-1)
            {
                totalcount++;
                filesize=fileStat.st_size;
                if (filesize==byte)
                    count++;
            }
            if (link!=-1)
            {   
                totalcount++;
                linksize=fileStat.st_nlink;
                if (linksize==link)
                    count++;
            }              
           
            if (totalcount==count)
                add2StringStrore(start->d_name,space,'y');
            else{
                add2StringStrore(start->d_name,space,'n'); 
            }
            

                totalcount=0;
                count=0; 

        }
        else{
            char permissions[9];
            char path[1024];
            char temp[100];
            snprintf(path, sizeof(path), "%s/%s", name, start->d_name);
            stat(path, &fileStat);
            totalcount=0;
            count=0;

            if (filename[0]!= '"'){   
                totalcount++;
                strcpy(temp,start->d_name);         
                stringUpperCase(temp);
                if(regexpMatch(temp,filename))
                    count++;
            }
            if (perm[0]!= '"'){
                permCounter=0;
                totalcount++;
                permissions[0]=( (fileStat.st_mode & S_IRUSR) ? 'r' : '-');
                permissions[1]=( (fileStat.st_mode & S_IWUSR) ? 'w' : '-');
                permissions[2]=( (fileStat.st_mode & S_IXUSR) ? 'x' : '-');
                permissions[3]=( (fileStat.st_mode & S_IRGRP) ? 'r' : '-');
                permissions[4]=( (fileStat.st_mode & S_IWGRP) ? 'w' : '-');
                permissions[5]=( (fileStat.st_mode & S_IXGRP) ? 'x' : '-');
                permissions[6]=( (fileStat.st_mode & S_IROTH) ? 'r' : '-');
                permissions[7]=( (fileStat.st_mode & S_IWOTH) ? 'w' : '-');
                permissions[8]=( (fileStat.st_mode & S_IXOTH) ? 'x' : '-');

                
                for (int i = 0; i < 9; i++)
                {   
                    if (perm[i]==permissions[i]){
                        permCounter++;
                    }
                }

                if (permCounter==9)
                    count++;


            }
            if (type!='"')
            {
                totalcount++;

                if (S_ISDIR(fileStat.st_mode)){
                    if (type=='d')
                        count++;
                }

                else if (S_ISREG(fileStat.st_mode)){
                    if (type=='f')
                        count++;
                }

                else if (S_ISCHR(fileStat.st_mode)){
                    if (type=='c')
                        count++;
                }

                else if (S_ISBLK(fileStat.st_mode)){

                    if (type=='b')
                        count++;
                }

                else if (S_ISFIFO(fileStat.st_mode)){
                    if (type=='p')
                        count++;
                }

                else if (S_ISLNK(fileStat.st_mode)){
                    if (type=='l')
                        count++;
                }

                else if (S_ISSOCK(fileStat.st_mode)){
                    if (type=='s')
                        count++;
                }

            }
            if (byte!=-1)
            {
                totalcount++;
                filesize=fileStat.st_size;
                if (filesize==byte)
                    count++;
            }
            if (link!=-1)
            {   
                totalcount++;
                linksize=fileStat.st_nlink;
                if (linksize==link)
                    count++;
            }               

          
            if (totalcount==count)
                add2StringStrore(start->d_name,space,'y');
            else
                add2StringStrore(start->d_name,space,'n'); 
            
            totalcount=0;
            count=0;

         } 
    }
    closedir(dir);

}
int main(int argc, char *argv[]) 
{
    int opt,i,j,x,bflag=0,tflag=0,fflag=0,pflag=0,lflag=0,wflag=0,space=0;
    int byte,link,spacenum,spaceindex,spacecounter,sizee,size2;
    int k=0,atoiCounter=0;
    char type;
    char permissions[9];
    char filename[100];
    int temp2;
    char path[1024];
    //char **stringStore;
    signal(SIGINT, INThandler);
    
    stringStore=(char **)malloc(100480*sizeof(char *));
    for (int i=0;i<100480;i++)
    {
        stringStore[i]=(char*)malloc(100*sizeof(char));
    }
       
    int *arr;
    arr=(int*)malloc(100480*sizeof(int));
      
    while((opt = getopt(argc, argv, "w:f:b:t:p:l:"))!=-1) 
    { 
        switch(opt) 
        { 
            case 'w':           
                    strcpy(path,optarg);
                    wflag=1;
                    
            break;

            case 'b':
                byte=atoi(optarg);
                bflag=1;
            break;

            case 't':
                type=optarg[0];
                tflag=1;
            break;

            case 'f':
                stringUpperCase(optarg);
                strcpy(filename,optarg);
                fflag=1;
            break;
            case 'p':
                strcpy(permissions,optarg);
                pflag=1;
            break;
            case 'l':
                link=atoi(optarg);
                lflag=1;
            break;
            default :
                break;
            
        } 
    }
     
        


       
        if (wflag==0)
        {
            fprintf(stderr, "Path not entered");
            exit(1);   
        }

        if (fflag==0)
            filename[0]='"';
        if (bflag==0)
            byte=-1;
        if (tflag==0)
            type='"';
        if (pflag==0)
            permissions[0]='"';
        if (lflag==0)
            link=-1;
        
        if (fflag==0 && bflag==0 && tflag==0 && pflag==0 && lflag==0)
        {
            fprintf(stderr, "Missing information has been entered");
            exit(1);
        }    

        
        printFileTree(path,space,filename,permissions,type,byte,link);
     
        i=0;
        while(stringStore[i][0]!=0){
            i++;

        }

        
        sizee=i;   
        k=0;
        i=0;
        spacecounter=0;
        while(i<sizee){
            char foratoi[5]="";
            if(stringStore[i][strlen(stringStore[i])-2]=='y' && stringStore[i][strlen(stringStore[i])-1]=='x'){
                k=strlen(stringStore[i])-3;
                atoiCounter=0;
                while(stringStore[i][k]>='0' && stringStore[i][k]<='9'){
                    foratoi[atoiCounter]=stringStore[i][k];
                    k--;
                    atoiCounter++;
                }
                strrev(foratoi);
               
                spacenum=atoi(foratoi);
                stringStore[i][strlen(stringStore[i])-1]='w';
                arr[spacecounter]=i;
                spacecounter++;
                j=i+1;
                while(j<sizee){
                    char foratoi[5]="";
                    if (stringStore[j][strlen(stringStore[j])-1]=='x')
                    {
                        k=strlen(stringStore[j])-3;
                        atoiCounter=0;
                        while(stringStore[j][k]>='0' && stringStore[j][k]<='9'){
                            foratoi[atoiCounter]=stringStore[j][k];
                            k--;
                            atoiCounter++;
                        }
                        strrev(foratoi);
                        spaceindex=atoi(foratoi);
                        if (spaceindex<spacenum)
                        {
                            spacenum=spaceindex;
                            stringStore[j][strlen(stringStore[j])-1]='w';
                            arr[spacecounter]=j;
                            spacecounter++;
                        }
                        
                    }else break;
                    j++;     
                }
                
            }


            i++;
        }
        
    
        size2=spacecounter;
        for (i = 0; i < (size2 - 1); i++)
        {
            for (j = 0; j < size2 - 1 - i; j++ )
            {
                if (arr[j] > arr[j+1])
                {
                    temp2 = arr[j+1];
                    arr[j+1] = arr[j];
                    arr[j] = temp2;
                }
            }
        }   

        i=spacecounter-1;
        if(i==-1){
            fprintf(stderr, "No File Found");
            exit(1); 
        }
        while(0<=i){
            char foratoi[5]="";
            char sstring[100]="";            


            k=strlen(stringStore[arr[i]])-3;
            atoiCounter=0;
            while(stringStore[arr[i]][k]>='0' && stringStore[arr[i]][k]<='9'){
                foratoi[atoiCounter]=stringStore[arr[i]][k];
                k--;
                atoiCounter++;
            }      
            getSubstring(stringStore[arr[i]],sstring,0,k);
            strrev(foratoi);
            spacenum=atoi(foratoi);
            printf("|");
            for (x = 0; x <spacenum; x++)
            {
                printf("--");
            }
            printf("%s\n",sstring);
            i--;
        }
        i=0;
        while(stringStore[i]!=0)
        {
            free(stringStore[i]);
            i++;
        } 
        free(stringStore);
        free(arr);

        return 0;
}