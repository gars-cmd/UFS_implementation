#include "mymkfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//############-VARS-###########
superblock sb;
inode *inodes;
d_block *dbs;
myopenfile *openfiles = NULL;




// init our filesystem
void mymkfs(int total_size){

  int avaible_size = total_size - sizeof(superblock);

  // sb.num_inodes = 10;
  // sb.num_blocks = 100;
  // sb.size_blocks = sizeof(d_block);



  sb.num_inodes = (avaible_size/10)/(sizeof(inode)); // we want the all size of inodes to be 10 % of the superblock
  sb.num_blocks = (avaible_size - (sb.num_inodes )) /sizeof(d_block) ;
  sb.size_blocks = sizeof(d_block);

  int i;
  inodes = malloc(sizeof(inode) *sb.num_inodes);
  for (i=0; i<sb.num_inodes; i++) {
    inodes[i].size = -1;
    inodes[i].first_block = -1;
    inodes[i].type = 1; // default file type we change the type on myopendir;
    strcpy(inodes[i].name, "emptyfi");
  }// init the inodes list 

  dbs = malloc(sizeof(d_block)*sb.num_blocks);
  for (i = 0; i<sb.num_blocks; i++) {
    dbs[i].next_block_num = -1;
    dbs[i].last_pos = 0;
  }



}


void sync_fs(){
  FILE *file;
  file = fopen("fs_data", "w+");

  //superblock
  fwrite(&sb, sizeof(superblock), 1,file);
  //inodes
  fwrite(inodes, sizeof(inode), sb.num_inodes, file);
  //blocks
  fwrite(dbs, sizeof(d_block), sb.num_blocks, file);

  fclose(file);
}

void mount_fs(){
  FILE *file;
  file = fopen("fs_data", "r");

  // read the superblock 
  fread(&sb , sizeof(superblock) , 1 , file);

  inodes = malloc(sizeof(inode)* sb.num_inodes);
  dbs = malloc(sizeof(d_block)* sb.num_blocks);
  int i; 
  openfiles = malloc(sizeof(myopenfile)*MAX_FILES);
  for ( i = 0; i < MAX_FILES; i++) {
    openfiles[i].inode_pos = -1;
    openfiles[i].filenum = -1;
    openfiles[i].seek_pos = 0;
  }

  //read the inodes and the blocks
  fread(inodes, sizeof(inode), sb.num_inodes, file);
  fread(dbs, sizeof(d_block), sb.num_blocks, file);

  fclose(file);
}

void print_fs(){
  printf("superblock info\n");
  printf("\t num inodes = %d size of inode = %d \n",sb.num_inodes, (int)sizeof(inode));
  printf("\t num blocks = %d\n",sb.num_blocks);
  printf("\t size of a block = %d\n",sb.size_blocks);

  printf("inodes infos\n");
  for (int i = 0; i < sb.num_inodes; i++) {
    printf("\t size -> %d , name -> %s \n",inodes[i].size , inodes[i].name);
  }
  for (int i = 0; i < sb.num_blocks; i++) {
    printf("\t block num = %d , next block -> %d \n",i,dbs[i].next_block_num);
  }
}



int mymount(const char *source, const char *target, const char *filesytemtype, unsigned long mountflags, const void *data){

  return 0;
}


int allocate_file(char *name){
  //find an empty inode 
  int in = find_empty_inode();
  if (in == -1) {
    printf("from allocate file :the inodes is full\n");
    return -1;
  }

  //find an empty block
  int block = find_empty_block(); 
  if (block == -1) {
    printf("from allocate file :the blocks are full\n");
    return -1;
  }

  if (strlen(name) > NAME-1 ) {
    printf("from allocate file :the name is too long \n");
    return -1;
  }

  // claim them 
  inodes[in].first_block = block;
  dbs[block].next_block_num = -2;
  strcpy(inodes[in].name, name);
  printf("a new file with the name %s was added \n",name);
  return in;
}


