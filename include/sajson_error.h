#pragma once

#include "sajson.h"

#include <string>
#include <sstream>

namespace sajson {
    const char* get_error_text(error code) {
        switch(code) {
            case ERROR_SUCCESS: return "no error";
            case ERROR_OUT_OF_MEMORY: return  "out of memory";
            case ERROR_UNEXPECTED_END: return  "unexpected end of input";
            case ERROR_MISSING_ROOT_ELEMENT: return  "missing root element";
            case ERROR_BAD_ROOT: return  "document root must be object or array";
            case ERROR_EXPECTED_COMMA: return  "expected ,";
            case ERROR_MISSING_OBJECT_KEY: return  "missing object key";
            case ERROR_EXPECTED_COLON: return  "expected :";
            case ERROR_EXPECTED_END_OF_INPUT: return  "expected end of input";
            case ERROR_UNEXPECTED_COMMA: return  "unexpected comma";
            case ERROR_EXPECTED_VALUE: return  "expected value";
            case ERROR_EXPECTED_NULL: return  "expected 'null'";
            case ERROR_EXPECTED_FALSE: return  "expected 'false'";
            case ERROR_EXPECTED_TRUE: return  "expected 'true'";
            case ERROR_MSSING_EXPONENT: return  "missing exponent";
            case ERROR_ILLEGAL_CODEPOINT: return  "illegal unprintable codepoint in string";
            case ERROR_INVALID_UNICODE_ESCAPE: return  "invalid character in unicode escape";
            case ERROR_UNEXPECTED_END_OF_UTF16: return  "unexpected end of input during UTF-16 surrogate pair";
            case ERROR_EXPECTED_U: return  "expected \\u";
            case ERROR_INVALID_UTF16_TRAIL_SURROGATE: return  "invalid UTF-16 trail surrogate";
            case ERROR_UNKNOWN_ESCAPE: return  "unknown escape";
            case ERROR_INVALID_UTF8: return  "invalid UTF-8";
        }

        return "unknown error";
    }

    bool has_significant_error_arg(error code) {
        return code == ERROR_ILLEGAL_CODEPOINT;
    }

    std::string get_error_message(int line, int column, error code, int arg) {
        std::ostringstream sout;
        sout << get_error_text(code);
        if(has_significant_error_arg(code)) {
            sout << ": " << arg;
        }
        return sout.str();
    }

    std::string get_error_message(const document& doc) {
        return get_error_message(doc.get_error_line(), doc.get_error_column(), doc.get_error_code(), doc.get_error_arg());
    }
}