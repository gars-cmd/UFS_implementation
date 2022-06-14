#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mymkfs.h"


int main (int argc, char *argv[])
{

// ##############################-TEST / MYMKFS / MOUNT-##############################

  mymkfs(10000);
  printf("we create the file system with mymkfs \n");
  sync_fs("test");

  mymount("fs_data", NULL, 0, NULL);
  printf("we mount the file fs_data as our system file \n");



// ##############################-MYOPEN / MYREAD / MYCLOSE / MYWRITE-##############################
 char file1[] = "myf1";
 printf("we try to open the file %s that doesn't exist already in O_CREATE mode \n",file1);
 int fd1 = myopen(file1, O_CREATE);
 printf("we create and open the file with fd %d\n",fd1);
 printf("we create another file with the name  but don't open it \n");
 int fd2 = allocate_file("myf2");
 printf("we create and open the file with fd %d\n",fd2);
 printf("let's try to write the file we didn't open\n");
 int res = mywrite(fd2, "just a test",12);
 printf("the write function return %d then didn't work cause the file wasn't open\n",res);
 printf("then let's write the file myf1 <it's just a text>\n");
 mywrite(fd1, "it's just a text", 17);
 printf("now we read it \n");
 char bufff1[17];
 myread(fd1, bufff1, 17);
 printf("from the file with fd %d we read is : %s\n",fd1,bufff1);
//

// ##############################-MYOPENDIR / MYREADDIR-############################## 
//
printf("now we will create and open  a new directory with pathname : dd1 \n");
myDIR* dd1 = myopendir("dd1");
printf("the directory have the fd %d \n",dd1->inode_pos);

printf("let's create a directory into this directory with pathname /dd1/dd12 \n");
myDIR* dd12 = myopendir("/dd1/dd12");
printf("the directory have the fd %d \n",dd12->inode_pos);

printf("let's create a file at place /dd1/dd12/f3 \n");
int fd3 = myopen("/dd1/dd12/f3",O_CREATE);
printf("the file descriptor of the new file is %d\n",fd3);

printf("let's write <abcdefgh12345678> in the file\n");
mywrite(fd3, "abcdefgh12345678", 31);

printf("now we read it \n");
char bufff2[17];
memset(bufff2, 0, 17);
myread(fd3, bufff2, 17);
printf("from the file with fd %d we read %s\n",fd3,bufff2);

printf("now  we close the dir /dd1 and al lthe files in \n");
myclosedir(dd1);

printf("let's try to read the dd2 dir \n");
myreaddir(dd12);

printf("now we open the file f3 \n");
myopen("f3", 0);

printf("now let's try to lseek \n");
mylseek(fd3, 5, SEEK_CUR); // we read from place 3
printf("we lseek SEEK_SET with offset 5 \n");

char* bufff3 = calloc(6, 1);
myread(fd3, bufff3, 5);
printf("we read 5 char after the lseek from the file with text <abcdefgh12345678>\n");
printf("we read : %s\n",bufff3);

char* bufff4 = calloc(6, 1);
mylseek(fd3, 5, SEEK_END);
printf("we lseek from the end with SEEK_END with offset 5\n");
myread(fd3, bufff4, 5);
printf("we read 5 char from the end -5 char  of the file with text <abcdefgh12345678>\n");
printf("we read : %s\n",bufff4);

  return 0;
}
