#pragma once

#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#include <string> // for error messages.  kill someday?

#define SAJSON_LIKELY(x) __builtin_expect(!!(x), 1)
#define SAJSON_UNLIKELY(x) __builtin_expect(!!(x), 0)

namespace sajson {
    enum class type : unsigned char {
        integer = 0,
        double_ = 1,
        null = 2,
        false_ = 3,
        true_ = 4,
        string = 5,
        array = 6,
        object = 7,
    };

    constexpr size_t TYPE_BITS = 3;
    constexpr size_t TYPE_SHIFT = sizeof(size_t) * 8 - TYPE_BITS;
    constexpr size_t TYPE_MASK = (1 << TYPE_BITS) - 1;
    constexpr size_t VALUE_MASK = (~size_t(0u)) >> TYPE_BITS;

    constexpr size_t ROOT_MARKER = size_t(-1) & VALUE_MASK;

    inline type get_element_type(size_t s) {
        return static_cast<type>((s >> TYPE_SHIFT) & TYPE_MASK);
    }

    inline size_t get_element_value(size_t s) {
        return s & VALUE_MASK;
    }

    inline size_t make_element(type t, size_t value) {
        //assert(value & VALUE_MASK == 0);
        //value &= VALUE_MASK;
        return value | (static_cast<size_t>(t) << TYPE_SHIFT);
    }

    class string {
    public:
        string() = delete;

        string(const char* text, size_t length)
            : text(text)
            , _length(length)
        {}

        const char* data() const {
            return text;
        }

        const size_t length() const {
            return _length;
        }

        std::string as_string() const {
            return std::string(text, text + _length);
        }

    private:
        const char* const text;
        const size_t _length;
    };

    class literal : public string {
    public:
        explicit literal(const char* text)
            : string(text, strlen(text))
        {}
    };

    union integer_storage {
        int i;
        size_t u;
    };
    static_assert(sizeof(integer_storage) == sizeof(size_t), "integer_storage must have same size as one structure slot");

    union double_storage {
        static constexpr size_t word_length = sizeof(double) / sizeof(size_t);

        double d;
        size_t u[word_length];
    };
    static_assert(sizeof(double_storage) == sizeof(double), "double_storage should have same size as double");

    class value {
    public:
        explicit value(type value_type, const size_t* payload, const char* text)
            : value_type(value_type)
            , payload(payload)
            , text(text)
        {}

        type get_type() const {
            return value_type;
        }

        // valid iff get_type() is type::array or type::object
        size_t get_length() const {
            return payload[0];
        }

        // valid iff get_type() is type::array
        value get_array_element(size_t index) const {
            size_t element = payload[1 + index];
            return value(get_element_type(element), payload + get_element_value(element), text);
        }

        // valid iff get_type() is type::object
        string get_object_key(size_t index) const {
            const size_t* s = payload + 1 + index * 3;
            return string(text + s[0], s[1] - s[0]);
        }

        // valid iff get_type() is type::object
        value get_object_value(size_t index) const {
            size_t element = payload[3 + index * 3];
            return value(get_element_type(element), payload + get_element_value(element), text);
        }

        // valid iff get_type() is type::integer
        int get_integer_value() const {
            integer_storage s;
            s.u = payload[0];
            return s.i;
        }

        // valid iff get_type() is type::double_
        double get_double_value() const {
            double_storage s;
            for (unsigned i = 0; i < double_storage::word_length; ++i) {
                s.u[i] = payload[i];
            }
            return s.d;
        }

        // valid iff get_type() is type::integer or type::double_
        double get_number_value() const {
            if (get_type() == type::integer) {
                return get_integer_value();
            } else {
                return get_double_value();
            }
        }

        // valid iff get_type() is type::string
        size_t get_string_length() const {
            return payload[1] - payload[0];
        }

        // valid iff get_type() is type::string
        std::string as_string() const {
            return std::string(text + payload[0], text + payload[1]);
        }

    private:
        const type value_type;
        const size_t* const payload;
        const char* const text;
    };

    class document {
    public:
        explicit document(const char* text, const size_t* structure, type root_type, const size_t* root, size_t error_line, size_t error_column, const std::string& error_message)
            : text(text)
            , structure(structure)
            , root_type(root_type)
            , root(root)
            , error_line(error_line)
            , error_column(error_column)
            , error_message(error_message)
        {}

        ~document() {
            delete[] structure;
        }

        bool is_valid() const {
            return !!structure;
        }

        value get_root() const {
            return value(root_type, root, text);
        }

        size_t get_error_line() const {
            return error_line;
        }

        size_t get_error_column() const {
            return error_column;
        }

        std::string get_error_message() const {
            return error_message;
        }

    private:
        const char* const text;
        const size_t* const structure;
        const type root_type;
        const size_t* const root;
        const size_t error_line;
        const size_t error_column;
        const std::string error_message;
    };

