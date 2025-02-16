#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "opendal.h"

/*

Object manipulation, metadata and information retrieval.

Operations over directories are limited compared to files and can be confusing in some cases.

*/

void print_supported_capabilities(opendal_capability cap, int native) {
    if (native) {
        printf("=== Native Capabilities ===\n");
    } else {
        printf("=== Supported Capabilities ===\n");
    }

    if (cap.stat) printf("Stat\n");
    if (cap.stat_with_if_match) printf("Stat with If-Match\n");
    if (cap.stat_with_if_none_match) printf("Stat with If-None-Match\n");
    if (cap.read) printf("Read\n");
    if (cap.read_with_if_match) printf("Read with If-Match\n");
    if (cap.read_with_if_none_match) printf("Read with If-None-Match\n");
    if (cap.read_with_override_cache_control) printf("Read with Cache Control Override\n");
    if (cap.read_with_override_content_disposition) printf("Read with Content-Disposition Override\n");
    if (cap.read_with_override_content_type) printf("Read with Content-Type Override\n");
    if (cap.write) printf("Write\n");
    if (cap.write_can_multi) printf("Write (Multi-part)\n");
    if (cap.write_can_empty) printf("Write (Empty Allowed)\n");
    if (cap.write_can_append) printf("Write (Append)\n");
    if (cap.create_dir) printf("Create Directory\n");
    if (cap.delete_) printf("Delete\n");
    if (cap.copy) printf("Copy\n");
    if (cap.rename) printf("Rename\n");
    if (cap.list) printf("List\n");
    if (cap.list_with_limit) printf("List with Limit\n");
    if (cap.list_with_start_after) printf("List with Start After\n");
    if (cap.list_with_recursive) printf("List with Recursive\n");
    if (cap.presign) printf("Presign\n");
    if (cap.presign_read) printf("Presign Read\n");
    if (cap.presign_stat) printf("Presign Stat\n");
    if (cap.presign_write) printf("Presign Write\n");
    if (cap.shared) printf("Shared\n");
    if (cap.blocking) printf("Blocking\n");
    if (cap.write_multi_min_size > 0) printf("Write Multi Min Size: %lu bytes\n", cap.write_multi_min_size);
    if (cap.write_multi_max_size > 0) printf("Write Multi Max Size: %lu bytes\n", cap.write_multi_max_size);
    if (cap.write_total_max_size > 0) printf("Write Total Max Size: %lu bytes\n", cap.write_total_max_size);
	if (cap.write_with_cache_control) printf("Write with Cache Control\n");
    if (cap.write_with_content_disposition) printf("Write with Content Disposition\n");
    if (cap.write_with_content_type) printf("Write with Content Type\n");

    printf("============================\n");
}

