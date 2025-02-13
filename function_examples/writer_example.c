#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "opendal.h"

/*

Streaming writes to a path. Supports writing data in chunks.

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
    uint32_t size = 1048576;
    char buffer1[size];

    int res = read(fd, &buffer1[0], size);
    assert(res == (int)size);
    close(fd);

    // Setup writer
    opendal_result_operator_writer result_writer = opendal_operator_writer(op, path);
    assert(result_writer.error == NULL);
    opendal_writer* writer = result_writer.writer;

    // Write the data in chunks
    uint16_t chunk_size = 1024;
    uint32_t total_written = 0;
    uint16_t total_chunks = 0;

    while (total_written < size) {
        opendal_bytes data = {
            .data = (uint8_t*)buffer1 + total_written,
            .len = chunk_size
        };
        opendal_result_writer_write write_result = opendal_writer_write(writer, &data);
        assert(write_result.error == NULL);
        assert(write_result.size == chunk_size);
        total_written += write_result.size;
        total_chunks++;
    }

    /**
     * OpenDAL writer free acts like a close operation.
     * Reading before closing will return an outdated result or an error.
     */
    opendal_writer_free(writer);
    printf("Expected %d chunks, wrote %d\n", size / chunk_size, total_chunks);

    // Read data
    opendal_result_read r = opendal_operator_read(op, path);
    assert(r.error == NULL);
    opendal_bytes read_data = r.data;
    printf("Read data length: %zu\n", read_data.len);

    // Cleanup
    opendal_bytes_free(&read_data);
    opendal_operator_free(op);
    return 0;
}
