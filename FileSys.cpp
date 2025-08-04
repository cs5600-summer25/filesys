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
void FileSys::unmount() {
    bfs.unmount();
}

void FileSys::mkdir(const char *name) {
    // Check filename length
    if (strlen(name) > MAX_FNAME_SIZE) {
        cout << "Error: File name is too long" << endl;
        return;
    }

    dirblock_t curr;
    bfs.read_block(curr_dir, (void *) &curr);

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
    bfs.write_block(new_block, (void *) &new_dir);

    snprintf(curr.dir_entries[free_index].name, sizeof(curr.dir_entries[free_index].name), "%s", name);
    curr.dir_entries[free_index].block_num = new_block;
    curr.num_entries++;

    bfs.write_block(curr_dir, (void *) &curr);
}

// switch to a directory
void FileSys::cd(const char *name) {
    dirblock_t curr;
    bfs.read_block(curr_dir, (void *) &curr);

    for (int i = 0; i < MAX_DIR_ENTRIES; ++i) {
        if (curr.dir_entries[i].block_num != 0 &&
            strcmp(curr.dir_entries[i].name, name) == 0) {

            // Read target block to check if it's a directory
            dirblock_t target;
            bfs.read_block(curr.dir_entries[i].block_num, (void *) &target);

            if (target.magic == DIR_MAGIC_NUM) {
                curr_dir = curr.dir_entries[i].block_num; // switch to that directory
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
void FileSys::rmdir(const char *name) {
    // get the current dir
    dirblock_t curr_d;
    bfs.read_block(curr_dir, &curr_d);

    // loop through sub dir to see if there is a match
    for (int i = 0; i < curr_d.num_entries; i++) {

        if (strcmp(curr_d.dir_entries[i].name, name) == 0) {

            // if there is a matching name
            // First check if it is a directory
            short target_block_num = curr_d.dir_entries[i].block_num;
            if (!is_directory(target_block_num)) {
                cout << "File is not a directory" << endl;
                return;
            }

            dirblock_t target_dir;
            bfs.read_block(target_block_num, &target_dir);

            // if it is a directory
            // check if that directory contains nothing
            if (target_dir.num_entries != 0) {
                cout << "Directory is not empty" << endl;
                return;
            }

            // reclaim the block
            bfs.reclaim_block(target_block_num);

            // shift the entry array for current dir
            for (int j = i; j < curr_d.num_entries - 1; j++) {
                curr_d.dir_entries[j] = curr_d.dir_entries[j + 1];
            }

            // empty the last entry
            curr_d.dir_entries[curr_d.num_entries - 1].name[0] = '\0';
            curr_d.dir_entries[curr_d.num_entries - 1].block_num = 0;

            // decrement the counter for file under current directory
            curr_d.num_entries--;

            // save the new block to disk
            bfs.write_block(curr_dir, &curr_d);

            // for DEBUG
            cout << "Removed directory: " << name << endl;

            return;
        }

    }

    // if there is not matching name under current dir
    cout << "File does not exist" << endl;

    // handle
    // Directory is not empty dir_to_rm.entry != empty
    // File does not exist cannot find name
    // File is not a directory
}

// list the contents of current directory
void FileSys::ls() {
    dirblock_t curr;
    bfs.read_block(curr_dir, (void *) &curr);

    for (int i = 0; i < MAX_DIR_ENTRIES; ++i) {
        short blk = curr.dir_entries[i].block_num;
        if (blk == 0) continue;

        dirblock_t dir;
        bfs.read_block(blk, (void *) &dir);

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
    bfs.read_block(curr_dir, (void *) &curr);

    // check if file already exists
    for (int i = 0; i < MAX_DIR_ENTRIES; ++i) {
        if (curr.dir_entries[i].block_num != 0 &&
            strcmp(curr.dir_entries[i].name, name) == 0) {
            cout << "Error: File exists" << endl;
            return;
        }
    }

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
void FileSys::append(const char *name, const char *data) {
    dirblock_t curr;
    bfs.read_block(curr_dir, (void *) &curr);

    // Find the file
    int file_index = -1;
    for (int i = 0; i < MAX_DIR_ENTRIES; ++i) {
        if (curr.dir_entries[i].block_num != 0 &&
            strcmp(curr.dir_entries[i].name, name) == 0) {
            file_index = i;
            break;
        }
    }

    if (file_index == -1) {
        cout << "Error: File does not exist" << endl;
        return;
    }

    // Read the inode
    inode_t inode;
    bfs.read_block(curr.dir_entries[file_index].block_num, (void *) &inode);

    // Check if it's a file (not directory)
    if (inode.magic != INODE_MAGIC_NUM) {
        cout << "Error: File is a directory" << endl;
        return;
    }

    // Check if append would exceed maximum file size
    unsigned int data_len = strlen(data);
    if (inode.size + data_len > MAX_DATA_BLOCKS * BLOCK_SIZE) {
        cout << "Error: Append exceeds maximum file size" << endl;
        return;
    }

    // Append the data
    unsigned int data_pos = 0;
    while (data_pos < data_len) {
        // Find the last block or create new one
        int block_index = inode.size / BLOCK_SIZE;
        int offset_in_block = inode.size % BLOCK_SIZE;

        // If we need a new block
        if (offset_in_block == 0 || inode.blocks[block_index] == 0) {
            short new_block = bfs.get_free_block();
            if (new_block == 0) {
                cout << "Disk is full" << endl;
                return;
            }
            inode.blocks[block_index] = new_block;

            // Initialize new block
            datablock_t new_data_block;
            memset(&new_data_block, 0, sizeof(datablock_t));
            bfs.write_block(new_block, (void *) &new_data_block);
        }

        // Read current data block
        datablock_t data_block;
        bfs.read_block(inode.blocks[block_index], (void *) &data_block);

        // Copy data to block
        while (offset_in_block < BLOCK_SIZE && data_pos < data_len) {
            data_block.data[offset_in_block] = data[data_pos];
            offset_in_block++;
            data_pos++;
            inode.size++;
        }

        // Write block back
        bfs.write_block(inode.blocks[block_index], (void *) &data_block);
    }

    // Update inode
    bfs.write_block(curr.dir_entries[file_index].block_num, (void *) &inode);
}

// display the contents of a data file
void FileSys::cat(const char *name) {
}

// display the last N bytes of the file
void FileSys::tail(const char *name, unsigned int n) {
}

// delete a data file
void FileSys::rm(const char *name) {
}

// display stats about file or directory
void FileSys::stat(const char *name) {}

// HELPER FUNCTIONS (optional)
bool FileSys::is_directory(short block_num) {
    char buffer[BLOCK_SIZE];
    bfs.read_block(block_num, &buffer);

    unsigned int magic;
    memcpy(&magic, buffer, sizeof(magic));

    return magic == DIR_MAGIC_NUM;
}

