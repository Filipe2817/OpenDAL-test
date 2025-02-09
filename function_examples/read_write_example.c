#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include "opendal.h"

/*

- Read: Full read from a path with no configuration

- Write: Full write to a path with no configuration (resets when writing to the same path)

*/

opendal_bytes create_bytes(char* str) {
    return (opendal_bytes) {
        .data = (uint8_t*)str,
        .len = strlen(str)
    };
}

// OpenDAL is not guaranteed to null-terminate messages
void safe_printf_opendal_messages(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    while (*fmt) {
        if (*fmt == '%') { // Only handle %s and %B (opendal_bytes)
            fmt++;
            if (*fmt == 's') {
                const char* str = va_arg(args, const char*);
                fputs(str, stdout);
            } else {
                assert(*fmt == 'B');
                opendal_bytes bytes = va_arg(args, opendal_bytes);
                printf("%.*s", (int)bytes.len, bytes.data);
            }
            fmt++;
        } else {
            putchar(*fmt++);
        }
    }

    va_end(args);
}

// Test1: Read before writing (read from non-existent path)
void test1(opendal_operator *op, char *path) {
    opendal_result_read r = opendal_operator_read(op, path);
    assert(r.error != NULL);
    
    printf("Error: %u\n", r.error->code);
    opendal_bytes error_msg = r.error->message;
    safe_printf_opendal_messages("Error message: %B\n", error_msg);
    
    opendal_error_free(r.error);
}

// Test2: Write and read
void test2(opendal_operator *op, char *path, opendal_bytes data) {
    // Write
    opendal_error* error = opendal_operator_write(op, path, &data);
    assert(error == NULL);
    safe_printf_opendal_messages("Wrote to path (%s): %B\n", path, data);

    // Read
    opendal_result_read r = opendal_operator_read(op, path);
    assert(r.error == NULL);
    opendal_bytes read_data = r.data;
    assert(read_data.len == data.len);
    safe_printf_opendal_messages("Read from path (%s): %B\n", path, read_data);

    // Cleanup (Necessary to free the error when it exists)
    opendal_bytes_free(&read_data);
}

// Test3: Double write (bigger before smaller data)
void test3(opendal_operator *op, char *path, opendal_bytes data, opendal_bytes bigger_data) {
    opendal_error* error = opendal_operator_write(op, path, &bigger_data);
    assert(error == NULL);
    safe_printf_opendal_messages("Wrote to path (%s): %B\n", path, bigger_data);

    error = opendal_operator_write(op, path, &data);
    assert(error == NULL);
    safe_printf_opendal_messages("Wrote to path (%s): %B\n", path, data);

    opendal_result_read r = opendal_operator_read(op, path);
    assert(r.error == NULL);
    opendal_bytes read_data = r.data;
    safe_printf_opendal_messages("Read from path (%s): %B\n", path, read_data);

    opendal_bytes_free(&read_data);
}

// Test4: Write to multiple paths (chained) and special characters
void test4(opendal_operator *op, char *path1, char *path2, char *path3, opendal_bytes data, opendal_bytes bigger_data, opendal_bytes special_data) {
    // Write to paths
    opendal_error* error = opendal_operator_write(op, path1, &data);
    assert(error == NULL);
    safe_printf_opendal_messages("Wrote to path1 (%s): %B\n", path1, data);
    error = opendal_operator_write(op, path2, &bigger_data);
    assert(error == NULL);
    safe_printf_opendal_messages("Wrote to path2 (%s): %B\n", path2, bigger_data);
    error = opendal_operator_write(op, path3, &special_data);
    assert(error == NULL);
    safe_printf_opendal_messages("Wrote to path3 (%s): %B\n", path3, special_data);

    // Read from paths
    opendal_result_read r1 = opendal_operator_read(op, path1);
    opendal_result_read r2 = opendal_operator_read(op, path2);
    opendal_result_read r3 = opendal_operator_read(op, path3);
    assert(r1.error == NULL && r2.error == NULL && r3.error == NULL);

    // Check data
    opendal_bytes read_data1 = r1.data;
    opendal_bytes read_data2 = r2.data;
    opendal_bytes read_data3 = r3.data;
    assert(read_data1.len == data.len && read_data2.len == bigger_data.len && read_data3.len == special_data.len);
    safe_printf_opendal_messages("Read from path1 (%s): %B\n", path1, read_data1);
    safe_printf_opendal_messages("Read from path2 (%s): %B\n", path2, read_data2);
    safe_printf_opendal_messages("Read from path3 (%s): %B\n", path3, read_data3);

    // Cleanup
    opendal_bytes_free(&read_data1);
    opendal_bytes_free(&read_data2);
    opendal_bytes_free(&read_data3);
}

// Test5: Write bigger data (2^20 = 1048576 bytes = 1 MB)
void test5(opendal_operator *op, char *path) {
    int fd = open("function_examples/bytes.txt", O_RDONLY);
    assert(fd != -1);
    int size = 1048576;
    char buffer[size];

    int res = read(fd, &buffer[0], size);
    assert(res == size);
    close(fd);
    
    opendal_bytes data = {
        .data = (uint8_t*)buffer,
        .len = size
    };

    opendal_error* error = opendal_operator_write(op, path, &data);
    assert(error == NULL);
    printf("First 8 bytes written: %.8s\n", data.data);
    printf("Last 8 bytes written: %.8s\n", &data.data[size - 8]);

    opendal_result_read r = opendal_operator_read(op, path);
    assert(r.error == NULL);
    opendal_bytes read_data = r.data;
    assert(read_data.len == data.len);
    printf("First 8 bytes read: %.8s\n", read_data.data);
    printf("Last 8 bytes read: %.8s\n", &read_data.data[size - 8]);
    printf("Read data has the expected length: %zu\n", read_data.len);

    opendal_bytes_free(&read_data);
}

int main(void) {
    // Data
    char *str = "Test message";
    char *bigger_str = "A bigger text with more characters";
    char *special_str = "Text with some special characters: !@#$%^&*()_+☺";

    // Paths
    char *path = "/testpath";
    char *path2 = "/testpath/secondpath";
    char *path3 = "/testpath/testpath";

    // Create operator with memory backend
    opendal_result_operator_new result = opendal_operator_new("memory", NULL);
    assert(result.op != NULL);
    assert(result.error == NULL);

    // Get operator and data
    opendal_operator *op = result.op;
    opendal_bytes data = create_bytes(str);
    opendal_bytes bigger_data = create_bytes(bigger_str);
    opendal_bytes special_data = create_bytes(special_str);

    // Tests
    printf("\n------------ Test1: read non-existent path ---------------------\n\n");
    test1(op, path);
    printf("\n------------ Test2: write and read -----------------------------\n\n");
    test2(op, path, data);
    printf("\n------------ Test3: double write -------------------------------\n\n");
    test3(op, path, data, bigger_data);
    printf("\n------------ Test4: chained paths and special chars ------------\n\n");
    test4(op, path, path2, path3, data, bigger_data, special_data);
    printf("\n------------ Test5: write 1MB of data --------------------------\n\n");
    test5(op, path);
    printf("\n----------------------------------------------------------------\n\n");

    // Cleanup
    opendal_operator_free(op);
    return 0;
}
