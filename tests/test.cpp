#include <UnitTest++.h>
#include "sajson.h"
using namespace sajson;

inline bool success(const document& doc) {
    if (!doc.is_valid()) {
        fprintf(stderr, "%s\n", doc.get_error_message().c_str());
        return false;
    }
    return true;
}

TEST(empty_array) {
    const sajson::document& document = parse(literal("[]"));
    assert(success(document));
    const value& root = document.get_root();
    CHECK_EQUAL(true, document.is_valid());
    CHECK_EQUAL(TYPE_ARRAY, root.get_type());
    CHECK_EQUAL(0u, root.get_length());
}

TEST(array_whitespace) {
    const sajson::document& document = parse(literal(" [ ] "));
    assert(success(document));
    const value& root = document.get_root();
    CHECK_EQUAL(TYPE_ARRAY, root.get_type());
    CHECK_EQUAL(0u, root.get_length());
}

TEST(array_zero) {
    const sajson::document& document = parse(literal("[0]"));
    assert(success(document));
    const value& root = document.get_root();
    CHECK_EQUAL(TYPE_ARRAY, root.get_type());
    CHECK_EQUAL(1u, root.get_length());

    const value& e0 = root.get_array_element(0);
    CHECK_EQUAL(TYPE_INTEGER, e0.get_type());
    CHECK_EQUAL(0, e0.get_number_value());
}

TEST(nested_array) {
    const sajson::document& document = parse(literal("[[]]"));
    assert(success(document));
    const value& root = document.get_root();
    CHECK_EQUAL(TYPE_ARRAY, root.get_type());
    CHECK_EQUAL(1u, root.get_length());
        
    const value& e1 = root.get_array_element(0);
    CHECK_EQUAL(TYPE_ARRAY, e1.get_type());
    CHECK_EQUAL(0u, e1.get_length());
}

TEST(packed_arrays) {
    const sajson::document& document = parse(literal("[0,[0,[0],0],0]"));
    assert(success(document));
    const value& root = document.get_root();
    CHECK_EQUAL(TYPE_ARRAY, root.get_type());
    CHECK_EQUAL(3u, root.get_length());

    const value& root0 = root.get_array_element(0);
    CHECK_EQUAL(TYPE_INTEGER, root0.get_type());
    CHECK_EQUAL(0, root0.get_number_value());

    const value& root2 = root.get_array_element(2);
    CHECK_EQUAL(TYPE_INTEGER, root2.get_type());
    CHECK_EQUAL(0, root2.get_number_value());

    const value& root1 = root.get_array_element(1);
    CHECK_EQUAL(TYPE_ARRAY, root1.get_type());
    CHECK_EQUAL(3u, root1.get_length());

    const value& sub0 = root1.get_array_element(0);
    CHECK_EQUAL(TYPE_INTEGER, sub0.get_type());
    CHECK_EQUAL(0, sub0.get_number_value());

    const value& sub2 = root1.get_array_element(2);
    CHECK_EQUAL(TYPE_INTEGER, sub2.get_type());
    CHECK_EQUAL(0, sub2.get_number_value());

    const value& sub1 = root1.get_array_element(1);
    CHECK_EQUAL(TYPE_ARRAY, sub1.get_type());
    CHECK_EQUAL(1u, sub1.get_length());

    const value& inner = sub1.get_array_element(0);
    CHECK_EQUAL(TYPE_INTEGER, inner.get_type());
    CHECK_EQUAL(0, inner.get_number_value());
}

TEST(deep_nesting) {
    const sajson::document& document = parse(literal("[[[[]]]]"));
    assert(success(document));
    const value& root = document.get_root();
    CHECK_EQUAL(TYPE_ARRAY, root.get_type());
    CHECK_EQUAL(1u, root.get_length());
        
    const value& e1 = root.get_array_element(0);
    CHECK_EQUAL(TYPE_ARRAY, e1.get_type());
    CHECK_EQUAL(1u, e1.get_length());

    const value& e2 = e1.get_array_element(0);
    CHECK_EQUAL(TYPE_ARRAY, e2.get_type());
    CHECK_EQUAL(1u, e2.get_length());

    const value& e3 = e2.get_array_element(0);
    CHECK_EQUAL(TYPE_ARRAY, e3.get_type());
    CHECK_EQUAL(0u, e3.get_length());
}

