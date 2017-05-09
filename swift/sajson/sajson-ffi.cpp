#include "../../include/sajson.h"
#include "sajson_header_wrapper.h"

char* copyStdStringToUnownedChar(std::string str) {
    char* result = new char(str.length() + 1); // +1 for null terminator
    str.copy(result, str.length());
    return result;
}

// never instantiated, only inherits so static_cast is legal
struct sajson_document: sajson::document {};

static sajson::document* unwrap(sajson_document* doc) {
    return static_cast<sajson::document*>(doc);
}

static sajson_document* wrap(sajson::document* doc) {
    return static_cast<sajson_document*>(doc);
}

struct sajson_value: sajson::value {};

static sajson_value* wrap(sajson::value* value) {
    return static_cast<sajson_value*>(value);
}

static sajson::value* unwrap(sajson_value* value) {
    return static_cast<sajson::value*>(value);
}

uint8_t sajson_get_element_type(size_t element) {
    return sajson::get_element_type(element);
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

// Note: The char array is unowned and the caller is expected to clean up the bytes.
char* sajson_get_error_message(sajson_document* doc) {
    return copyStdStringToUnownedChar(unwrap(doc)->get_error_message());
}

uint8_t sajson_get_root_type(sajson_document* doc) {
    return unwrap(doc)->_internal_get_root_type();
}

//const size_t* sajson_get_root(sajson_document* doc) {
sajson_value* sajson_get_root(sajson_document* doc) {
    auto value = unwrap(doc)->get_root();
    return wrap(new(std::nothrow) sajson::value(std::move(value)));
    //return unwrap(doc)->_internal_get_root();
}

//////

uint8_t sajson_get_value_type(sajson_value* value) {
    return unwrap(value)->get_type();
}

int sajson_value_get_integer_value(sajson_value* value) {
    return unwrap(value)->get_integer_value();
}

double sajson_value_get_double_value(sajson_value* value) {
    return unwrap(value)->get_double_value();
}

// Note: The char array is unowned and the caller is expected to clean up the bytes.
char* sajson_value_get_string_value(sajson_value* value) {
    return copyStdStringToUnownedChar(unwrap(value)->as_string());
}

size_t sajson_value_get_length(sajson_value* value) {
    return unwrap(value)->get_length();
}

sajson_value* sajson_value_get_array_element(sajson_value* value, size_t index) {
    auto element = unwrap(value)->get_array_element(index);
    return wrap(new(std::nothrow) sajson::value(std::move(element)));
}

// Note: The char array is unowned and the caller is expected to clean up the bytes.
char* sajson_value_get_object_key(sajson_value* value, size_t index) {
    return copyStdStringToUnownedChar(unwrap(value)->get_object_key(index).as_string());
}

sajson_value* sajson_value_get_object_value(sajson_value* value, size_t index) {
    auto element = unwrap(value)->get_object_value(index);
    return wrap(new(std::nothrow) sajson::value(std::move(element)));
}
