#include <stdlib.h>
#include "heap.h"
#include <fs/vfs.h>
#include "fs/rfs.h"

fs_node_t * root_fs = 0;

void init_root_fs() 
{
	root_fs = create_rfs_directory("root");
}

fs_node_t * init_vfs() 
{
	if (root_fs != 0) return root_fs;

	init_root_fs();

	return root_fs;
}

int is_directory(fs_node_t * node) 
{

	if ((node->flags & FS_DIR) == FS_DIR) 
	{
		return 1;	
	}

	return 0;
}

uint32 read_fs(fs_node_t *node, uint32 offset, uint32 size, uint8 *buffer)
{

  // Has the node got a read callback?
  if (node->read != 0)
  {
    return node->read(node, offset, size, buffer);
  }
  else 
  {
    return 0;
  }

}

uint32 write_fs(fs_node_t *node, uint32 offset, uint32 size, uint8 *buffer)
{

  // Has the node got a read callback?
  if (node->write != 0)
  {
    return node->write(node, offset, size, buffer);
  }
  
  else
  {
    return 0;
  }

}

void open_fs(fs_node_t * node) {
	
	if (node->open != 0)
        {
		node->open(node);
	}

	return;
}

void close_fs(fs_node_t * node) {

	if (node->close != 0)
 	{
		node->close(node);
	}

	return;
}

struct dirent * readdir_fs (fs_node_t * node, uint32 idx) {

	if (node->readdir != 0)
	{
		return node->readdir(node, idx);
	}

	return 0;
}

fs_node_t * finddir_fs (fs_node_t * node, char * name) {

	if (node->finddir != 0) 
	{
		return node->finddir(node, name);
	}

	return 0;
}

void bindnode_fs(fs_node_t * node, fs_node_t * target) {

	if (node->bindnode != 0)
	{
		node->bindnode(node, target);
	}

}

void unbindnode_fs(fs_node_t * node, fs_node_t * target) {

	if (node->unbindnode != 0)
	{
		node->unbindnode(node, target);
	}

}