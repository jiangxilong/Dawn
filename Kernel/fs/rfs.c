#include "rfs.h"
#include <stdlib.h>

rfs_t * rfs_entrys_pointer = 0;
uint32 num_rfs_entrys = 0;

struct dirent * root_readdir (fs_node_t * node, uint32 idx) {
	rfs_t * rfs_struct = &rfs_entrys_pointer[node->inode];

	if (rfs_struct->num_directory_entrys > idx) 
	{
		struct dirent * ret = malloc(sizeof(struct dirent));
		strcpy(ret->name, rfs_struct->directory_entrys[idx]->name);
		return ret;
	}

	return 0;
}

fs_node_t * root_finddir (fs_node_t * node, char * name) {
	rfs_t * rfs_struct = &rfs_entrys_pointer[node->inode];
	uint32 iter = 0;

	for (iter = 0; iter < rfs_struct->num_directory_entrys; iter++) 
	{
		if (strcmp(rfs_struct->directory_entrys[iter]->name, name) == 0) 
		{
			return rfs_struct->directory_entrys[iter];
		}
	}

	return 0;
}

void root_bind_node(fs_node_t * boundto, fs_node_t * node) {
	rfs_t * rfs_struct = &rfs_entrys_pointer[boundto->inode];

	rfs_struct->num_directory_entrys++;
	
	if (rfs_struct->directory_entrys == 0) //First bind
	{
		rfs_struct->directory_entrys = malloc(sizeof(fs_node_t *) * rfs_struct->num_directory_entrys);

		rfs_struct->directory_entrys[rfs_struct->num_directory_entrys - 1] = node;
		
	} else {
		fs_node_t ** dir_entrys_new = malloc(sizeof(fs_node_t *) * rfs_struct->num_directory_entrys);
		memcpy(dir_entrys_new, rfs_struct->directory_entrys, sizeof(fs_node_t *) * (rfs_struct->num_directory_entrys - 1));
		dir_entrys_new[rfs_struct->num_directory_entrys - 1] = node;

		free(rfs_struct->directory_entrys);
		rfs_struct->directory_entrys = dir_entrys_new;
	}
}

void root_unbind_node(fs_node_t * boundto, fs_node_t * node) {
	rfs_t * rfs_struct = &rfs_entrys_pointer[boundto->inode];

	uint32 nid = 0;
	fs_node_t * nptr = 0;

	for (nid = 0; nid < rfs_struct->num_directory_entrys; nid++) {
		nptr = rfs_struct->directory_entrys[nid];
		if (nptr == node) break;
		nptr = 0;	
	}

	if (nptr == 0) return; //No nptr, node isn't bound to the root.

	fs_node_t ** new_node_dir = malloc(sizeof(fs_node_t *) * rfs_struct->num_directory_entrys - 1);
	memcpy(new_node_dir, rfs_struct->directory_entrys, sizeof(fs_node_t *) * nid); //Copy up to the nid

	if ((nid + 1) != rfs_struct->num_directory_entrys) 
	{
		memcpy(new_node_dir + nid, rfs_struct->directory_entrys + (nid + 1), sizeof(fs_node_t *) * rfs_struct->num_directory_entrys - (nid + 1));
		free(rfs_struct->directory_entrys);
		rfs_struct->directory_entrys = (fs_node_t **) new_node_dir;
		rfs_struct->num_directory_entrys--;
	}
	else 
	{
		free(rfs_struct->directory_entrys);
		rfs_struct->directory_entrys = new_node_dir;
		rfs_struct->num_directory_entrys--;
	}
	
}

fs_node_t * create_rfs_directory(char * name) 
{
	num_rfs_entrys++;

	if (rfs_entrys_pointer == 0) 
	{
		rfs_entrys_pointer = malloc(sizeof(rfs_t) * num_rfs_entrys);
		memset(rfs_entrys_pointer, 0, sizeof(rfs_t) * num_rfs_entrys); //Init the ptr to null
		
		fs_node_t * ret = malloc(sizeof(fs_node_t));
		memset(ret, 0, sizeof(fs_node_t)); //Init it
		strcpy(ret->name, name);
		ret->flags |= FS_DIR;
		ret->inode = num_rfs_entrys - 1;

		ret->readdir = root_readdir;
		ret->finddir = root_finddir;
		
		ret->bindnode = root_bind_node;
		ret->unbindnode = root_unbind_node;
		return ret;

	} 
	else 
	{
		rfs_t * rfse_n = malloc(sizeof(rfs_t) * num_rfs_entrys);
		memcpy(rfse_n, rfs_entrys_pointer, sizeof(rfs_t) * (num_rfs_entrys - 1));

		rfse_n[num_rfs_entrys - 1].directory_entrys = 0;
		rfse_n[num_rfs_entrys - 1].num_directory_entrys = 0;

		free(rfs_entrys_pointer);
		rfs_entrys_pointer = rfse_n;

		fs_node_t * ret = malloc(sizeof(fs_node_t));
		memset(ret, 0, sizeof(fs_node_t)); //Init it
		strcpy(ret->name, name);
		ret->flags |= FS_DIR;
		ret->inode = num_rfs_entrys - 1;

		ret->readdir = root_readdir;
		ret->finddir = root_finddir;
		
		ret->bindnode = root_bind_node;
		ret->unbindnode = root_unbind_node;
		return ret;
	}
}