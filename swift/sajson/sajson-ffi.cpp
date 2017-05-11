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

// MARK: -
/// TODO: The following should be removed when they can be ported to Swift:

static sajson_value* wrap(sajson::value* doc) {
    return static_cast<sajson_value*>(doc);
}

static sajson::value* unwrap(sajson_value* value) {
    return static_cast<sajson::value*>(value);
}

const sajson_element* sajson_get_value_payload(struct sajson_value* value) {
    return unwrap(value)->_internal_get_payload();
}

uint8_t sajson_get_value_type(sajson_value* value) {
    return unwrap(value)->get_type();
}

sajson_value* sajson_create_value(size_t type, const size_t* payload, const unsigned char* input) {
    auto inputCasted = reinterpret_cast<const char*>(input);
    auto value = sajson::value(sajson::type(type), payload, inputCasted);
    return wrap(new(std::nothrow) sajson::value(std::move(value)));
}

sajson_value* sajson_object_get_value_of_key(sajson_value* parentValue, const char* key, size_t length) {
    auto value = unwrap(parentValue)->get_value_of_key(sajson::string(key, length));
    return wrap(new(std::nothrow) sajson::value(std::move(value)));
}
