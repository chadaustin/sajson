#include <stddef.h>

struct Document;

#ifdef __cplusplus
extern "C" {
#endif

struct Document* sajson_parse_single_allocation(char* bytes, size_t length);
struct Document* sajson_parse_dynamic_allocation(char* bytes, size_t length);
void sajson_free_document(struct Document* doc);
bool sajson_has_error(struct Document* doc);
size_t sajson_get_error_line(struct Document* doc);
size_t sajson_get_error_column(struct Document* doc);
const char* sajson_get_error_message(struct Document* doc);

#ifdef __cplusplus
}
#endif