TEST(more_array_integer_packing) {
    const sajson::document& document = parse(literal("[[[[0]]]]"));
    assert(success(document));
    const value& root = document.get_root();
    CHECK_EQUAL(TYPE_ARRAY, root.get_type());
    CHECK_EQUAL(1u, root.get_length());
        
    const value& e1 = root.get_array_element(0);
    CHECK_EQUAL(TYPE_ARRAY, e1.get_type());
    CHECK_EQUAL(1u, e1.get_length());

    const value& e2 = e1.get_array_element(0);
    CHECK_EQUAL(TYPE_ARRAY, e2.get_type());
    CHECK_EQUAL(1u, e2.get_length());

    const value& e3 = e2.get_array_element(0);
    CHECK_EQUAL(TYPE_ARRAY, e3.get_type());
    CHECK_EQUAL(1u, e3.get_length());

    const value& e4 = e3.get_array_element(0);
    CHECK_EQUAL(TYPE_INTEGER, e4.get_type());
    CHECK_EQUAL(0, e4.get_integer_value());
}

SUITE(integers) {
    TEST(negative_and_positive_integers) {
        const sajson::document& document = parse(literal(" [ 0, -1, 22] "));
        assert(success(document));
        const value& root = document.get_root();
        CHECK_EQUAL(TYPE_ARRAY, root.get_type());
        CHECK_EQUAL(3u, root.get_length());

        const value& e0 = root.get_array_element(0);
        CHECK_EQUAL(TYPE_INTEGER, e0.get_type());
        CHECK_EQUAL(0, e0.get_integer_value());
        CHECK_EQUAL(0, e0.get_number_value());

        const value& e1 = root.get_array_element(1);
        CHECK_EQUAL(TYPE_INTEGER, e1.get_type());
        CHECK_EQUAL(-1, e1.get_integer_value());
        CHECK_EQUAL(-1, e1.get_number_value());

        const value& e2 = root.get_array_element(2);
        CHECK_EQUAL(TYPE_INTEGER, e2.get_type());
        CHECK_EQUAL(22, e2.get_integer_value());
        CHECK_EQUAL(22, e2.get_number_value());
    }

    TEST(integers) {
        const sajson::document& document = parse(literal("[0,1,2,3,4,5,6,7,8,9,10]"));
        assert(success(document));
        const value& root = document.get_root();
        CHECK_EQUAL(TYPE_ARRAY, root.get_type());
        CHECK_EQUAL(11u, root.get_length());

        for (int i = 0; i < 11; ++i) {
            const value& e = root.get_array_element(i);
            CHECK_EQUAL(TYPE_INTEGER, e.get_type());
            CHECK_EQUAL(i, e.get_integer_value());
        }
    }

    TEST(integer_whitespace) {
        const sajson::document& document = parse(literal(" [ 0 , 0 ] "));
        assert(success(document));
        const value& root = document.get_root();
        CHECK_EQUAL(TYPE_ARRAY, root.get_type());
        CHECK_EQUAL(2u, root.get_length());
        const value& element = root.get_array_element(1);
        CHECK_EQUAL(TYPE_INTEGER, element.get_type());
        CHECK_EQUAL(0, element.get_integer_value());
    }
}

TEST(unit_types) {
    const sajson::document& document = parse(literal("[ true , false , null ]"));
    assert(success(document));
    const value& root = document.get_root();
    CHECK_EQUAL(TYPE_ARRAY, root.get_type());
    CHECK_EQUAL(3u, root.get_length());

    const value& e0 = root.get_array_element(0);
    CHECK_EQUAL(TYPE_TRUE, e0.get_type());

    const value& e1 = root.get_array_element(1);
    CHECK_EQUAL(TYPE_FALSE, e1.get_type());
        
    const value& e2 = root.get_array_element(2);
    CHECK_EQUAL(TYPE_NULL, e2.get_type());
}

