/*
 * Copyright (c) 2012 Chad Austin
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <ostream>

#include <string> // for error messages.  kill someday?

#define SAJSON_LIKELY(x) __builtin_expect(!!(x), 1)
#define SAJSON_UNLIKELY(x) __builtin_expect(!!(x), 0)

namespace sajson {
    enum type {
        TYPE_INTEGER = 0,
        TYPE_DOUBLE = 1,
        TYPE_NULL = 2,
        TYPE_FALSE = 3,
        TYPE_TRUE = 4,
        TYPE_STRING = 5,
        TYPE_ARRAY = 6,
        TYPE_OBJECT = 7,
    };

    inline std::ostream& operator<<(std::ostream& os, type t) {
        switch (t) {
            case TYPE_INTEGER: return os << "<integer>";
            case TYPE_DOUBLE:  return os << "<double>";
            case TYPE_NULL:    return os << "<null>";
            case TYPE_FALSE:   return os << "<false>";
            case TYPE_TRUE:    return os << "<true>";
            case TYPE_STRING:  return os << "<string>";
            case TYPE_ARRAY:   return os << "<array>";
            case TYPE_OBJECT:  return os << "<object>";
            default:           return os << "<unknown type";
        }
    }

    static const size_t TYPE_BITS = 3;
    static const size_t TYPE_SHIFT = sizeof(size_t) * 8 - TYPE_BITS;
    static const size_t TYPE_MASK = (1 << TYPE_BITS) - 1;
    static const size_t VALUE_MASK = size_t(-1) >> TYPE_BITS;

    static const size_t ROOT_MARKER = size_t(-1) & VALUE_MASK;

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

        string(); /*=delete*/
    };

    class literal : public string {
    public:
        explicit literal(const char* text)
            : string(text, strlen(text))
        {}
    };

    class refcount {
    public:
        refcount()
            : pn(new size_t(1))
        {}

        refcount(const refcount& rc)
            : pn(rc.pn)
        {
            ++*pn;
        }

        ~refcount() {
            if (--*pn == 0) {
                delete pn;
            }
        }

        size_t count() const {
            return *pn;
        }

    private:
        size_t* pn;

        refcount& operator=(const refcount&);
    };

    class mutable_string_view {
    public:
        mutable_string_view()
            : length(0)
            , data(0)
        {}

        mutable_string_view(const literal& s)
            : length(s.length())
        {
            data = new char[length];
            memcpy(data, s.data(), length);
        }

        mutable_string_view(const string& s)
            : length(s.length())
        {
            data = new char[length];
            memcpy(data, s.data(), length);
        }

        ~mutable_string_view() {
            if (uses.count() == 1) {
                delete[] data;
            }
        }

        size_t get_length() const {
            return length;
        }

        char* get_data() const {
            return data;
        }
        
    private:
        refcount uses;
        size_t length;
        char* data;
    };

    union integer_storage {
        int i;
        size_t u;
    };
    // TODO: reinstate with c++03 implementation
    //static_assert(sizeof(integer_storage) == sizeof(size_t), "integer_storage must have same size as one structure slot");

    union double_storage {
        enum {
            word_length = sizeof(double) / sizeof(size_t)
        };

        double d;
        size_t u[word_length];
    };
    // TODO: reinstate with c++03 implementation
    //static_assert(sizeof(double_storage) == sizeof(double), "double_storage should have same size as double");

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

        // valid iff get_type() is TYPE_ARRAY or TYPE_OBJECT
        size_t get_length() const {
            return payload[0];
        }

        // valid iff get_type() is TYPE_ARRAY
        value get_array_element(size_t index) const {
            size_t element = payload[1 + index];
            return value(get_element_type(element), payload + get_element_value(element), text);
        }

        // valid iff get_type() is TYPE_OBJECT
        string get_object_key(size_t index) const {
            const size_t* s = payload + 1 + index * 3;
            return string(text + s[0], s[1] - s[0]);
        }

        // valid iff get_type() is TYPE_OBJECT
        value get_object_value(size_t index) const {
            size_t element = payload[3 + index * 3];
            return value(get_element_type(element), payload + get_element_value(element), text);
        }

        // valid iff get_type() is TYPE_INTEGER
        int get_integer_value() const {
            integer_storage s;
            s.u = payload[0];
            return s.i;
        }

        // valid iff get_type() is TYPE_DOUBLE
        double get_double_value() const {
            double_storage s;
            for (unsigned i = 0; i < double_storage::word_length; ++i) {
                s.u[i] = payload[i];
            }
            return s.d;
        }

        // valid iff get_type() is TYPE_INTEGER or TYPE_DOUBLE
        double get_number_value() const {
            if (get_type() == TYPE_INTEGER) {
                return get_integer_value();
            } else {
                return get_double_value();
            }
        }

        // valid iff get_type() is TYPE_STRING
        size_t get_string_length() const {
            return payload[1] - payload[0];
        }

        // valid iff get_type() is TYPE_STRING
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
        explicit document(mutable_string_view& input, const size_t* structure, type root_type, const size_t* root, size_t error_line, size_t error_column, const std::string& error_message)
            : input(input)
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
            return value(root_type, root, input.get_data());
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
        mutable_string_view input;
        const size_t* const structure;
        const type root_type;
        const size_t* const root;
        const size_t error_line;
        const size_t error_column;
        const std::string error_message;
    };

    class parser {
    public:
        parser(const mutable_string_view& msv, size_t* structure)
            : input(msv)
            , input_end(input.get_data() + input.get_length())
            , structure(structure)
            , p(input.get_data())
            , temp(structure)
            , root_type(TYPE_NULL)
            , out(structure + input.get_length())
            , error_line(0)
            , error_column(0)
        {}

        document get_document() {
            if (parse()) {
                return document(input, structure, root_type, out, 0, 0, std::string());
            } else {
                delete[] structure;
                return document(input, 0, TYPE_NULL, 0, error_line, error_column, error_message);
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
                current_structure_type = TYPE_ARRAY;
            } else if (c == '{') {
                current_structure_type = TYPE_OBJECT;
            } else {
                return error("document root must be object or array");
            }
            ++p;

            size_t* current_base = temp;
            *temp++ = make_element(current_structure_type, ROOT_MARKER);

            parse_result result = error_result();
            
            for (;;) {
                char closing_bracket = (current_structure_type == TYPE_OBJECT ? '}' : ']');

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

                if (current_structure_type == TYPE_OBJECT && c != '}') {
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
                        next_type = TYPE_ARRAY;
                        goto push;
                    case '{':
                        next_type = TYPE_OBJECT;
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
                        if (current_structure_type == TYPE_ARRAY) {
                            structure_installer = &parser::install_array;
                            goto pop;
                        } else {
                            return error("expected }");
                        }
                    case '}':
                        if (current_structure_type == TYPE_OBJECT) {
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

        bool has_remaining_characters(ptrdiff_t remaining) {
            return input_end - p >= remaining;
        }

        parse_result parse_null() {
            if (SAJSON_UNLIKELY(!has_remaining_characters(4))) {
                return error("unexpected end of input");
            }
            char p1 = p[1];
            char p2 = p[2];
            char p3 = p[3];
            if (SAJSON_UNLIKELY(p1 != 'u' || p2 != 'l' || p3 != 'l')) {
                return error("expected 'null'");
            }
            p += 4;
            return TYPE_NULL;
        }

        parse_result parse_false() {
            if (SAJSON_UNLIKELY(!has_remaining_characters(5))) {
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
            return TYPE_FALSE;
        }

        parse_result parse_true() {
            if (SAJSON_UNLIKELY(!has_remaining_characters(4))) {
                return error("unexpected end of input");
            }
            char p1 = p[1];
            char p2 = p[2];
            char p3 = p[3];
            if (SAJSON_UNLIKELY(p1 != 'r' || p2 != 'u' || p3 != 'e')) {
                return error("expected 'true'");
            }
            p += 4;
            return TYPE_TRUE;
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
            double d = 0.0; // gcc complains that d might be used uninitialized which actually isn't true. but appease the warning.
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
                return TYPE_DOUBLE;
            } else {
                integer_storage is;
                is.i = i;

                *--out = is.u;
                return TYPE_INTEGER;
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

            return TYPE_ARRAY;
        }

        struct ObjectItemRecord {
            size_t key_start;
            size_t key_end;
            size_t value;
        };

        struct ObjectItemRecordComparator {
            ObjectItemRecordComparator(const char* input)
                : input(input)
            {}

            bool operator()(const ObjectItemRecord& left, const ObjectItemRecord& right) const {
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

        private:
            const char* input;
        };

        parse_result install_object(size_t* object_base) {
            const size_t length = (temp - object_base) / 3;
            ObjectItemRecord* oir = reinterpret_cast<ObjectItemRecord*>(object_base);
            std::sort(
                oir,
                oir + length,
                ObjectItemRecordComparator(input.get_data()));

            size_t* const new_base = out - length * 3 - 1;
            size_t i = length;
            while (i--) {
                // I think this addition is legal because the tag bits are at the top?
                *(--out) = *(--temp) + (object_base - new_base);
                *(--out) = *(--temp);
                *(--out) = *(--temp);
            }
            *(--out) = length;

            return TYPE_OBJECT;
        }

        parse_result parse_string(size_t* tag = 0) {
            if (!tag) {
                out -= 2;
                tag = out;
            }

            ++p; // "
            size_t start = p - input.get_data();
            for (;;) {
                if (SAJSON_UNLIKELY(p >= input_end)) {
                    return error("unexpected end of input");
                }

                if (SAJSON_UNLIKELY(*p >= 0 && *p < 0x20)) {
                    return error("illegal unprintable codepoint in string");
                }
            
                switch (*p) {
                    case '"':
                        tag[0] = start;
                        tag[1] = p - input.get_data();
                        ++p;
                        return TYPE_STRING;
                        
                    case '\\':
                        return parse_string_slow(tag, start);

                    default:
                        ++p;
                        break;
                }
            }
        }

        parse_result read_hex(unsigned& u) {
            unsigned v = 0;
            int i = 4;
            while (i--) {
                unsigned char c = *p++;
                if (c >= '0' && c <= '9') {
                    c -= '0';
                } else if (c >= 'a' && c <= 'f') {
                    c = c - 'a' + 10;
                } else if (c >= 'A' && c <= 'F') {
                    c = c - 'A' + 10;
                } else {
                    return error("invalid character in unicode escape");
                }
                v = (v << 4) + c;
            }

            u = v;
            return TYPE_NULL; // ???
        }

        void write_utf8(unsigned codepoint, char*& end) {
            if (codepoint < 0x80) {
                *end++ = codepoint;
            } else if (codepoint < 0x800) {
                *end++ = 0xC0 | (codepoint >> 6);
                *end++ = 0x80 | (codepoint & 0x3F);
            } else if (codepoint < 0x10000) {
                *end++ = 0xE0 | (codepoint >> 12);
                *end++ = 0x80 | ((codepoint >> 6) & 0x3F);
                *end++ = 0x80 | (codepoint & 0x3F);
            } else {
                assert(codepoint < 0x200000);
                *end++ = 0xF0 | (codepoint >> 18);
                *end++ = 0x80 | ((codepoint >> 12) & 0x3F);
                *end++ = 0x80 | ((codepoint >> 6) & 0x3F);
                *end++ = 0x80 | (codepoint & 0x3F);
            }
        }

        parse_result parse_string_slow(size_t* tag, size_t start) {
            char* end = p;
            
            for (;;) {
                if (SAJSON_UNLIKELY(p >= input_end)) {
                    return error("unexpected end of input");
                }

                if (SAJSON_UNLIKELY(*p < 0x20)) {
                    return error("illegal unprintable codepoint in string");
                }
            
                switch (*p) {
                    case '"':
                        tag[0] = start;
                        tag[1] = end - input.get_data();
                        ++p;
                        return TYPE_STRING;

                    case '\\':
                        ++p;
                        if (SAJSON_UNLIKELY(p >= input_end)) {
                            return error("unexpected end of input");
                        }

                        char replacement;
                        switch (*p) {
                            case '"': replacement = '"'; goto replace;
                            case '\\': replacement = '\\'; goto replace;
                            case '/': replacement = '/'; goto replace; 
                            case 'b': replacement = '\b'; goto replace;
                            case 'f': replacement = '\f'; goto replace;
                            case 'n': replacement = '\n'; goto replace;
                            case 'r': replacement = '\r'; goto replace;
                            case 't': replacement = '\t'; goto replace;
                            replace:
                                *end++ = replacement;
                                ++p;
                                break;
                            case 'u': {
                                ++p;
                                if (SAJSON_UNLIKELY(!has_remaining_characters(4))) {
                                    return error("unexpected end of input");
                                }
                                unsigned u = 0; // gcc's complaining that this could be used uninitialized. wrong.
                                parse_result result = read_hex(u);
                                if (!result) {
                                    return result;
                                }
                                if (u >= 0xD800 && u <= 0xDBFF) {
                                    if (SAJSON_UNLIKELY(!has_remaining_characters(6))) {
                                        return error("unexpected end of input during UTF-16 surrogate pair");
                                    }
                                    char p0 = p[0];
                                    char p1 = p[1];
                                    if (p0 != '\\' || p1 != 'u') {
                                        return error("expected \\u");
                                    }
                                    p += 2;
                                    unsigned v = 0; // gcc's complaining that this could be used uninitialized. wrong.
                                    result = read_hex(v);
                                    if (!result) {
                                        return result;
                                    }

                                    if (v < 0xDC00 || v > 0xDFFF) {
                                        return error("invalid UTF-16 trail surrogate");
                                    }
                                    u = 0x10000 + (((u - 0xD800) << 10) | (v - 0xDC00));
                                }
                                write_utf8(u, end);
                                break;
                            }
                            default:
                                return error("unknown escape");
                        }
                        break;
                        
                    default:
                        *end++ = *p++;
                        break;
                }
            }
        }

        mutable_string_view input;
        char* const input_end;
        size_t* const structure;

        char* p;
        size_t* temp;
        type root_type;
        size_t* out;
        size_t error_line;
        size_t error_column;
        std::string error_message;
    };

    template<typename StringType>
    document parse(const StringType& string) {
        mutable_string_view ms(string);

        size_t length = string.length();
        size_t* structure = new size_t[length];

        return parser(ms, structure).get_document();
    }
}
