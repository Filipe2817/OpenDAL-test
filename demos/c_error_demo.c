#include "assert.h"
#include "opendal.h"
#include "stdio.h"

int main() {
    /* Initialize a operator for "memory" backend, with no options */
    opendal_result_operator_new result = opendal_operator_new("memory", 0);
    assert(result.op != NULL);
    assert(result.error == NULL);

    opendal_operator* op = result.op;

    /* The read is supposed to fail */
    opendal_result_read r = opendal_operator_read(op, "/testpath");
    assert(r.error != NULL);
    assert(r.error->code == OPENDAL_NOT_FOUND);

    /* Lets print the error message out */
    struct opendal_bytes* error_msg = &r.error->message;
    for (int i = 0; i < error_msg->len; ++i) {
        printf("%c", error_msg->data[i]);
    }

    /* free the error since the error is not NULL */
    opendal_error_free(r.error);

    /* the operator_ptr is also heap allocated */
    opendal_operator_free(op);
}
