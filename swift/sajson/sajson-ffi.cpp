#include "../../include/sajson.h"
#include "sajson_header_wrapper.h"

// never instantiated, only inherits so static_cast is legal
struct sajson_document: sajson::document {};

static sajson::document* unwrap(sajson_document* doc) {
    return static_cast<sajson::document*>(doc);
}

static sajson_document* wrap(sajson::document* doc) {
    return static_cast<sajson_document*>(doc);
}

sajson_document* sajson_parse_single_allocation(char* bytes, size_t length) {
    auto doc = sajson::parse(sajson::single_allocation(), sajson::mutable_string_view(length, bytes));
    return wrap(new(std::nothrow) sajson::document(std::move(doc)));
}

sajson_document* sajson_parse_dynamic_allocation(char* bytes, size_t length) {
    auto doc = sajson::parse(sajson::dynamic_allocation(), sajson::mutable_string_view(length, bytes));
    return wrap(new(std::nothrow) sajson::document(std::move(doc)));
}

void sajson_free_document(sajson_document* doc) {
    delete unwrap(doc);
}

bool sajson_has_error(sajson_document* doc) {
    return !unwrap(doc)->is_valid();
}

size_t sajson_get_error_line(sajson_document* doc) {
    return unwrap(doc)->get_error_line();
}

size_t sajson_get_error_column(sajson_document* doc) {
    return unwrap(doc)->get_error_column();
}

const char* sajson_get_error_message(sajson_document* doc) {
    return unwrap(doc)->get_error_message().c_str();
}

uint8_t sajson_get_root_type(sajson_document* doc) {
    return unwrap(doc)->_internal_get_root_type();
}

const size_t* sajson_get_root(sajson_document* doc) {
    return unwrap(doc)->_internal_get_root();
}