SUITE(doubles) {
    TEST(doubles) {
        const sajson::document& document = parse(literal("[-0,-1,-34.25]"));
        assert(success(document));
        const value& root = document.get_root();
        CHECK_EQUAL(TYPE_ARRAY, root.get_type());
        CHECK_EQUAL(3u, root.get_length());

        const value& e0 = root.get_array_element(0);
        CHECK_EQUAL(TYPE_INTEGER, e0.get_type());
        CHECK_EQUAL(-0, e0.get_integer_value());

        const value& e1 = root.get_array_element(1);
        CHECK_EQUAL(TYPE_INTEGER, e1.get_type());
        CHECK_EQUAL(-1, e1.get_integer_value());

        const value& e2 = root.get_array_element(2);
        CHECK_EQUAL(TYPE_DOUBLE, e2.get_type());
        CHECK_EQUAL(-34.25, e2.get_double_value());
    }

    TEST(exponents) {
        const sajson::document& document = parse(literal("[2e+3,0.5E-5,10E+22]"));
        assert(success(document));
        const value& root = document.get_root();
        CHECK_EQUAL(TYPE_ARRAY, root.get_type());
        CHECK_EQUAL(3u, root.get_length());

        const value& e0 = root.get_array_element(0);
        CHECK_EQUAL(TYPE_DOUBLE, e0.get_type());
        CHECK_EQUAL(2000, e0.get_double_value());

        const value& e1 = root.get_array_element(1);
        CHECK_EQUAL(TYPE_DOUBLE, e1.get_type());
        CHECK_CLOSE(0.000005, e1.get_double_value(), 1e-20);

        const value& e2 = root.get_array_element(2);
        CHECK_EQUAL(TYPE_DOUBLE, e2.get_type());
        CHECK_EQUAL(10e22, e2.get_double_value());
    }

    TEST(long_no_exponent) {
        const sajson::document& document = parse(literal("[9999999999,99999999999]"));
        assert(success(document));
        const value& root = document.get_root();
        CHECK_EQUAL(TYPE_ARRAY, root.get_type());
        CHECK_EQUAL(2u, root.get_length());
        
        const value& e0 = root.get_array_element(0);
        CHECK_EQUAL(TYPE_DOUBLE, e0.get_type());
        CHECK_EQUAL(9999999999.0, e0.get_double_value());

        const value& e1 = root.get_array_element(1);
        CHECK_EQUAL(TYPE_DOUBLE, e1.get_type());
        CHECK_EQUAL(99999999999.0, e1.get_double_value());
    }

    TEST(exponent_offset) {
        const sajson::document& document = parse(literal("[0.005e3]"));
        assert(success(document));
        const value& root = document.get_root();
        CHECK_EQUAL(TYPE_ARRAY, root.get_type());
        CHECK_EQUAL(1u, root.get_length());
        
        const value& e0 = root.get_array_element(0);
        CHECK_EQUAL(TYPE_DOUBLE, e0.get_type());
        CHECK_EQUAL(5.0, e0.get_double_value());
    }
}

