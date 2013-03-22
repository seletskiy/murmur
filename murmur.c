#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//
// Copyright Austin Appleby
// See: https://sites.google.com/site/murmurhash/
//
uint64_t murmur(char* buf, uint64_t buf_len, uint64_t seed)
{
	const uint64_t m = 0xc6a4a7935bd1e995LLU;
	const int r = 47;

	uint64_t h = seed ^ (buf_len * m);

	const uint64_t * data = (const uint64_t *)buf;
	const uint64_t * end = data + (buf_len / 8);

	while(data != end)
	{
		uint64_t k = *data++;

		k *= m;
		k ^= k >> r;
		k *= m;

		h ^= k;
		h *= m;
	}

	const unsigned char * data2 = (const unsigned char*)data;

	switch(buf_len & 7)
	{
	case 7: h ^= (uint64_t)data2[6] << 48;
	case 6: h ^= (uint64_t)data2[5] << 40;
	case 5: h ^= (uint64_t)data2[4] << 32;
	case 4: h ^= (uint64_t)data2[3] << 24;
	case 3: h ^= (uint64_t)data2[2] << 16;
	case 2: h ^= (uint64_t)data2[1] << 8;
	case 1: h ^= (uint64_t)data2[0];
			h *= m;
	};

	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}

int main(int argc, char** argv) {
    int fd;
    ssize_t n;
    uint64_t seed;
    int page_size = getpagesize();
    int off;
    struct stat st;
    char *buf;

    if (argc < 2) {
        printf("usage: %s file.ext\n", argv[0]);
        return 1;
    }

    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        printf("can not open file\n");
        return 2;
    }

    fstat(fd, &st);

    off = 0;
    seed = 0;
    while (off < st.st_size) {
        buf = mmap(NULL, page_size, PROT_READ, MAP_PRIVATE, fd, off);

        seed = murmur(buf,
            (st.st_size - off) > page_size ? page_size : (st.st_size - off),
            seed);
        munmap(buf, page_size);
    
        if (off && off % (page_size * 1024 * 10) == 0) {
            fprintf(stderr, "hashed %6i MB\n", (uint64_t)off / 1024 / 1024);
        }

        off += page_size;
    }

    printf("%llx\n", seed);

    close(fd);

    return 0;
}
