#include <stdio.h>
#include <memory.h>
#include <errno.h>

#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

int create_and_write_shared_memory(char *mmap_key, char *value, unsigned int size)
{
	int shm_fd; //this is the file descriptor for the shared memory in the kernel

	
	//CREATE
	/* *** Create a shared memory in the kernel space. It this shared memory already exists, then truncate it to 0 bytes again. *** */
	shm_fd=shm_open(mmap_key,O_CREAT | O_RDWR | O_TRUNC, 0660);

	if(shm_fd<0)
	{
		printf("Ooops..failure on shm_open on shm_fd. errcode=%d\n",errno);
		return -1;
	}


	//RESIZE
	/* *** After creation, the shared memory has the size 0 butes. Hence we need to resize it appropriately. *** */
	if(ftruncate(shm_fd,size)==-1)
	{
		printf("Error on ftruncate to allocate size of shared memory region.\n");
		return -1;
	}

	//MAP
	/* *** Map the shared memory which was created previously in the kernel space. Map it into the Virtual Address Space of the process *** */
	void *shm_reg=mmap(NULL, //let the operating system to decide the base address of the shared memory
				 size, //sizeof the shared memory to map to the virtual address space of the process
				 PROT_READ | PROT_WRITE, //shared memory is Read and Write
				 MAP_SHARED, //shared memory is accessible to different processes
				 shm_fd, //file descriptor of the shared memory from line 14
				 0); //offset from the base address of the physical/shared memory to be mapped


	/* *** shm_reg is the address in the virtual address space of the process, just like any other address.
	Linux paging mechanism maps this address to the starting address of the shared memory region in the kernel space.
	Any operation perfomed by the process on shm_reg is actually an operation performed in the shared memory that resides in the kernel/operating system space. *** */
	
	
	memset(shm_reg,0,size);// flush the segment of shared memory created because garbage data could exist there and we need it clean

	memcpy(shm_reg,value,size); // copy the data 'value'  to the shared memory region

	/* *** Destroy the mapping from the virtual address space of the process to the shared memory in kernel space *** */
	munmap(shm_reg,size);

	/* *** The reader will not be able to read shm if the writer unlinks
	***   shm_unlink() function API deassociates the shared memory object from its name, but it does not flush the content of the shared memory  */
	// shm_unlink(mmap_key); 


   /* *** close() function API decreases the reference count of the shared memory kernel resource. Every kernel resource has a reference count which accounts the process that use the resource.
   when reference count =0, then the operating system erases the content of the shared memory resources. Although, some of the operating systems do not flush the content of the kernel resource 
   until the next reboot *** */
    close(shm_fd);

	return size;

}



int read_from_shared_memory(char *mmap_key, char *buffer, unsigned int buffer_size, unsigned int bytes_to_read)
{
	int shm_fd=0; //this is the var for the file descriptor of the shared memory
	int rc=0; // this is the integer for the return from munmap() which is the success of un-mapping the shared memory

	//OPEN
	shm_fd=shm_open(mmap_key, O_CREAT | O_RDONLY, 0660); //Open the shared memory region from which it wishes to read the data

	if(shm_fd<0)
	{
		printf("Ooops...failure in shm_open on shm_fd with error code=%d", errno);
		return -1;
	}


	//MAP
	void *shm_reg=mmap(NULL, bytes_to_read, PROT_READ, MAP_SHARED, shm_fd,0);//using the shm_reg pointer we can access the shared memory in the kernel space

	if(shm_reg==MAP_FAILED)
	{
		printf("Error on mapping.\n");
		return -1;
	}

	//COPY DATA FROM SHARED MEMORY REGION TO LOCAL BUFFER
	memcpy(buffer,shm_reg,bytes_to_read);

	//UNMAP the virtual address space of the process with the shared memory in the kernel
	rc=munmap(shm_reg,bytes_to_read);


	if(rc<0)
	{
		printf("munmap failed\n");
		return -1;
	}

	close(shm_fd);
	return bytes_to_read;
}










