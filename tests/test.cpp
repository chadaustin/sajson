#undef NDEBUG

#include <assert.h>
#include "sajson.h"
using namespace sajson;

#define CHECK_EQUAL(x, y) assert((x) == (y));

inline bool success(const document& doc) {
    if (!doc.is_valid()) {
        fprintf(stderr, "%s\n", doc.get_error_message().c_str());
        return false;
    }
    return true;
}

int main() {
    {
        const auto& document = parse(literal("[]"));
        assert(success(document));
        const auto& root = document.get_root();
        CHECK_EQUAL(true, document.is_valid());
        CHECK_EQUAL(type::array, root.get_type());
        CHECK_EQUAL(0, root.get_length());
    }

    {
        const auto& document = parse(literal(" [ ] "));
        assert(success(document));
        const auto& root = document.get_root();
        CHECK_EQUAL(type::array, root.get_type());
        CHECK_EQUAL(0, root.get_length());
    }

    {
        const auto& document = parse(literal("[0]"));
        assert(success(document));
        const auto& root = document.get_root();
        CHECK_EQUAL(type::array, root.get_type());
        CHECK_EQUAL(1, root.get_length());

        const auto& e0 = root.get_array_element(0);
        CHECK_EQUAL(type::integer, e0.get_type());
        CHECK_EQUAL(0, e0.get_number_value());
    }

    {
        const auto& document = parse(literal("[[]]"));
        assert(success(document));
        const auto& root = document.get_root();
        CHECK_EQUAL(type::array, root.get_type());
        CHECK_EQUAL(1, root.get_length());
        
        const auto& e1 = root.get_array_element(0);
        CHECK_EQUAL(type::array, e1.get_type());
        CHECK_EQUAL(0, e1.get_length());
    }

    {
        const auto& document = parse(literal("[0,[0,[0],0],0]"));
        assert(success(document));
        const auto& root = document.get_root();
        CHECK_EQUAL(type::array, root.get_type());
        CHECK_EQUAL(3, root.get_length());

        const auto& root0 = root.get_array_element(0);
        CHECK_EQUAL(type::integer, root0.get_type());
        CHECK_EQUAL(0, root0.get_number_value());

        const auto& root2 = root.get_array_element(2);
        CHECK_EQUAL(type::integer, root2.get_type());
        CHECK_EQUAL(0, root2.get_number_value());

        const auto& root1 = root.get_array_element(1);
        CHECK_EQUAL(type::array, root1.get_type());
        CHECK_EQUAL(3, root1.get_length());

        const auto& sub0 = root1.get_array_element(0);
        CHECK_EQUAL(type::integer, sub0.get_type());
        CHECK_EQUAL(0, sub0.get_number_value());

        const auto& sub2 = root1.get_array_element(2);
        CHECK_EQUAL(type::integer, sub2.get_type());
        CHECK_EQUAL(0, sub2.get_number_value());

        const auto& sub1 = root1.get_array_element(1);
        CHECK_EQUAL(type::array, sub1.get_type());
        CHECK_EQUAL(1, sub1.get_length());

        const auto& inner = sub1.get_array_element(0);
        CHECK_EQUAL(type::integer, inner.get_type());
        CHECK_EQUAL(0, inner.get_number_value());
    }

    {
        const auto& document = parse(literal("[[[[]]]]"));
        assert(success(document));
        const auto& root = document.get_root();
        CHECK_EQUAL(type::array, root.get_type());
        CHECK_EQUAL(1, root.get_length());
        
        const auto& e1 = root.get_array_element(0);
        CHECK_EQUAL(type::array, e1.get_type());
        CHECK_EQUAL(1, e1.get_length());

        const auto& e2 = e1.get_array_element(0);
        CHECK_EQUAL(type::array, e2.get_type());
        CHECK_EQUAL(1, e2.get_length());

        const auto& e3 = e2.get_array_element(0);
        CHECK_EQUAL(type::array, e3.get_type());
        CHECK_EQUAL(0, e3.get_length());
    }

    {
        const auto& document = parse(literal(" [ 0, -1, 2] "));
        assert(success(document));
        const auto& root = document.get_root();
        CHECK_EQUAL(type::array, root.get_type());
        CHECK_EQUAL(3, root.get_length());

        const auto& e0 = root.get_array_element(0);
        CHECK_EQUAL(type::integer, e0.get_type());
        CHECK_EQUAL(0, e0.get_integer_value());
        CHECK_EQUAL(0, e0.get_number_value());

        const auto& e1 = root.get_array_element(1);
        CHECK_EQUAL(type::integer, e1.get_type());
        CHECK_EQUAL(-1, e1.get_integer_value());
        CHECK_EQUAL(-1, e1.get_number_value());

        const auto& e2 = root.get_array_element(2);
        CHECK_EQUAL(type::integer, e2.get_type());
        CHECK_EQUAL(2, e2.get_integer_value());
        CHECK_EQUAL(2, e2.get_number_value());
    }

    {
        const auto& document = parse(literal(" [ 0 , 0 ] "));
        assert(success(document));
        const auto& root = document.get_root();
        CHECK_EQUAL(type::array, root.get_type());
        CHECK_EQUAL(2, root.get_length());
        const auto& element = root.get_array_element(1);
        CHECK_EQUAL(type::integer, element.get_type());
        CHECK_EQUAL(0, element.get_integer_value());
    }

    {
        const auto& document = parse(literal("[[[[0]]]]"));
        assert(success(document));
        const auto& root = document.get_root();
        CHECK_EQUAL(type::array, root.get_type());
        CHECK_EQUAL(1, root.get_length());
        
        const auto& e1 = root.get_array_element(0);
        CHECK_EQUAL(type::array, e1.get_type());
        CHECK_EQUAL(1, e1.get_length());

        const auto& e2 = e1.get_array_element(0);
        CHECK_EQUAL(type::array, e2.get_type());
        CHECK_EQUAL(1, e2.get_length());

        const auto& e3 = e2.get_array_element(0);
        CHECK_EQUAL(type::array, e3.get_type());
        CHECK_EQUAL(1, e3.get_length());

        const auto& e4 = e3.get_array_element(0);
        CHECK_EQUAL(type::integer, e4.get_type());
        CHECK_EQUAL(0, e4.get_integer_value());
    }

    {
        const auto& document = parse(literal("[ true , false , null ]"));
        assert(success(document));
        const auto& root = document.get_root();
        CHECK_EQUAL(type::array, root.get_type());
        CHECK_EQUAL(3, root.get_length());

        const auto& e0 = root.get_array_element(0);
        CHECK_EQUAL(type::true_, e0.get_type());

        const auto& e1 = root.get_array_element(1);
        CHECK_EQUAL(type::false_, e1.get_type());
        
        const auto& e2 = root.get_array_element(2);
        CHECK_EQUAL(type::null, e2.get_type());
    }

    {
        const auto& document = parse(literal("[0,1,2,3,4,5,6,7,8,9,10]"));
        assert(success(document));
        const auto& root = document.get_root();
        CHECK_EQUAL(type::array, root.get_type());
        CHECK_EQUAL(11, root.get_length());

        for (int i = 0; i < 11; ++i) {
            const auto& e = root.get_array_element(i);
            CHECK_EQUAL(type::integer, e.get_type());
            CHECK_EQUAL(i, e.get_integer_value());
        }
    }

    {
        const auto& document = parse(literal("[-0,-1,-34.25]"));
        assert(success(document));
        const auto& root = document.get_root();
        CHECK_EQUAL(type::array, root.get_type());
        CHECK_EQUAL(3, root.get_length());

        const auto& e0 = root.get_array_element(0);
        CHECK_EQUAL(type::integer, e0.get_type());
        CHECK_EQUAL(-0, e0.get_integer_value());

        const auto& e1 = root.get_array_element(1);
        CHECK_EQUAL(type::integer, e1.get_type());
        CHECK_EQUAL(-1, e1.get_integer_value());

        const auto& e2 = root.get_array_element(2);
        CHECK_EQUAL(type::double_, e2.get_type());
        CHECK_EQUAL(-34.25, e2.get_double_value());
    }

    {
        const auto& document = parse(literal("[2e+3,0.5E-5,10E+22]"));
        assert(success(document));
        const auto& root = document.get_root();
        CHECK_EQUAL(type::array, root.get_type());
        CHECK_EQUAL(3, root.get_length());

        const auto& e0 = root.get_array_element(0);
        CHECK_EQUAL(type::double_, e0.get_type());
        CHECK_EQUAL(2000, e0.get_double_value());

        const auto& e1 = root.get_array_element(1);
        CHECK_EQUAL(type::double_, e1.get_type());
        CHECK_EQUAL(0.000005, e1.get_double_value());

        const auto& e2 = root.get_array_element(2);
        CHECK_EQUAL(type::double_, e2.get_type());
        CHECK_EQUAL(10e22, e2.get_double_value());
    }

    {
        const auto& document = parse(literal("[\"\", \"foobar\"]"));
        assert(success(document));
        const auto& root = document.get_root();
        CHECK_EQUAL(type::array, root.get_type());
        CHECK_EQUAL(2, root.get_length());
        
        const auto& e0 = root.get_array_element(0);
        CHECK_EQUAL(type::string, e0.get_type());
        CHECK_EQUAL(0, e0.get_string_length());
        CHECK_EQUAL("", e0.as_string());
        
        const auto& e1 = root.get_array_element(1);
        CHECK_EQUAL(type::string, e1.get_type());
        CHECK_EQUAL(6, e1.get_string_length());
        CHECK_EQUAL("foobar", e1.as_string());
    }

    {
        const auto& document = parse(literal("{}"));
        assert(success(document));
        const auto& root = document.get_root();
        CHECK_EQUAL(type::object, root.get_type());
        CHECK_EQUAL(0, root.get_length());
    }

    {
        const auto& document = parse(literal("{\"a\":{\"b\":{}}} "));
        assert(success(document));
        const auto& root = document.get_root();
        CHECK_EQUAL(type::object, root.get_type());
        CHECK_EQUAL(1, root.get_length());

        const auto& key = root.get_object_key(0);
        CHECK_EQUAL("a", key.as_string());

        const auto& element = root.get_object_value(0);
        CHECK_EQUAL(type::object, element.get_type());
        CHECK_EQUAL("b", element.get_object_key(0).as_string());

        const auto& inner = element.get_object_value(0);
        CHECK_EQUAL(type::object, inner.get_type());
        CHECK_EQUAL(0, inner.get_length());
    }

    {
        const auto& document = parse(literal(" { \"a\" : 0 } "));
        assert(success(document));
        const auto& root = document.get_root();
        CHECK_EQUAL(type::object, root.get_type());
        CHECK_EQUAL(1, root.get_length());

        const auto& key = root.get_object_key(0);
        CHECK_EQUAL("a", key.as_string());

        const auto& element = root.get_object_value(0);
        CHECK_EQUAL(type::integer, element.get_type());
        CHECK_EQUAL(0, element.get_integer_value());
    }

    {
        const auto& document = parse(literal(" { \"b\" : 1 , \"a\" : 0 } "));
        assert(success(document));
        const auto& root = document.get_root();
        CHECK_EQUAL(type::object, root.get_type());
        CHECK_EQUAL(2, root.get_length());
        
        const auto& k0 = root.get_object_key(0);
        const auto& e0 = root.get_object_value(0);
        CHECK_EQUAL("a", k0.as_string());
        CHECK_EQUAL(type::integer, e0.get_type());
        CHECK_EQUAL(0, e0.get_integer_value());

        const auto& k1 = root.get_object_key(1);
        const auto& e1 = root.get_object_value(1);
        CHECK_EQUAL("b", k1.as_string());
        CHECK_EQUAL(type::integer, e1.get_type());
        CHECK_EQUAL(1, e1.get_integer_value());
    }

    // error tests
    {
        const auto& document = parse(literal(""));
        CHECK_EQUAL(false, document.is_valid());
        CHECK_EQUAL(1, document.get_error_line());
        CHECK_EQUAL(1, document.get_error_column());
        CHECK_EQUAL("no root element", document.get_error_message());
    }

    {
        const auto& document = parse(literal("[][]"));
        CHECK_EQUAL(false, document.is_valid());
        CHECK_EQUAL(1, document.get_error_line());
        //CHECK_EQUAL(3, document.get_error_column());
        CHECK_EQUAL("expected end of input", document.get_error_message());
    }

    {
        const auto& document = parse(literal("0"));
        CHECK_EQUAL(false, document.is_valid());
        CHECK_EQUAL(1, document.get_error_line());
        CHECK_EQUAL(1, document.get_error_column());
        CHECK_EQUAL("document root must be object or array", document.get_error_message());
    }

    {
        const auto& document = parse(literal("[0 0]"));
        CHECK_EQUAL(false, document.is_valid());
        CHECK_EQUAL(1, document.get_error_line());
        //CHECK_EQUAL(3, document.get_error_column());
        CHECK_EQUAL("expected ,", document.get_error_message());
    }

    {
        const auto& document = parse(literal("{0:0}"));
        CHECK_EQUAL(false, document.is_valid());
        CHECK_EQUAL(1, document.get_error_line());
        //CHECK_EQUAL(3, document.get_error_column());
        CHECK_EQUAL("object key must be quoted", document.get_error_message());
    }

    {
        const auto& document = parse(literal("{\"0\"}"));
        CHECK_EQUAL(false, document.is_valid());
        CHECK_EQUAL(1, document.get_error_line());
        //CHECK_EQUAL(3, document.get_error_column());
        CHECK_EQUAL("expected :", document.get_error_message());
    }

    {
        const auto& document = parse(literal("[truf"));
        CHECK_EQUAL(false, document.is_valid());
        CHECK_EQUAL(1, document.get_error_line());
        //CHECK_EQUAL(3, document.get_error_column());
        CHECK_EQUAL("expected 'true'", document.get_error_message());
    }

    {
        const auto& document = parse(literal("[tru"));
        CHECK_EQUAL(false, document.is_valid());
        CHECK_EQUAL(1, document.get_error_line());
        //CHECK_EQUAL(3, document.get_error_column());
        CHECK_EQUAL("unexpected end of input", document.get_error_message());
    }

    {
        const auto& document = parse(literal("[}"));
        CHECK_EQUAL(false, document.is_valid());
        CHECK_EQUAL(1, document.get_error_line());
        //CHECK_EQUAL(3, document.get_error_column());
        CHECK_EQUAL("expected ]", document.get_error_message());
    }

    {
        const auto& document = parse(literal("{]"));
        CHECK_EQUAL(false, document.is_valid());
        CHECK_EQUAL(1, document.get_error_line());
        //CHECK_EQUAL(3, document.get_error_column());
        CHECK_EQUAL("object key must be quoted", document.get_error_message());
    }
}
