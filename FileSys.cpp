// Computing Systems: File System
// Implements the file system commands that are available to the shell.

#include <cstring>
#include <iostream>
using namespace std;

#include "BasicFileSys.h"
#include "Blocks.h"
#include "FileSys.h"

// mounts the file system
void FileSys::mount() {
  bfs.mount();
  curr_dir = 1;
}

// unmounts the file system
void FileSys::unmount() { bfs.unmount(); }

void FileSys::mkdir(const char *name) {
  // Check filename length
  if (strlen(name) > MAX_FNAME_SIZE) {
    cout << "Error: File name is too long" << endl;
    return;
  }

  dirblock_t curr;
  bfs.read_block(curr_dir, (void *)&curr);

  for (int i = 0; i < MAX_DIR_ENTRIES; ++i) {
    if (curr.dir_entries[i].block_num != 0 &&
        strcmp(curr.dir_entries[i].name, name) == 0) {
      cout << "Error: File exists" << endl;
      return;
    }
  }

  int free_index = -1;
  for (int i = 0; i < MAX_DIR_ENTRIES; ++i) {
    if (curr.dir_entries[i].block_num == 0) {
      free_index = i;
      break;
    }
  }

  if (free_index == -1) {
    cout << "Error: Directory is full" << endl;
    return;
  }

  short new_block = bfs.get_free_block();
  if (new_block == 0) {
    cout << "Error: Disk is full" << endl;
    return;
  }

  dirblock_t new_dir;
  memset(&new_dir, 0, sizeof(dirblock_t));
  new_dir.magic = DIR_MAGIC_NUM;
  new_dir.num_entries = 0;
  bfs.write_block(new_block, (void *)&new_dir);

  snprintf(curr.dir_entries[free_index].name,
           sizeof(curr.dir_entries[free_index].name), "%s", name);
  curr.dir_entries[free_index].block_num = new_block;
  curr.num_entries++;

  bfs.write_block(curr_dir, (void *)&curr);
}

// switch to a directory
void FileSys::cd(const char *name) {
  dirblock_t curr;
  bfs.read_block(curr_dir, (void *)&curr);

  for (int i = 0; i < MAX_DIR_ENTRIES; ++i) {
    if (curr.dir_entries[i].block_num != 0 &&
        strcmp(curr.dir_entries[i].name, name) == 0) {
      dirblock_t target;
      bfs.read_block(curr.dir_entries[i].block_num, (void *)&target);

      if (target.magic == DIR_MAGIC_NUM) {
        curr_dir = curr.dir_entries[i].block_num;  // switch to that directory
        return;
      } else {
        cout << "Error: File is not a directory" << endl;
        return;
      }
    }
  }

  cout << "Error: File does not exist" << endl;
}

// switch to home directory
void FileSys::home() { 
  curr_dir = 1; 
}

// remove a directory
void FileSys::rmdir(const char *name) {}

// list the contents of current directory
void FileSys::ls() {
  dirblock_t curr;
  bfs.read_block(curr_dir, (void *)&curr);

  for (int i = 0; i < MAX_DIR_ENTRIES; ++i) {
    short blk = curr.dir_entries[i].block_num;
    if (blk == 0) continue;

    dirblock_t dir;
    bfs.read_block(blk, (void *)&dir);

    if (dir.magic == DIR_MAGIC_NUM) {
      cout << curr.dir_entries[i].name << "/" << endl;
    } else {
      cout << curr.dir_entries[i].name << endl;
    }
  }
}

// create an empty data file
void FileSys::create(const char *name) {
  if (strlen(name) > MAX_FNAME_SIZE) {
    cout << "Error: Filename is too long" << endl;
    return;
  }

  // read current directory
  dirblock_t curr;
  bfs.read_block(curr_dir, (void *)&curr);

  // check if file already exists
  for (int i = 0; i < MAX_DIR_ENTRIES; ++i) {
    if (curr.dir_entries[i].block_num != 0 &&
        strcmp(curr.dir_entries[i].name, name) == 0) {
      cout << "Error: File exists" << endl;
      return;
    }
  }

  // find a free entry in the directory
  int free_index = -1;
  for (int i = 0; i < MAX_DIR_ENTRIES; ++i) {
    if (curr.dir_entries[i].block_num == 0) {
      free_index = i;
      break;
    }
  }
  if (free_index == -1) {
    cout << "Error: Directory is full" << endl;
    return;
  }

  // allocate inode
  short new_inode_block = bfs.get_free_block();
  if (new_inode_block == 0) {
    cout << "Error: Disk is full" << endl;
    return;
  }

  // initialize inode
  inode_t inode;
  memset(&inode, 0, sizeof(inode));
  inode.magic = INODE_MAGIC_NUM;
  inode.size = 0;
  for (int i = 0; i < MAX_DATA_BLOCKS; i++) {
    inode.blocks[i] = 0;
  }
  bfs.write_block(new_inode_block, (void *)&inode);

  strcpy(curr.dir_entries[free_index].name, name);
  curr.dir_entries[free_index].block_num = new_inode_block;
  curr.num_entries++;

  bfs.write_block(curr_dir, (void *)&curr);
}

// append data to a data file
void FileSys::append(const char *name, const char *data) {}

// display the contents of a data file
void FileSys::cat(const char *name) {}

// display the last N bytes of the file
void FileSys::tail(const char *name, unsigned int n) {}

// delete a data file
void FileSys::rm(const char *name) {}

// display stats about file or directory
void FileSys::stat(const char *name) {}

// HELPER FUNCTIONS (optional)