int find_empty_inode(){
  int i;
  for (i=0; i<sb.num_inodes; i++) {
    if (inodes[i].first_block == -1) {
      return i;
    }
  }
  return -1;
}  //find an empty inode


int find_empty_block(){
  int i;
  for (i=0; i<sb.num_blocks; i++) {
    if (dbs[i].next_block_num == -1) {
      return i;
    }
  }
  return -1;
}  //find an empty inode

void set_filesize(int filenum, int size){
  int tmp = size+BLOCKSIZE -1;
  int num = tmp / BLOCKSIZE;

  int bn = inodes[filenum].first_block;
  num--;

  while (num>0) {
    int next_num = dbs[bn].next_block_num;
    if (next_num == -2) {
      int empty = find_empty_block();
      dbs[bn].next_block_num = empty;
      dbs[empty].next_block_num = -2;
    }
    bn = dbs[bn].next_block_num;
    num--;
  }
  shorten_file(bn);
  dbs[bn].next_block_num = -2;
}


void write_bytes(int filenum, int pos, char data){
  int relative_block = pos / BLOCKSIZE;

  int bn = get_block_num(filenum , relative_block);

  int offset = pos%BLOCKSIZE;

  dbs[bn].data[offset] = (data);
  dbs[bn].last_pos++;
}


void shorten_file(int bn){
  int nn = dbs[bn].next_block_num;
  if (nn>=0) {
    shorten_file(nn);

  }
  dbs[bn].next_block_num = -1;
}


int get_block_num(int file, int offset){
  int togo = offset;
  int bn = inodes[file].first_block;
  while (togo>0) {
    bn = dbs[bn].next_block_num ;
    togo--;
  }
  return bn;
}

int myopen(const char *pathname, int flags){

  int size = strlen(pathname);
  char copy[size];
  strcpy(copy, pathname);
  char * n_dir = malloc(NAME);
  char * n_file = malloc(NAME);

  for (int i = 0; i < NAME; i++) {
    n_file[i] = '\0';
  }
  int i = 0;
  const char separ[2] = "/";
  // case we made open(/) open everything from root
  if (pathname[0] == '/' && size <=2) {
    printf("case we need to open all the filesystem\n");
    for ( i = 0; i < sb.num_inodes; i++) {
      if (inodes[i].type != -1) {
      add_to_openfiles(i); 
      }
    }
    // special case to implement
  }
  else {
    //case of /filename 
    n_file = strtok(copy, separ);
    while (n_dir != NULL) {
      n_dir = strtok(NULL, separ);
      if (n_dir !=NULL) {
        
      }
    }
  }
  printf("from myopen :the filename is : -%s-\n",n_file);
  if (strlen(n_file) > NAME) {
    printf("from myopen : error in getting the name\n");
    return -1;
  }
  int err_type = 0;
  for (i = 0; i < sb.num_inodes; i++) {
    if ( strcmp(inodes[i].name, n_file) == 0) {
      //we get a file 
      if (inodes[i].type == 1){
      printf("from myopen : we find a match with the name\n");
         int res_add = add_to_openfiles(i);
         if (res_add == -1) {
          printf("from myopen : the file is already open\n");
          return i;
         }
          printf("from myopen : new file added to the open list\n");
          return i;
      }
      else {
        err_type++;
      }
    }
    if (err_type > 0) {
      printf("there a directory with the same name to open it try myopendir \n");
      return -1;
    }
  }
  printf("from myopen : there is no match with inode with this name\n");

  if (flags == 'r') {
    printf("from myopen : we open a file on read only but there is no match \n");
    return -1;
  }
  printf("from myopen : we are in w or r+ mode , then we create one \n");

  int result = allocate_file(n_file);
  if (result == -1) {
    printf("from myopen : an error occur when added a new file\n");
    return -1;
  }
  free(n_dir);
  free(n_file);
  return result;
}