    class parser {
    public:
        parser(const char* input, size_t length, size_t* structure)
            : input(input)
            , input_end(input + length)
            , structure(structure)
            , p(input)
            , temp(structure)
            , root_type(type::null)
            , out(structure + length)
        {}

        document get_document() {
            if (parse()) {
                return document(input, structure, root_type, out, 0, 0, std::string());
            } else {
                delete[] structure;
                return document(0, 0, type::null, 0, error_line, error_column, error_message);
            }
        }

    private:
        struct error_result {
            operator bool() const {
                return false;
            }
        };

        struct parse_result {
            parse_result(error_result)
                : success(false)
            {}

            parse_result(type t)
                : success(true)
                , value_type(t)
            {}

            bool operator!() const {
                return !success;
            }

            bool success;
            type value_type;
        };

        bool at_eof() {
            return p == input_end;
        }

        char peek_structure() {
            for (;;) {
                if (p == input_end) {
                    // 0 is never legal as a structural character in json text so treat it as eof
                    return 0;
                }
                switch (*p) {
                    case 0x20:
                    case 0x09:
                    case 0x0A:
                    case 0x0D:
                        ++p;
                        continue;
                    default:
                        return *p;
                }
            }                
        }

        char peek() {
            assert(p < input_end);
            return *p;
        }

        void consume() {
            assert(p < input_end);
            ++p;
        }

        error_result error(const char* message) {
            error_line = 1;
            error_column = 1;
            error_message = message;
            return error_result();
        }

        bool parse() {
            char c = peek_structure();
            if (c == 0) {
                return error("no root element");
            }

            type current_structure_type;
            if (c == '[') {
                current_structure_type = type::array;
            } else if (c == '{') {
                current_structure_type = type::object;
            } else {
                return error("document root must be object or array");
            }
            ++p;

            size_t* current_base = temp;
            *temp++ = make_element(current_structure_type, ROOT_MARKER);

            parse_result result = error_result();
            
            for (;;) {
                char closing_bracket = (current_structure_type == type::object ? '}' : ']');

                c = peek_structure();
                if (temp > current_base + 1) {
                    if (c != closing_bracket) {
                        if (c == ',') {
                            ++p;
                            c = peek_structure();
                        } else {
                            return error("expected ,");
                        }
                    }
                }

                if (current_structure_type == type::object && c != '}') {
                    if (c != '"') {
                        return error("object key must be quoted");
                    }
                    result = parse_string(temp);
                    if (peek_structure() != ':') {
                        return error("expected :");
                    }
                    ++p;
                    temp += 2;
                }

                switch (peek_structure()) {
                    type next_type;
                    parse_result (parser::*structure_installer)(size_t* base);

                    case 0:
                        return error("unexpected end of input");
                    case 'n':
                        result = parse_null();
                        break;
                    case 'f':
                        result = parse_false();
                        break;
                    case 't':
                        result = parse_true();
                        break;
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                    case '-':
                        result = parse_number();
                        break;
                    case '"':
                        result = parse_string();
                        break;

                    case '[':
                        next_type = type::array;
                        goto push;
                    case '{':
                        next_type = type::object;
                        goto push;
                    push: {
                        ++p;
                        size_t* previous_base = current_base;
                        current_base = temp;
                        *temp++ = make_element(current_structure_type, previous_base - structure);
                        current_structure_type = next_type;
                        continue;
                    }

                    case ']':
                        if (current_structure_type == type::array) {
                            structure_installer = &parser::install_array;
                            goto pop;
                        } else {
                            return error("expected }");
                        }
                    case '}':
                        if (current_structure_type == type::object) {
                            structure_installer = &parser::install_object;
                            goto pop;
                        } else {
                            return error("expected ]");
                        }
                    pop: {
                        ++p;
                        size_t element = *current_base;
                        result = (this->*structure_installer)(current_base + 1);
                        size_t parent = get_element_value(element);
                        if (parent == ROOT_MARKER) {
                            root_type = result.value_type;
                            goto done;
                        }
                        temp = current_base;
                        current_base = structure + parent;
                        current_structure_type = get_element_type(element);
                        break;
                    }
                    default:
                        printf("%c\n", *p);
                        return error("cannot parse unknown value");
                }

                if (!result) {
                    return result.success;
                }

                *temp++ = make_element(result.value_type, out - current_base - 1);
            }

        done:
            if (0 == peek_structure()) {
                return true;
            } else {
                return error("expected end of input");
            }
        }

        parse_result parse_null() {
            if (SAJSON_UNLIKELY(4 > input_end - p)) {
                return error("unexpected end of input");
            }
            char p1 = p[1];
            char p2 = p[2];
            char p3 = p[3];
            if (SAJSON_UNLIKELY(p1 != 'u' || p2 != 'l' || p3 != 'l')) {
                return error("expected 'null'");
            }
            p += 4;
            return type::null;
        }

