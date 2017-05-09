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
#if __LP64__
typedef unsigned long sajson_element;
#else
typedef unsigned int sajson_element;
#endif

#ifdef __cplusplus
static_assert(sizeof(sajson_element) == sizeof(size_t), "sajson_element should be pointer-sized and also convert to the right Swift types");

extern "C" {
#endif

    uint8_t sajson_get_element_type(size_t element);
    struct sajson_document* sajson_parse_single_allocation(char* bytes, size_t length);
    struct sajson_document* sajson_parse_dynamic_allocation(char* bytes, size_t length);
    void sajson_free_document(struct sajson_document* doc);
    int sajson_has_error(struct sajson_document* doc);
    size_t sajson_get_error_line(struct sajson_document* doc);
    size_t sajson_get_error_column(struct sajson_document* doc);
    char* sajson_get_error_message(struct sajson_document* doc);
    uint8_t sajson_get_root_type(struct sajson_document* doc);
    const sajson_element* sajson_get_root(struct sajson_document* doc);
    const unsigned char* sajson_get_input(struct sajson_document* doc);
    size_t sajson_get_input_length(struct sajson_document* doc);

    uint8_t sajson_get_value_type(struct sajson_value* value);
    int sajson_value_get_integer_value(struct sajson_value* value);
    double sajson_value_get_double_value(struct sajson_value* value);
    char* sajson_value_get_string_value(struct sajson_value* value);
    size_t sajson_value_get_length(struct sajson_value* value);
    struct sajson_value* sajson_value_get_array_element(struct sajson_value* value, size_t index);
    char* sajson_value_get_object_key(struct sajson_value* value, size_t index);
    struct sajson_value* sajson_value_get_object_value(struct sajson_value* value, size_t index);
#ifdef __cplusplus
}
#endif


#endif /* sajson_header_wrapper_h */
