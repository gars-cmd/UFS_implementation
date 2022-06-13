#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// thank https://www.youtube.com/watch?v=n2AAhiujAqs

#define BLOCKSIZE 512
#define NAME 8
#define MAX_FILES 10000 

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#define FOLDER 3
#define FILE_T 4
#define O_CREATE 5


// ############################## STRUCT-PART ############################## 

typedef struct superblock 
{
  int num_inodes;
  int num_blocks;
  int size_blocks;
} superblock;

typedef struct myDIR
{
  int size;
  int inode_pos;
  int* inodes_list;
}myDIR;

typedef struct inode{
  int size;
  int first_block;
  char name[NAME];
  myDIR* ptr; // need to implement default -> null if it' s a directory a pointer to the Mydir 
}inode;

typedef struct d_block{
  int next_block_num;
  char data[BLOCKSIZE];
  int last_pos;
}d_block;

typedef struct myopenfile
{
  int filenum;
  int inode_pos;
  int seek_pos;
}myopenfile;

typedef struct mydirent
{
  int fd;
  char name[NAME];
}mydirent;





// ############################## FUNCTIONS-PART ############################## 

void mymkfs(int size); //initialize a new filesytem 

int mymount(const char *source ,
const char * target , const char *filesytemtype,unsigned long mountflags);
void mount_fs(const char* src);//read to the filesytem
void print_fs(); //print the info about the filesytem
void sync_fs(const char* trg); //write to the filesytem 
int myopen (const char *pathname , int flags);//not done
int myclose(int myfd);//not done
ssize_t myread(int myfd , void *buf ,size_t count );//not done
ssize_t mywrite(int myfd ,const void *buf ,size_t count ); //not done
int allocate_file(const char name[NAME]); // find a place to allocate a new file
int find_empty_inode(); //find an empty inode
int find_empty_block(); // find an empty block
void set_filesize(int filenum , int  size );
void write_bytes(int filenum , int pos , char text);
void shorten_file(int bn);
int get_block_num(int file , int offset);
int add_to_openfiles(int fd);
int close_to_openfiles(int fd);
off_t mylseek(int myfd , off_t offset , int whence);
myDIR *myopendir(const char *name);
int find_fileodir(const char* pathname, int type, int flag);
int open_rec_dir(myDIR* curr_dir);
mydirent *myreaddir(myDIR * dirp);
int myclosedir(myDIR* dirp);
int close_rec_dir(myDIR* curr_dir);
