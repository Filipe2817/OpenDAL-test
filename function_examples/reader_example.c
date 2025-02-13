#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "opendal.h"

/*

Streaming reads from a path. Supports reading data in chunks.

*/

int main(void) {
    char *path = "/testpath";
    
    // Create operator with memory backend
    opendal_result_operator_new result = opendal_operator_new("memory", NULL);
    assert(result.op != NULL);
    assert(result.error == NULL);
    opendal_operator *op = result.op;

    // Load Data
    int fd = open("function_examples/bytes.txt", O_RDONLY);
    assert(fd != -1);
    int size = 1048576;
    char buffer1[size];

    int res = read(fd, &buffer1[0], size);
    assert(res == size);
    close(fd);
    
    opendal_bytes data = {
        .data = (uint8_t*)buffer1,
        .len = size
    };

    // Write data
    opendal_error* error = opendal_operator_write(op, path, &data);
    assert(error == NULL);

    // Setup reader
    opendal_result_operator_reader result_reader = opendal_operator_reader(op, path);
    assert(result_reader.error == NULL);
    opendal_reader* reader = result_reader.reader;

    // Read the data in chunks
    uint16_t chunk_size = 1024;
    char buffer2[chunk_size];
    int total_read = 0;
    uint16_t total_chunks = 0;
    
    while (total_read < size) {
        opendal_result_reader_read read_result = opendal_reader_read(reader, (uint8_t*) buffer2, chunk_size);
        assert(read_result.error == NULL);
        assert(read_result.size == chunk_size);
        total_read += read_result.size;
        total_chunks++;
    }

    //printf("Last chunk:\n");
    //for (int i = 0; i < chunk_size; i++) {
    //    printf("%c", buffer2[i]);
    //}
    //printf("\n");
    printf("Expected %d chunks, got %d\n", size / chunk_size, total_chunks);

    // Cleanup
    opendal_reader_free(reader);
    opendal_operator_free(op);
    return 0;
}
