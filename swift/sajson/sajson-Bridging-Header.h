//#include <stddef.h>

struct sajson_document;

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

struct sajson_document* sajson_parse_single_allocation(char* bytes, size_t length);
struct sajson_document* sajson_parse_dynamic_allocation(char* bytes, size_t length);
void sajson_free_document(struct sajson_document* doc);
bool sajson_has_error(struct sajson_document* doc);
size_t sajson_get_error_line(struct sajson_document* doc);
size_t sajson_get_error_column(struct sajson_document* doc);
const char* sajson_get_error_message(struct sajson_document* doc);
uint8_t sajson_get_root_type(struct sajson_document* doc);
const sajson_element* sajson_get_root(struct sajson_document* doc);

#ifdef __cplusplus
}
#endif
