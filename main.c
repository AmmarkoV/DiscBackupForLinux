#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>



char * path_cat2 (const char *str1,const char *str2)
{
    size_t str1_len = strlen(str1);
    size_t str2_len = strlen(str2);
    char *result;
    result = malloc((str1_len+str2_len+1)*sizeof *result);
    strcpy (result,str1);
    unsigned int i,j;
    for(i=str1_len, j=0; ((i<(str1_len+str2_len)) && (j<str2_len)); i++, j++)
    {
        result[i]=str2[j];
    }
    result[str1_len+str2_len]='\0';
    return result;
}

int clearPrevious( const char * outputPath)
{

 char command[2048];
   snprintf(command,2048," rm output_image.iso");
 int  i=system(command);

   snprintf(command,2048," rm outputPath/*");
   i=system(command);

}


int writeDisc()
{
 char command[2048];
 snprintf(command,2048,"growisofs -dvd-compat -Z /dev/sr0=output_image.iso");
 int i=system(command);

return 1;

}



int executeWrite(unsigned int part , const char * outputPath)
{
   char command[2048];
   snprintf(command,2048,"mkisofs -V disc%u -lfJR -o output_image.iso %s",part,outputPath);
   int i=system(command);

return 1;
}


int makeLinkedFolders(const char * directoryPath,const char * outputPath,unsigned long splitSize)
{
    struct stat st;
    struct dirent *dp= {0};
    unsigned long accumaltedSize=0;
    unsigned int part=1;
// enter existing path to directory below
    DIR *dir = opendir(directoryPath);
    if (dir==0)
    {
        return 0;
    }
    while ((dp=readdir(dir)) != 0)
    {
        //TODO: remove // from requests.. of dp->d_name is like /filename.ext
        //char *tmp = path_cat(client_path, dp->d_name);

        if (dp->d_name==0)
        {
            fprintf(stderr,"Got garbage out of readdir(%s)\n",directoryPath);
        }
        else if ( (strcmp(dp->d_name,".")!=0) && (strcmp(dp->d_name,"..")!=0) )
        {
            if (
                (strlen(dp->d_name)>0) &&
                (dp->d_name[0]=='.')
            )
            {
                fprintf(stderr,"hidden file %s from directory list\n" ,dp->d_name);
            }
            else
            {

                fprintf(stderr,"File %s - ",dp->d_name);
                char * fullpath = path_cat2(directoryPath,dp->d_name);
                if (fullpath!=0 )
                {
                 if ( stat(fullpath, &st) == 0 )
                    {
                      if (S_ISDIR(st.st_mode))
                       {
                         //Handle DIR here
                         fprintf(stderr,"ignoring directory\n" ,dp->d_name);
                       }
                         else
                       {
                        if (accumaltedSize+st.st_size>splitSize)
                        {
                         //BLOCK AND DO WRITE HERE..
                         executeWrite(outputPath);
                         writeDisc();
                         clearPrevious(outputPath);
                        }
                         accumaltedSize+=st.st_size;
                         fprintf(stderr,"%u bytes - acc %u\n",st.st_size,accumaltedSize);


                         char command[2048];
                         snprintf(command,2048,"ln -s %s %s/%s",fullpath,outputPath,dp->d_name);
                         int i=system(command);
                        }
                    }
                    else
                    {
                        fprintf(stderr,"Error stating file %s -> %s\n",fullpath,strerror(errno));
                    }
                    free(fullpath);
                    fullpath=0;
                }
                //---------------------------------
              }// <- we still have space
            }

        }



    closedir(dir);
    return 1;
}







int main(int argc, char *argv[])
{
    if ( argc<3 ) { return 0; }
    makeLinkedFolders(argv[1],argv[2],4*1024*1024*1024);
    fprintf(stderr,"Done..!\n");
    return 0;
}
