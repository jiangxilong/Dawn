#ifndef _RFS_DEFINITION_H_
#define _RFS_DEFINITION_H_
#include <fs/vfs.h>

/* RFS is the name I gave to the internal temporary filesystem used by the OS which other filesystems are bound to
   the name stood for root FS, though its not just for the root file system any more, there seems to be no real reason to change the name
   RFS is only a simple VFS implementation that allows the creation of temporary directory structures by the OS for organizational purposes
   (And the binding of all file systems together), do not overestimate its capabilities...
 */

struct rfs_structure
{
	fs_node_t** directory_entrys; //The fs nodes in this directory
	uint32_t num_directory_entrys; //A int used to store the size of the array
};

typedef struct rfs_structure rfs_t;

fs_node_t* createRfsDirectory(char * name, fs_node_t* parent);

#endif //_RFS_DEFINITION_H_
