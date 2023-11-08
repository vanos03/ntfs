#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>


//mft_size == 0x400


uint64_t cluster_size = 0;
int fd = -1;
uint8_t *shared_cluster = NULL;
int mft_addr = 8*4*512;
int record_size = 0x400;

void open_img(char* name) {
	if( (-1) == (fd = open(name, O_RDONLY)) ) {
		perror(__func__);
		exit(-1);
	}

	uint8_t sector[512];
	if ( 512 != read(fd, sector, 512) ) {
		fprintf(stderr,"Error read boot sector\n");
		exit(-1);
	}

	uint16_t sector_size = *((uint16_t*)(sector + 0xb));
	uint8_t cluster_per_sector = sector[0xd];

	cluster_size = ((uint64_t)sector_size) * ((uint64_t)cluster_per_sector);
	if ( NULL == ( shared_cluster = malloc(cluster_size)) ) {
		perror(__func__);
		exit(-1);
	}
}

void read_cluster_in_place(uint64_t num) 
{
	uint64_t offset = num * record_size + mft_addr;
	if ( offset != lseek(fd, offset, SEEK_SET) ) {
		perror(__func__);
		exit(-1);
	}

	if ( cluster_size != read(fd, shared_cluster, cluster_size) ) {
		perror(__func__);
		exit(-1);
	}
}

/*
void read_mft(int number)
{
	uint8_t sector[512];
	lseek(fd, mft_addr + number * record_size, SEEK_SET);
	read(fd, sector, 512);
	write(1, sector, 512);
}
*/
int main(int argc, char* argv[])
{
	open_img(argv[1]);
	read_cluster_in_place(1);
//	read_mft(argv[2]);
	write(1, shared_cluster, 512);	

	return 0;
}




void __attribute__((destructor)) garbages_collector (void) 
{
	if ( fd > 0) {
		fprintf(stderr,"Close image file\n");
		close(fd);
	}

	if ( NULL != shared_cluster ) {
		fprintf(stderr,"Free shared memory\n");
		free(shared_cluster);
	}

}
