/// Wraps the header file to expose C bindings.

#ifndef sajson_header_wrapper_h
#define sajson_header_wrapper_h

#ifdef __cplusplus
#else
#include <stddef.h>
#endif

struct sajson_document;
struct sajson_value;

// size_t gets turned into Int but we want unsigned on the Swift side
typedef unsigned long sajson_element;

#ifdef __cplusplus
static_assert(sizeof(sajson_element) == sizeof(size_t), "sajson_element should be pointer-sized and also convert to the right Swift types");

extern "C" {
#endif

    struct sajson_document* sajson_parse_single_allocation(char* bytes, size_t length);
    struct sajson_document* sajson_parse_dynamic_allocation(char* bytes, size_t length);
    void sajson_free_document(struct sajson_document* doc);
    int sajson_has_error(struct sajson_document* doc);
    size_t sajson_get_error_line(struct sajson_document* doc);
    size_t sajson_get_error_column(struct sajson_document* doc);
    const char* sajson_get_error_message(struct sajson_document* doc);
    uint8_t sajson_get_root_type(struct sajson_document* doc);
    const sajson_element* sajson_get_root(struct sajson_document* doc);
    const unsigned char* sajson_get_input(struct sajson_document* doc);
    size_t sajson_get_input_length(struct sajson_document* doc);

    // TODO: The following should be removed when they can be ported to Swift:
    const sajson_element* sajson_get_value_payload(struct sajson_value* value);
    uint8_t sajson_get_value_type(struct sajson_value* value);
    struct sajson_value* sajson_create_value(size_t type, const sajson_element* payload, const unsigned char* input);
    struct sajson_value* sajson_object_get_value_of_key(struct sajson_value* parent, const char* bytes, size_t length);

#ifdef __cplusplus
}
#endif


#endif /* sajson_header_wrapper_h */
