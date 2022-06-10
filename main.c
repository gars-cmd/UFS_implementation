#include <stdio.h>
#include <stdlib.h>
#include "mymkfs.h"


int main (int argc, char *argv[])
{
  mymkfs(10000);
  sync_fs();
  mount_fs();
  int f0 =  allocate_file("uno");
  int f1 = allocate_file("dos");
  // int f2 = allocate_file("tres");
  // int f3 = allocate_file("quattro");
  //
  // char buff[] = "hello les bitches-0-";
  // char buff1[] = "hello les bitches-1-";
  // char buff2[] = "hello les bitches-2-";
  // char buff3[] = "hello les bitches-3-";
  //
  // write_bytes(f0, 0, buff);
  // write_bytes(f1, 0, buff1);
  // write_bytes(f2, 0, buff2);
  // write_bytes(f3, 0, buff3);
  // set_filesize(f0, 5000);

  sync_fs();

  myopen("/uno", 0);

  mywrite(f0, "abcdefgh12345678",17);

  // print_fs();
  //
  //
   char buff[6];
  myread(f0, buff, 6);
  printf("we read :-%s-\n",buff); sync_fs();
  mylseek(f0,5,SEEK_SET);
  myread(f0, buff, 6);
  printf("we read :-%s-\n",buff); sync_fs();
  myclose(f0);

  printf("done with success\n");

  return 0;
}
