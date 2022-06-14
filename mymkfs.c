#include "myfs.h"
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

  sb.num_inodes = (avaible_size/10)/(sizeof(inode)); // we want the all size of inodes to be 10 % of the superblock
  sb.num_blocks = (avaible_size - (sb.num_inodes )) /sizeof(d_block) ;
  sb.size_blocks = sizeof(d_block);

  int i;
  inodes = malloc(sizeof(inode) *sb.num_inodes);
  for (i=0; i<sb.num_inodes; i++) {
    inodes[i].size = -1;
    inodes[i].first_block = -1;
    inodes[i].ptr = NULL; // default file type we change the type on myopendir;
    strcpy(inodes[i].name, "emptyfi");
  }// init the inodes list 

  dbs = malloc(sizeof(d_block)*sb.num_blocks);
  for (i = 0; i<sb.num_blocks; i++) {
    dbs[i].next_block_num = -1;
    dbs[i].last_pos = 0;
  }
}


void sync_fs(const char* trg){
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

void mount_fs(const char* src){
  FILE *file;
  file = fopen(src,"r");

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



int mymount(const char *source, const char *target, const char *filesytemtype, unsigned long mountflags){
  if (source == NULL && target == NULL) {
    printf("no enough parameter was defined\n");
    return -1;
  }
  if (source != NULL) {
    mount_fs(source);
  }
  if (target != NULL) {
    sync_fs(target);
  }
  return 0;
}


int allocate_file(const char *name){
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
  int size_n = strlen(name);
  if (size_n > NAME-1 ) {
    printf("from allocate file :the name is too long size: %d \n",size_n);
    return -1;
  }else {
    // printf("from allocate_file : the size of the name is %d\n",size_n);
  }
  // claim them 
  inodes[in].first_block = block;
  dbs[block].next_block_num = -2;
  strcpy(inodes[in].name, name);
  inodes[in].ptr=NULL;
  // printf("a new file/fodler with the name %s was added \n",inodes[in].name);
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
  int ans = find_fileodir(pathname, FILE_T, flags); // get the file descriptor for the wanted file 
  int res = add_to_openfiles(ans);
  if (res == -1) {
    printf("from myread : the file is already open \n");
    return -1;
  }else {
    return ans;
  }
  return ans;
}

int myclose(int myfd){
  int i;
  //we check in the close to openfile for open or not file
  int res_clos = close_to_openfiles(myfd);
  if (res_clos == -1) {
    printf("from my close: an error occur while closed\n");
    return -1;
  }
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

  for (int i = start_seek; i < start_seek+count; i++) {
    ans[i-start_seek]=dbs[start_block].data[i];
  }
  ans[count] = '\0';
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
  return size_buff;//we return the size of data that was written 
}

off_t mylseek(int myfd, off_t offset, int whence){

  if (openfiles[myfd].filenum == -1) {
    printf("from mylseek : you need to open the file before to moove the seek_pointer\n");
    return -1;
  }

  switch (whence) {
    case 0: // case of SEEK_SET
            // printf("from mylseek : we moove the pointer to the %d char\n",offset);
      openfiles[myfd].seek_pos = offset;
      break;
    case 1: // case of SEEK_CUR
            // printf("from mylseek we : moove the pointer to the curr pos of %d to %d char\n",openfiles[myfd].seek_pos , openfiles[myfd].seek_pos + offset);
      openfiles[myfd].seek_pos += offset; 
      break;
    case 2:
      // printf("from mylseek we : moove the pointer to the end pos at %d to %d char\n",dbs[inodes->first_block].last_pos , dbs[inodes->first_block].last_pos - offset );
      openfiles[myfd].seek_pos = dbs[inodes->first_block].last_pos - offset;
  }
  int seek_final = openfiles[myfd].seek_pos;
  return seek_final;
}

int find_fileodir(const char* pathname , int type, int flag){ // type = 1 for folder , type = 0 for file
  char separ = '/';
  int size = strlen(pathname);
  int type_c = FILE_T; 

  int i = 0;
  int nbr_dir = 1; // case of path with : folder/folder/../file
  if (pathname[0] == separ) { // case of  path with : /folder/folder/../file
    nbr_dir--;
  }

  // count the number of folder until we reach the choosen file / folder 
  while(i < size){
    if (pathname[i] == separ) {
      nbr_dir++;
    }
    i++;
  }

  if (nbr_dir == 1) {
    int it = 0;
    //if we have an only file but the path start with '/'
    if (pathname[0] == separ) {
      it=1;
    }
    char f_name[NAME];
    memset(f_name, 0, NAME);
    strncpy(f_name, pathname+it, size-it);
    for ( i = it; i < sb.num_inodes; i++) {
      if (strcmp(inodes[i].name,f_name)==0)  {
        //there is a match 
        if (inodes[i].ptr !=NULL) {
          type_c = FOLDER ;
          if (type_c == type) {
            return i;
          }
          else {
            if (flag != O_CREATE) {
              printf("the file doesn't exist and NON O_CREATE mode \n");
              return -1;
            }
            int ans = allocate_file(f_name);

            return ans;
          }
        }
        if (type_c == type) {
          return i;
        }else {
          if (flag != O_CREATE) {
            printf("the file doesn't exist and NON O_CREATE mode \n");
            return -1;
          }
          int ans = allocate_file(f_name);
          inodes[ans].ptr = malloc(sizeof(myDIR));
          inodes[ans].ptr->inode_pos = ans;
          inodes[ans].ptr->size =0;
          inodes[ans].ptr->inodes_list = malloc(sizeof(int)*10);
          return ans;
        }
      }

    }
    if (flag != O_CREATE) {
      printf("the file doesn't exist and NON O_CREATE mode \n");
      return -1;
    }
    int ans = allocate_file(f_name);
    if (type == FOLDER) {
      inodes[ans].ptr = malloc(sizeof(myDIR));
      inodes[ans].ptr->size = 0;
      inodes[ans].ptr->inode_pos = ans;
      inodes[ans].ptr->inodes_list = malloc(sizeof(int)*10);

      return ans;
    }else {
      return ans;
    }

  }

  char arr_name[nbr_dir][NAME];
  memset(arr_name, 0, 8*nbr_dir);

  i = 0; 
  int j = 0; 
  int k = 0; 
  if ( pathname[0] == separ) {
    j = 1;
  }

  char tmp_word[NAME];
  memset(tmp_word, 0, NAME);

  while (j <= size) {
    if (pathname[j] == separ || pathname[j] == '\0') {
      j++; // increment the main counter for pathname
      k=0; // reset the counter of char in folder 
      strcpy(arr_name[i], tmp_word);
      memset(tmp_word, 0, NAME);
      i++; // increment the num of folder in the arr
    }
    else {
      tmp_word[k] = pathname[j];
      j++; // increment to egt the next char of the folder/file
      k++; // increment to store the next char into the arr
    }
  }

  // at this state we have an array with all the directory for our path 

  myDIR * curr_dir = NULL;
  //we first reach the first directory 
  for ( i = 0; i < sb.num_inodes; i++) {
    // int size1 = strlen(arr_name[0]);
    // int size2 = strlen(inodes[i].name);
    if (strcmp(inodes[i].name , arr_name[0]) == 0){
      if (inodes[i].ptr != NULL) { // we find a match and the match is a folder 
        curr_dir = inodes[i].ptr;
        break;
      }
    }
  }
  // we didn't find a match for the first folder
  if (curr_dir == NULL) {
    printf("from find file or folder : the path is wrong\n");
    return -1;
  }

  j=1;
  //the first folder exist 

  if (nbr_dir > 2) {
    while (curr_dir != NULL && j < nbr_dir) {
      int place=-1;
      for (i = 0; i < curr_dir->size; i++) {
        place = curr_dir->inodes_list[i];
        //if we find that the next folder/file exist 
        // int size1 = strlen(inodes[place].name);
        // int size2 = strlen(arr_name[j]);
        if (strcmp(inodes[place].name,arr_name[j])==0){
          //check if it's a folder 
          if (inodes[place].ptr != NULL) {
            curr_dir = inodes[place].ptr;
            break;
          }
        }
        if (i == curr_dir->size) {
          printf("from find file or folder : we do not find the next step of the path\n");
          return -1;
        }
      }
      j++;
    }
  }

  // at this step we just need to check for the last element
  for (int i = 0; i < curr_dir->size; i++) {
    int place = curr_dir->inodes_list[i];
    // we find the a match
    if (strcmp(inodes[place].name,arr_name[nbr_dir-1])==0) {
      //it's a folder
      if (inodes[place].ptr != NULL) {
        type_c = FOLDER;
      }
      if (type_c == type) {
        return place;
      }
    }
  }

  curr_dir->size++; // we will add a file to the last folder of the path
  if (flag != O_CREATE) {
    printf("the file doesn't exist and NON O_CREATE mode \n");
    return -1;
  }
  int ans = allocate_file(arr_name[nbr_dir -1]);
  curr_dir->inodes_list[curr_dir->size-1] = ans;
  return ans;
}

myDIR * myopendir(const char *name){
  int ans = find_fileodir(name, FOLDER, O_CREATE);
  if (ans == -1) {
    printf("need to put O_CREATE mode");
    return NULL;
  }
  if (inodes[ans].ptr == NULL) {
    myDIR * dir = malloc(sizeof(myDIR));
    int* list = malloc(sizeof(int)*10);
    dir->inode_pos = ans ;
    dir->inodes_list = list;
    dir->size = 0;
    inodes[ans].ptr = dir;
  }
  int res = open_rec_dir(inodes[ans].ptr);
  if (res == -1) {
    printf("from open_dir : the folder is empty there is no need to open it\n");
    return inodes[ans].ptr;
  }else {
    return inodes[ans].ptr;
  }
}

int open_rec_dir(myDIR* curr_dir){
  if (curr_dir->size == 0) {
    printf("from open_rec : the folder is empty there is no need to open something\n");
    return -1;
  }
  for (int i = 0; i < curr_dir->size; i++) {
    int curr_fd = curr_dir->inodes_list[i];
    //there is another folder to open to into the main one 
    if (inodes[curr_fd].ptr != NULL) {
      myDIR * deep_fold = inodes[curr_fd].ptr;
      // add recursively all the file to open file 
      open_rec_dir(deep_fold);
      //add the folder to the openfiles 
      openfiles[curr_fd].inode_pos =curr_fd;
      openfiles[curr_fd].filenum = inodes[curr_fd].first_block;
      openfiles[curr_fd].seek_pos = 0;
    }else {
      printf("add %s to open file \n",inodes[curr_fd].name);
      add_to_openfiles(curr_fd);
    }
  }
  return 0;
}

mydirent *myreaddir(myDIR * dirp){
  char* name = inodes[dirp->inode_pos].name;
  mydirent * anotherDIR = malloc(sizeof(mydirent));
  strcpy(anotherDIR->name, name); 
  anotherDIR->fd = dirp->inode_pos;
  return anotherDIR;
}

int myclosedir(myDIR* dirp){
  if (dirp->size == 0) {
    printf("from open_rec : the folder is empty there is no need to open something\n");
    return -1;
  }
  for (int i = 0; i < dirp->size; i++) {
    int curr_fd = dirp->inodes_list[i];
    //if it's a folder
    if (inodes[curr_fd].ptr != NULL) {
      //case of a deeper folder 
      myDIR* deep_fold = inodes[curr_fd].ptr;
      //call recursively the function
      myclosedir(deep_fold);
      //close  the folder himself
      openfiles[curr_fd].seek_pos =0;
      openfiles[curr_fd].filenum =-1;
      openfiles[curr_fd].inode_pos = -1;
      printf("the folder %s was closed\n",inodes[curr_fd].name);
    }
    else {
      openfiles[curr_fd].inode_pos = -1;
      openfiles[curr_fd].filenum = -1;
      openfiles[curr_fd].seek_pos = 0;
      printf("the file %s was closed \n",inodes[curr_fd].name);
    }
  }
  return 0;
}

