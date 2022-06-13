#include <stdio.h>
#include <stdlib.h>
#include "mymkfs.h"


int main (int argc, char *argv[])
{
  mymkfs(10000);
  sync_fs("test");
  mymount("fs_data", NULL, 0, NULL);
//   int f0 =  allocate_file("uno");
//   int f1 = allocate_file("dos");
//   sync_fs();
//
//   myopen("/uno", 0);
// //
//   mywrite(f0, "abcdefgh12345678",17);
// // // seek , read , open test 
//   char buff[6];
//   myread(f0, buff, 6);
//   printf("we read :-%s-\n",buff); sync_fs();
// //   mylseek(f0,5,SEEK_SET);
//   myread(f0, buff, 6);
// //   printf("we read :-%s-\n",buff); sync_fs();
//   myclose(f0);
//
// // find file or folder test  
//  // int ans1 =find_fileodir("/check", FILE_T , 0);
//  // // sync_fs();
//  // int ans2 = find_fileodir("check", FILE_T, 0);
//  // sync_fs();
//  // myopen("check",0);
//  // mywrite(ans1,"hello les tarlouzes", 20);
//  // char buff2[20];
//  // myread(ans2, buff2, 20);
//  // printf("the content of file check is : %s\n",buff2);
//  // printf("check file1  = %d , file 2 = %d\n",ans1,ans2);
//  sync_fs();
//  //
myDIR* d1 = myopendir("/lol");
// sync_fs();
// sync_fs();
myDIR* d2 = myopendir("/lol/tasse");
myDIR* d3 = myopendir("/lol/chou");
myDIR* d4 = myopendir("/lol/chou/tache");
int f4 = myopen("lol/chou/tache/gros", O_CREATE);
myDIR* d5 = myopendir("/lol/chou/tache/table");
int f11 = myopen("lol/tasse/tigre", O_CREATE);
int f12 = myopen("/lol/chou/tache/table/oiseau", O_CREATE);
mydirent* dd = myreaddir(d4);
printf("the dir name is : %s\n",dd->name);

open_rec_dir(d1);
myclosedir(d1);
open_rec_dir(d1);
mywrite(f12,"fesses",15);
mywrite(f4,"gros chat bleu",15);
char buff4[15];
char buff5[7];
myread(f4,buff4,15);
myread(f12,buff5,7);

printf("the content of /lol/chou/tache/table/oiseau is %s\n",buff5);
printf("the content of /lol/chou/tache/gros is %s\n",buff4);
 // sync_fs();
mywrite(f11, "test - test 2", 14);
char buff3[14];
myread(f11, buff3, 14);
printf("the content of file lol/tasse/tigre is %s \n",buff3);
printf("done with success\n");
//

  return 0;
}
