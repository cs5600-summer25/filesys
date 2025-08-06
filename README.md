# Simple File System
*by Yixuan Sun, Yuchen J, Jiagyi Z*

A simulation for file system that provide generic file system commands

---

## Compile
```bash
make
```

## Run
```bash
./filesys
```

## Sample Output

#### `rm` command
```
FS> create apple
FS> ls
apple
FS> rm apple
File removed successfully
FS> mkdir apple
// error handling
FS> rm apple
File is a directory
FS> rm apple2
File does not exist
```
---
#### `rmdir` command
```
// handle non-empty directory
FS> mkdir apple
FS> ls
apple/
FS> cd apple
FS> create banana
FS> ls
banana
FS> home
FS> rmdir apple
Directory is not empty
FS> cd apple
FS> rm banana
File removed successfully
FS> home
FS> rmdir apple
Removed directory: apple
```
```
// handle other errors
FS> ls
file1
apple
apple2/
FS> rmdir apple
File is not a directory
FS> rmdir apple3
File does not exist
```
---
#### `tail` command
```
FS> cat file1
abcdefg
// if n is greater than file length, it would print out all the content
FS> tail file1 10
abcdefg
// if n = 0, it would print out an empty line
FS> tail file1 0

FS> create newFile
// if a file is newly created and has nothing, it would print out following line
FS> tail newFile 10
<File Empty>
// error handling
FS> tail no 10
File does not exist
FS> mkdir dir1
FS> tail dir1 10
File is a directory
```
---
#### `append` command

---

#### `stat` command
```
FS> create file1
FS> stat file1
Inode block: 2
Bytes in file: 0
FS> append file1 abc
FS> stat file1
Inode block: 2
Bytes in file: 3
FS> append file1 defg
FS> stat file1
Inode block: 2
Bytes in file: 7
```
---
#### `cat` command

```
FS> create file1
FS> append file1 abc
FS> append file1 defg
FS> cat file1
abcdefg
```