SUITE(strings) {
    TEST(strings) {
        const sajson::document& document = parse(literal("[\"\", \"foobar\"]"));
        assert(success(document));
        const value& root = document.get_root();
        CHECK_EQUAL(TYPE_ARRAY, root.get_type());
        CHECK_EQUAL(2u, root.get_length());
        
        const value& e0 = root.get_array_element(0);
        CHECK_EQUAL(TYPE_STRING, e0.get_type());
        CHECK_EQUAL(0u, e0.get_string_length());
        CHECK_EQUAL("", e0.as_string());
        
        const value& e1 = root.get_array_element(1);
        CHECK_EQUAL(TYPE_STRING, e1.get_type());
        CHECK_EQUAL(6u, e1.get_string_length());
        CHECK_EQUAL("foobar", e1.as_string());
    }

    TEST(common_escapes) {
        // \"\\\/\b\f\n\r\t
        const sajson::document& document = parse(literal("[\"\\\"\\\\\\/\\b\\f\\n\\r\\t\"]"));
        assert(success(document));
        const value& root = document.get_root();
        CHECK_EQUAL(TYPE_ARRAY, root.get_type());
        CHECK_EQUAL(1u, root.get_length());
        
        const value& e0 = root.get_array_element(0);
        CHECK_EQUAL(TYPE_STRING, e0.get_type());
        CHECK_EQUAL(8u, e0.get_string_length());
        CHECK_EQUAL("\"\\/\b\f\n\r\t", e0.as_string());
    }

    TEST(escape_midstring) {
        const sajson::document& document = parse(literal("[\"foo\\tbar\"]"));
        assert(success(document));
        const value& root = document.get_root();
        CHECK_EQUAL(TYPE_ARRAY, root.get_type());
        CHECK_EQUAL(1u, root.get_length());
        
        const value& e0 = root.get_array_element(0);
        CHECK_EQUAL(TYPE_STRING, e0.get_type());
        CHECK_EQUAL(7u, e0.get_string_length());
        CHECK_EQUAL("foo\tbar", e0.as_string());
    }

    TEST(unfinished_string) {
        const sajson::document& document = parse(literal("[\""));
        CHECK_EQUAL(false, document.is_valid());
        CHECK_EQUAL(1u, document.get_error_line());
        //CHECK_EQUAL(3, document.get_error_column());
        CHECK_EQUAL("unexpected end of input", document.get_error_message());
    }

    TEST(unfinished_escape) {
        const sajson::document& document = parse(literal("[\"\\"));
        CHECK_EQUAL(false, document.is_valid());
        CHECK_EQUAL(1u, document.get_error_line());
        //CHECK_EQUAL(3, document.get_error_column());
        CHECK_EQUAL("unexpected end of input", document.get_error_message());
    }

    TEST(unprintables_are_not_valid_in_strings) {
        const sajson::document& document = parse(literal("[\"\x19\"]"));
        CHECK_EQUAL(false, document.is_valid());
        CHECK_EQUAL(1u, document.get_error_line());
        //CHECK_EQUAL(3, document.get_error_column());
        CHECK_EQUAL("illegal unprintable codepoint in string: 25", document.get_error_message());
    }

    TEST(unprintables_are_not_valid_in_strings_after_escapes) {
        const sajson::document& document = parse(literal("[\"\\n\x01\"]"));
        CHECK_EQUAL(false, document.is_valid());
        CHECK_EQUAL(2, document.get_error_column());
        CHECK_EQUAL("illegal unprintable codepoint in string: 1", document.get_error_message());
    }

    TEST(utf16_surrogate_pair) {
        const sajson::document& document = parse(literal("[\"\\ud950\\uDf21\"]"));
        assert(success(document));
        const value& root = document.get_root();
        CHECK_EQUAL(TYPE_ARRAY, root.get_type());
        CHECK_EQUAL(1u, root.get_length());
        
        const value& e0 = root.get_array_element(0);
        CHECK_EQUAL(TYPE_STRING, e0.get_type());
        CHECK_EQUAL(4u, e0.get_string_length());
        CHECK_EQUAL("\xf1\xa4\x8c\xa1", e0.as_string());
    }
}