        parse_result parse_false() {
            if (SAJSON_UNLIKELY(5 > input_end - p)) {
                return error("unexpected end of input");
            }
            char p1 = p[1];
            char p2 = p[2];
            char p3 = p[3];
            char p4 = p[4];
            if (SAJSON_UNLIKELY(p1 != 'a' || p2 != 'l' || p3 != 's' || p4 != 'e')) {
                return error("expected 'false'");
            }
            p += 5;
            return type::false_;
        }

        parse_result parse_true() {
            if (SAJSON_UNLIKELY(4 > input_end - p)) {
                return error("unexpected end of input");
            }
            char p1 = p[1];
            char p2 = p[2];
            char p3 = p[3];
            if (SAJSON_UNLIKELY(p1 != 'r' || p2 != 'u' || p3 != 'e')) {
                return error("expected 'true'");
            }
            p += 4;
            return type::true_;
        }
        
        parse_result parse_number() {
            bool negative = false;
            if ('-' == peek()) {
                ++p;
                negative = true;
            }

            // TODO: eof check

            bool try_double = false;

            int i = 0;
            double d;
            for (;;) {
                // TODO: eof check

                char c = peek();
                if (c < '0' || c > '9') {
                    break;
                }
                
                consume();
                char digit = c - '0';

                if (SAJSON_UNLIKELY(i > INT_MAX / 10 - 9)) {
                    try_double = true;
                    d = 10.0 * i + digit;
                    break;
                } else {
                    i = 10 * i + digit;
                }
            }

            if ('.' == peek()) {
                if (!try_double) {
                    try_double = true;
                    d = i;
                }
                consume();
                double place = 0.1;
                for (;;) {
                    char c = peek();
                    if (c >= '0' && c <= '9') {
                        consume();
                        d += (c - '0') * place;
                        place *= 0.1;
                    } else {
                        break;
                    }
                }
            }

            if (negative) {
                if (try_double) {
                    d = -d;
                } else {
                    i = -i;
                }
            }

            char e = peek();
            if ('e' == e || 'E' == e) {
                if (!try_double) {
                    try_double = true;
                    d = i;
                }
                consume();
                double exponentSign = 1;
                if ('-' == peek()) {
                    consume();
                    exponentSign = -1;
                } else if ('+' == peek()) {
                    consume();
                }

                double exponent = 0;
                for (;;) {
                    char c = peek();
                    if (c >= '0' && c <= '9') {
                        consume();
                        exponent = 10 * exponent + (c - '0');
                    } else {
                        break;
                    }
                }
                d *= pow(10.0, exponentSign * exponent);
            }

            if (try_double) {
                double_storage ns;
                ns.d = d;

                out -= ns.word_length;
                for (int i = 0; i < ns.word_length; ++i) {
                    out[i] = ns.u[i];
                }
                return type::double_;
            } else {
                integer_storage is;
                is.i = i;

                *--out = is.u;
                return type::integer;
            }
        }

        parse_result install_array(size_t* array_base) {
            const size_t length = temp - array_base;
            size_t* const new_base = out - length - 1;
            while (temp > array_base) {
                // I think this addition is legal because the tag bits are at the top?
                *(--out) = *(--temp) + (array_base - new_base);
            }
            *(--out) = length;

            return type::array;
        }

        struct ObjectItemRecord {
            size_t key_start;
            size_t key_end;
            size_t value;
        };

        parse_result install_object(size_t* object_base) {
            const size_t length = (temp - object_base) / 3;
            ObjectItemRecord* oir = reinterpret_cast<ObjectItemRecord*>(object_base);
            std::sort(
                oir,
                oir + length,
                [=](const ObjectItemRecord& left, const ObjectItemRecord& right) -> bool {
                    size_t left_length = left.key_end - left.key_start;
                    size_t right_length = right.key_end - right.key_start;
                    if (left_length < right_length) {
                        return true;
                    } else if (left_length > right_length) {
                        return false;
                    } else {
                        return memcmp(input + left.key_start, input + right.key_start, left_length) < 0;
                    }
                }
            );

            size_t* const new_base = out - length * 3 - 1;
            size_t i = length;
            while (i--) {
                // I think this addition is legal because the tag bits are at the top?
                *(--out) = *(--temp) + (object_base - new_base);
                *(--out) = *(--temp);
                *(--out) = *(--temp);
            }
            *(--out) = length;

            return type::object;
        }

        parse_result parse_string(size_t* tag = 0) {
            consume();
            size_t start = p - input;
            for (;;) {
                if ('"' == peek()) {
                    if (!tag) {
                        out -= 2;
                        tag = out;
                    }
                    tag[0] = start;
                    tag[1] = p - input;
                    consume();
                    return type::string;
                } else {
                    consume();
                }
            }
        }

        const char* const input;
        const char* const input_end;
        size_t* const structure;

        const char* p;
        size_t* temp;
        type root_type;
        size_t* out;
        size_t error_line = 0;
        size_t error_column = 0;
        std::string error_message;
    };

    template<typename StringType>
    document parse(const StringType& string) {
        size_t length = string.length();
        size_t* structure = new size_t[length];

        return parser(string.data(), length, structure).get_document();
    }
}
