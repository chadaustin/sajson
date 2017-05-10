#include "../../include/sajson.h"
#include "sajson_header_wrapper.h"

// never instantiated, only inherits so static_cast is legal
struct sajson_document: sajson::document {};
struct sajson_value: sajson::value {};

static sajson::document* unwrap(sajson_document* doc) {
    return static_cast<sajson::document*>(doc);
}

static sajson_document* wrap(sajson::document* doc) {
    return static_cast<sajson_document*>(doc);
}

static sajson_value* wrap(sajson::value* doc) {
    return static_cast<sajson_value*>(doc);
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

int sajson_has_error(sajson_document* doc) {
    return !unwrap(doc)->is_valid();
}

size_t sajson_get_error_line(sajson_document* doc) {
    return unwrap(doc)->get_error_line();
}

size_t sajson_get_error_column(sajson_document* doc) {
    return unwrap(doc)->get_error_column();
}

const char* sajson_get_error_message(sajson_document* doc) {
    // This is okay because get_error_message returns a reference to the internal std::string
    return unwrap(doc)->get_error_message().c_str();
}

uint8_t sajson_get_root_type(sajson_document* doc) {
    return unwrap(doc)->_internal_get_root_type();
}

const size_t* sajson_get_root(sajson_document* doc) {
    return unwrap(doc)->_internal_get_root();
}

const unsigned char* sajson_get_input(sajson_document* doc) {
    return reinterpret_cast<const unsigned char*>(
        unwrap(doc)->_internal_get_input().get_data());
}

size_t sajson_get_input_length(struct sajson_document* doc) {
    return unwrap(doc)->_internal_get_input().length();
}

sajson_value* sajson_object_get_value_of_key(sajson_value* parent, const char* key, size_t length) {
    auto value = parent->get_value_of_key(sajson::string(key, length));
    return wrap(new(std::nothrow) sajson::value(std::move(value)));
}
