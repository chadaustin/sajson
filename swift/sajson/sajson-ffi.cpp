#include "../../include/sajson.h"
#include "sajson-Bridging-Header.h"

// never instantiated, only inherits so static_cast is legal
struct Document: sajson::document {};

static sajson::document* unwrap(Document* doc) {
    return static_cast<sajson::document*>(doc);
}

static Document* wrap(sajson::document* doc) {
    return static_cast<Document*>(doc);
}

Document* sajson_parse_single_allocation(char* bytes, size_t length) {
    auto doc = sajson::parse(sajson::single_allocation(), sajson::mutable_string_view(length, bytes));
    return wrap(new(std::nothrow) sajson::document(std::move(doc)));
}

Document* sajson_parse_dynamic_allocation(char* bytes, size_t length) {
    auto doc = sajson::parse(sajson::dynamic_allocation(), sajson::mutable_string_view(length, bytes));
    return wrap(new(std::nothrow) sajson::document(std::move(doc)));
}

void sajson_free_document(Document* doc) {
    delete unwrap(doc);
}

bool sajson_has_error(Document* doc) {
    return unwrap(doc)->is_valid();
}

size_t sajson_get_error_line(Document* doc) {
    return unwrap(doc)->get_error_line();
}

size_t sajson_get_error_column(Document* doc) {
    return unwrap(doc)->get_error_column();
}

const char* sajson_get_error_message(Document* doc) {
    return unwrap(doc)->get_error_message().c_str();
}