SUITE(objects) {
    TEST(empty_object) {
        const sajson::document& document = parse(literal("{}"));
        assert(success(document));
        const value& root = document.get_root();
        CHECK_EQUAL(TYPE_OBJECT, root.get_type());
        CHECK_EQUAL(0u, root.get_length());
    }

    TEST(nested_object) {
        const sajson::document& document = parse(literal("{\"a\":{\"b\":{}}} "));
        assert(success(document));
        const value& root = document.get_root();
        CHECK_EQUAL(TYPE_OBJECT, root.get_type());
        CHECK_EQUAL(1u, root.get_length());

        const string& key = root.get_object_key(0);
        CHECK_EQUAL("a", key.as_string());

        const value& element = root.get_object_value(0);
        CHECK_EQUAL(TYPE_OBJECT, element.get_type());
        CHECK_EQUAL("b", element.get_object_key(0).as_string());

        const value& inner = element.get_object_value(0);
        CHECK_EQUAL(TYPE_OBJECT, inner.get_type());
        CHECK_EQUAL(0u, inner.get_length());
    }

    TEST(object_whitespace) {
        const sajson::document& document = parse(literal(" { \"a\" : 0 } "));
        assert(success(document));
        const value& root = document.get_root();
        CHECK_EQUAL(TYPE_OBJECT, root.get_type());
        CHECK_EQUAL(1u, root.get_length());

        const string& key = root.get_object_key(0);
        CHECK_EQUAL("a", key.as_string());

        const value& element = root.get_object_value(0);
        CHECK_EQUAL(TYPE_INTEGER, element.get_type());
        CHECK_EQUAL(0, element.get_integer_value());
    }

    TEST(object_keys_are_sorted) {
        const sajson::document& document = parse(literal(" { \"b\" : 1 , \"a\" : 0 } "));
        assert(success(document));
        const value& root = document.get_root();
        CHECK_EQUAL(TYPE_OBJECT, root.get_type());
        CHECK_EQUAL(2u, root.get_length());
        
        const string& k0 = root.get_object_key(0);
        const value& e0 = root.get_object_value(0);
        CHECK_EQUAL("a", k0.as_string());
        CHECK_EQUAL(TYPE_INTEGER, e0.get_type());
        CHECK_EQUAL(0, e0.get_integer_value());

        const string& k1 = root.get_object_key(1);
        const value& e1 = root.get_object_value(1);
        CHECK_EQUAL("b", k1.as_string());
        CHECK_EQUAL(TYPE_INTEGER, e1.get_type());
        CHECK_EQUAL(1, e1.get_integer_value());
    }

    TEST(object_keys_are_sorted_length_first) {
        const sajson::document& document = parse(literal(" { \"b\" : 1 , \"aa\" : 0 } "));
        assert(success(document));
        const value& root = document.get_root();
        CHECK_EQUAL(TYPE_OBJECT, root.get_type());
        CHECK_EQUAL(2u, root.get_length());
        
        const string& k0 = root.get_object_key(0);
        const value& e0 = root.get_object_value(0);
        CHECK_EQUAL("b", k0.as_string());
        CHECK_EQUAL(TYPE_INTEGER, e0.get_type());
        CHECK_EQUAL(1, e0.get_integer_value());

        const string& k1 = root.get_object_key(1);
        const value& e1 = root.get_object_value(1);
        CHECK_EQUAL("aa", k1.as_string());
        CHECK_EQUAL(TYPE_INTEGER, e1.get_type());
        CHECK_EQUAL(0, e1.get_integer_value());
    }

    TEST(binary_search_for_keys) {
        const sajson::document& document = parse(literal(" { \"b\" : 1 , \"aa\" : 0 } "));
        assert(success(document));
        const value& root = document.get_root();
        CHECK_EQUAL(TYPE_OBJECT, root.get_type());
        CHECK_EQUAL(2u, root.get_length());

        const size_t index_b = root.find_object_key(literal("b"));
        CHECK_EQUAL(0U, index_b);

        const size_t index_aa = root.find_object_key(literal("aa"));
        CHECK_EQUAL(1U, index_aa);

        const size_t index_c = root.find_object_key(literal("c"));
        CHECK_EQUAL(2U, index_c);

        const size_t index_ccc = root.find_object_key(literal("ccc"));
        CHECK_EQUAL(2U, index_ccc);
    }

    TEST(binary_search_handles_prefix_keys) {
        const sajson::document& document = parse(literal(" { \"prefix_key\" : 0 } "));
        assert(success(document));
        const value& root = document.get_root();
        CHECK_EQUAL(TYPE_OBJECT, root.get_type());
        CHECK_EQUAL(1u, root.get_length());

        const size_t index_prefix = root.find_object_key(literal("prefix"));
        CHECK_EQUAL(1U, index_prefix);
    }
}