int myclose(int myfd){
  int i;
  //we check in the close to openfile for open or not file
  int res_clos = close_to_openfiles(myfd);
  if (res_clos == -1) {
  printf("from my close: an error occur while closed\n");
  return -1;
  }
  printf("from myclose : success\n");
  return 0;
}

ssize_t myread(int myfd, void *buf, size_t count){

  //check before if the file is already open or not 
  if (openfiles[myfd].filenum == -1){
    printf("from myread : the file need to be open before reading\n");
    return -1;
  }   

  char* ans = malloc(count + 1);
  int start_block = inodes[myfd].first_block;

  int start_seek = openfiles[myfd].seek_pos;
  printf("from myread : we read from %d\n",start_seek);

  for (int i = start_seek; i < start_seek+count; i++) {
    int block = get_block_num(myfd, i);
    char tmp = dbs[start_block].data[i];
    printf("from my read : we read the data %c\n",tmp);
    ans[i-start_seek]=dbs[start_block].data[i];
  }
  ans[count] = '\0';
  printf("from my read the data we read is : %s\n",ans);
  strncpy(buf, ans, count);
  free(ans);


  return dbs[start_block].last_pos;
}

int add_to_openfiles(int fd){
  if (openfiles[fd].inode_pos == fd) {
    printf("the file is already open\n");
    return -1;
  }
  else {
    openfiles[fd].inode_pos = fd;
    openfiles[fd].filenum = inodes->first_block;
    openfiles[fd].seek_pos = 0;
   return 0;
  }
}

int close_to_openfiles(int fd){
  if (openfiles[fd].inode_pos ==-1) {
    printf("from close lone openfile:  the file is already closed\n");
    return -1;
  }
  openfiles[fd].filenum = -1;
  openfiles[fd].inode_pos = -1;
  openfiles[fd].seek_pos = 0;
  printf("from close lone openfile:  the file was closed\n");
  return 0;
}


ssize_t mywrite(int myfd, const void *buf, size_t count){
  //check for open or not file
  if (openfiles[myfd].filenum == -1) {
    printf("from mywrite : the file need to be open before write\n");
    return -1;
  }
  
  char* str_buff;
  str_buff = (char*)buf;
  int size_buff = strlen(str_buff);

  for (int i = 0; i < size_buff; i++) {
    write_bytes(myfd, i, str_buff[i]);
  }
  printf("from mywrite : the data was successfully writed \n");
  return size_buff;//we return the size of data that was written 
}

off_t mylseek(int myfd, off_t offset, int whence){

  if (openfiles[myfd].filenum == -1) {
   printf("from mylseek : you need to open the file before to moove the seek_pointer\n");
   return -1;
  }

  switch (whence) {
    case 0: // case of SEEK_SET
      printf("from mylseek : we moove the pointer to the %d char\n",offset);
      openfiles[myfd].seek_pos = offset;
      break;
    case 1: // case of SEEK_CUR
      printf("from mylseek we : moove the pointer to the curr pos of %d to %d char\n",openfiles[myfd].seek_pos , openfiles[myfd].seek_pos + offset);
      openfiles[myfd].seek_pos += offset; 
      break;
    case 2:
      printf("from mylseek we : moove the pointer to the end pos at %d to %d char\n",dbs[inodes->first_block].last_pos , dbs[inodes->first_block].last_pos - offset );
      openfiles[myfd].seek_pos = dbs[inodes->first_block].last_pos - offset;
    default:
      printf("wrong parameter need to enter SEEK_SET , SEEK_CUR OR SEEK_END\n");
      return -1;
  }
  int seek_final = openfiles[myfd].seek_pos;
  printf(" end of seek operation with final seek_pos at %d \n",seek_final);
  return seek_final;
}

myDIR *myopendir(const char *name){

  
}

int find_file_or_dir(const char* pathname , int type ){

}