void test_operator_info(opendal_operator *op) {
	opendal_operator_info *info = opendal_operator_info_new(op);

  	// Backend Workspace Name
	char *name = opendal_operator_info_get_name(info);
    assert(name != NULL);
  	printf("Backend Name: %s\n", name);
  	free(name);

	// Scheme (storage system)
	char *scheme = opendal_operator_info_get_scheme(info);
	assert(!strcmp(scheme, "memory"));
  	printf("Backend Scheme: %s\n", scheme);
	free(scheme);

	// Root Path
	char *root = opendal_operator_info_get_root(info);
	assert(!strcmp(root, "/myroot/"));
  	printf("Root Path: %s\n", root);
	free(root);

	// Full Capability (all OpenDAL features can support for the backend)
	opendal_capability full_cap = opendal_operator_info_get_full_capability(info);
    print_supported_capabilities(full_cap, 0);

	// Native Capability (features the backend supports natively)
	opendal_capability native_cap = opendal_operator_info_get_native_capability(info);
    print_supported_capabilities(native_cap, 1);

	opendal_operator_info_free(info);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void create_dir(opendal_operator *op, char *path) {
    opendal_error *error = opendal_operator_create_dir(op, path);
    assert(error == NULL);
    printf("Created directory: %s\n", path);
}

void exists_dir(opendal_operator *op, char *path) {
    opendal_result_exists exists = opendal_operator_exists(op, path);
    assert(exists.error == NULL);
    if (!exists.exists) {
        printf("Directory does not exist: %s\n", path);
    } else {
        printf("Directory exists: %s\n", path);
    }
}

void rename_object(opendal_operator *op, char *path, char *new_path) {
    opendal_error *error = opendal_operator_rename(op, path, new_path);
    if (error != NULL) {
        printf("Error: %u\n", error->code);
        opendal_bytes error_msg = error->message;
        printf("Error message: %.*s\n", (int)error_msg.len, error_msg.data);
        opendal_error_free(error);
    } else {
        printf("Renamed object: %s -> %s\n", path, new_path);
    }
}

void list_objects(opendal_operator *op, char *path, int identation) {
    opendal_result_list l = opendal_operator_list(op, path);
    assert(l.error == NULL);
    opendal_lister *lister = l.lister;
    opendal_result_lister_next next;
    opendal_entry *entry;

    while ((next = opendal_lister_next(lister)).entry != NULL) {
        entry = next.entry;
        char* entry_path = opendal_entry_path(entry);
        char* entry_name = opendal_entry_name(entry);

        opendal_result_stat s = opendal_operator_stat(op, entry_path);
        assert(s.error == NULL); 
        opendal_metadata *meta = s.meta;
        uint64_t length = opendal_metadata_content_length(meta);
        uint64_t last_modified = opendal_metadata_last_modified_ms(meta); // milliseconds

        if (!strcmp(entry_path, path)) { // Handle self entry to avoid infinite recursion
            printf("%*sSelf Entry: %s | Path: %s\n", identation , "", entry_name, entry_path);
        } else if (opendal_metadata_is_dir(meta)) {
            printf("%*sDirectory: %s | Path: %s | Content Length: %lu | Last Modified: %lu\n", identation, "", entry_name, entry_path, length, last_modified);
            list_objects(op, entry_path, identation + 4);
        } else {
            assert(opendal_metadata_is_file(meta));
            printf("%*sFile: %s | Path: %s | Content Length: %lu | Last Modified: %lu\n", identation, "", entry_name, entry_path, length, last_modified);
        }

        free(entry_path);
        free(entry_name);
        opendal_metadata_free(meta);
        opendal_entry_free(entry);
    }
    assert(next.error == NULL);
    opendal_lister_free(lister);
}

void delete_object(opendal_operator *op, char *path) {
    opendal_error *error = opendal_operator_delete(op, path);
    if (error != NULL) {
        printf("Error: %u\n", error->code);
        opendal_bytes error_msg = error->message;
        printf("Error message: %.*s\n", (int)error_msg.len, error_msg.data);
        opendal_error_free(error);
    } else {
        printf("Deleted object: %s\n", path);
    }
}

void test_object_operations(opendal_operator *op) {
    // Create directories
    create_dir(op, "/testdir1/");
    create_dir(op, "/testdir2/");
    create_dir(op, "/testdir1/subdir/");
    printf("\n");

    // Check if directories exist (can be applied to files)
    exists_dir(op, "/testdir1/");
    exists_dir(op, "/testdir2/");
    exists_dir(op, "/testdir1/subdir/");
    exists_dir(op, "/nonexistent/");
    printf("\n");

    // Create files (under testdir1)
    char* data = "Hello, World!";
    opendal_bytes bytes = { 
        .data = (uint8_t*)data,
        .len = 13
    };
    opendal_error *error = opendal_operator_write(op, "/testdir1/hello.txt", &bytes);
    assert(error == NULL);
    error = opendal_operator_write(op, "/testdir1/other.txt", &bytes);
    assert(error == NULL);

    // Rename objects
    // File
    rename_object(op, "/testdir1/hello.txt", "/testdir1/hello2.txt");
    // Directory
    rename_object(op, "/testdir1/", "/randomdir/");
    printf("\n");

    // List objects recursively + Metadata
    printf("Listing objects from root \"/\":\n");
    list_objects(op, "/", 4); // List root
    printf("\n");

    // Delete objects
    delete_object(op, "/testdir1/other.txt");
    delete_object(op, "/testdir1/subdir/");
    delete_object(op, "/testdir1/"); // Weird behavior, it deletes the self entry but not the directory itself (still contains hello.txt)
    delete_object(op, "/testdir2/");
    printf("\n");

    // List objects recursively + Metadata
    printf("Listing objects from root \"/\":\n");
    list_objects(op, "/", 4); // List root
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int main(void) {    
    // Create Operator with options
    opendal_operator_options *options = opendal_operator_options_new();
    opendal_operator_options_set(options, "root", "/myroot"); // key-value pairs

    opendal_result_operator_new result = opendal_operator_new("memory", options);
    opendal_operator *op = result.op;
    opendal_operator_options_free(options); // Options are no longer needed

    // Tests
    printf("\n------------ Test operator info --------------------------------\n\n");
    test_operator_info(op);
    printf("\n------------ Test object operations ----------------------------\n\n");
    test_object_operations(op);
    printf("\n----------------------------------------------------------------\n\n");

    opendal_operator_free(op);
    return 0;
}