SUITE(errors) {
    TEST(empty_file_is_invalid) {
        const sajson::document& document = parse(literal(""));
        CHECK_EQUAL(false, document.is_valid());
        CHECK_EQUAL(1u, document.get_error_line());
        CHECK_EQUAL(1u, document.get_error_column());
        CHECK_EQUAL("no root element", document.get_error_message());
    }

    TEST(two_roots_are_invalid) {
        const sajson::document& document = parse(literal("[][]"));
        CHECK_EQUAL(false, document.is_valid());
        CHECK_EQUAL(1u, document.get_error_line());
        //CHECK_EQUAL(3, document.get_error_column());
        CHECK_EQUAL("expected end of input", document.get_error_message());
    }

    TEST(root_must_be_object_or_array) {
        const sajson::document& document = parse(literal("0"));
        CHECK_EQUAL(false, document.is_valid());
        CHECK_EQUAL(1u, document.get_error_line());
        CHECK_EQUAL(1u, document.get_error_column());
        CHECK_EQUAL("document root must be object or array", document.get_error_message());
    }

    TEST(incomplete_object_key) {
        const sajson::document& document = parse(literal("{\"\\:0}"));
        CHECK_EQUAL(false, document.is_valid());
        CHECK_EQUAL(1u, document.get_error_line());
        CHECK_EQUAL(4u, document.get_error_column());
        CHECK_EQUAL("invalid object key", document.get_error_message());
    }

    TEST(commas_are_necessary_between_elements) {
        const sajson::document& document = parse(literal("[0 0]"));
        CHECK_EQUAL(false, document.is_valid());
        CHECK_EQUAL(1u, document.get_error_line());
        //CHECK_EQUAL(3, document.get_error_column());
        CHECK_EQUAL("expected ,", document.get_error_message());
    }

    TEST(keys_must_be_strings) {
        const sajson::document& document = parse(literal("{0:0}"));
        CHECK_EQUAL(false, document.is_valid());
        CHECK_EQUAL(1u, document.get_error_line());
        //CHECK_EQUAL(3, document.get_error_column());
        CHECK_EQUAL("object key must be quoted", document.get_error_message());
    }

    TEST(objects_must_have_keys) {
        const sajson::document& document = parse(literal("{\"0\"}"));
        CHECK_EQUAL(false, document.is_valid());
        CHECK_EQUAL(1u, document.get_error_line());
        //CHECK_EQUAL(3, document.get_error_column());
        CHECK_EQUAL("expected :", document.get_error_message());
    }

    TEST(invalid_true_literal) {
        const sajson::document& document = parse(literal("[truf"));
        CHECK_EQUAL(false, document.is_valid());
        CHECK_EQUAL(1u, document.get_error_line());
        //CHECK_EQUAL(3, document.get_error_column());
        CHECK_EQUAL("expected 'true'", document.get_error_message());
    }

    TEST(incomplete_true_literal) {
        const sajson::document& document = parse(literal("[tru"));
        CHECK_EQUAL(false, document.is_valid());
        CHECK_EQUAL(1u, document.get_error_line());
        //CHECK_EQUAL(3, document.get_error_column());
        CHECK_EQUAL("unexpected end of input", document.get_error_message());
    }

    TEST(must_close_array_with_square_bracket) {
        const sajson::document& document = parse(literal("[}"));
        CHECK_EQUAL(false, document.is_valid());
        CHECK_EQUAL(1u, document.get_error_line());
        //CHECK_EQUAL(3, document.get_error_column());
        CHECK_EQUAL("expected ]", document.get_error_message());
    }

    TEST(must_close_object_with_curly_brace) {
        const sajson::document& document = parse(literal("{]"));
        CHECK_EQUAL(false, document.is_valid());
        CHECK_EQUAL(1u, document.get_error_line());
        //CHECK_EQUAL(3, document.get_error_column());
        CHECK_EQUAL("object key must be quoted", document.get_error_message());
    }

#define CHECK_PARSE_ERROR(text, error_message)                          \
    do {                                                                \
        const sajson::document& document = parse(literal(text));        \
        CHECK_EQUAL(false, document.is_valid());                        \
        CHECK_EQUAL(error_message, document.get_error_message());       \
    } while (0)

    TEST(invalid_number) {
        CHECK_PARSE_ERROR("[-", "unexpected end of input");
        CHECK_PARSE_ERROR("[-12", "unexpected end of input");
        CHECK_PARSE_ERROR("[-12.", "unexpected end of input");
        CHECK_PARSE_ERROR("[-12.3", "unexpected end of input");
        CHECK_PARSE_ERROR("[-12e", "unexpected end of input");
        CHECK_PARSE_ERROR("[-12e-", "unexpected end of input");
        CHECK_PARSE_ERROR("[-12e+", "unexpected end of input");
        CHECK_PARSE_ERROR("[-12e3", "unexpected end of input");
    }
}

TEST(object_array_with_integers) {
    const sajson::document& document = parse(literal("[{ \"a\": 123456 }, { \"a\": 7890 }]"));
    assert(success(document));
    const value& root = document.get_root();
    CHECK_EQUAL(TYPE_ARRAY, root.get_type());
    CHECK_EQUAL(2u, root.get_length());

    const value& e1 = root.get_array_element(0);
    CHECK_EQUAL(TYPE_OBJECT, e1.get_type());
    size_t index_a = e1.find_object_key(literal("a"));
    const value& node = e1.get_object_value(index_a);
    CHECK_EQUAL(TYPE_INTEGER, node.get_type());
    CHECK_EQUAL(123456U, node.get_number_value());
    const value& e2 = root.get_array_element(1);
    CHECK_EQUAL(TYPE_OBJECT, e2.get_type());
    index_a = e2.find_object_key(literal("a"));
    const value& node2 = e2.get_object_value(index_a);
    CHECK_EQUAL(7890U, node2.get_number_value());
}

int main() {
    return UnitTest::RunAllTests();
}
