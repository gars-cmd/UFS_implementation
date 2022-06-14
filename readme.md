## Matala-7
---
For this seventh assignment we need 5o implement the unix file system .

### How to install
---

1. First you need to unzip the file 
2. Second run `make all`
3. Then run the test `./test`




###Explanation
---
Just explanation about few functions we implement:

- `mymkfs(size)` 
> this function set the number of inodes and blocks that will represent the files into our file system  
> we also init our inodes to default value. 
> **the max size for filename is 8**.
> the paramter size represent the total size of the file system.

- `sync_fs(target)`
> the function sync_fs open the file that will store our file system simulation.
> the function write into our file system the inodes and the blocks. 

- `mymount(source)`
> this function will read into the file system and create our array of inodes and our array of blocks where we stock our data 

- `myopen(fd,flag)`
> this function receive the parameter fd that represent the file descriptor of the file that we want to open.
> the parameter flag can be the macro O_CREATE that is 5 to open and create a file if he doesn't exist, every other number will open the file only if he exist.
> every file that is opened will be added to the array of openfiles.

- `myread(fd)`
> the function myread will check first for the presence of the fd in the array of openfiles only if it here the file will be read.

- `mywrite(fd,string,size)`
> the function mywrite will write size char into a buffer the block of the inode byte per byte into the buffer

- `myclose(fd)`
> the function myclose will close the file with the file descriptor fd only if the file is already open.

- `mylseek(fd , offset , whence)`
> the function mylseek will moove the pointer for read or write of a particular file to a specific index of the file.
> the whence parameter can be : 
> SEEK_SET to move the pointer to the char at place offset.
> SEEK_CUR to move the pointer to the curr pointer + offset.
> SEEK_END to move the pointer to the end of the file and go offset step back.

- `myopendir(name)`
> the function my open dir will open a directory and will open recursively all the file/directorys into it 
> if the directory doesn't exist in the given path the folder will be create.
> In this function when we create a folder we also initialize the parameter of the struct myDIR

- `myreaddir(dir)`
> the function myreaddir return a struct diernt that represent a dir and his name.

- `myclosedir()dir`
> the function will close recursively all the folder/files into the foldername/path given into the dir parameter.

### Notice
---
 
We only implement the first part of the assignment (without the part of myfile)
