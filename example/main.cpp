#include <assert.h>
#include "sajson.h"

using namespace sajson;

inline bool success(const document& doc) {
    if (!doc.is_valid()) {
        fprintf(stderr, "%s\n", doc.get_error_message().c_str());
        return false;
    }
    return true;
}

int main(int argc, char** argv) {
    const sajson::document& document = parse(literal(argv[0]));
    assert(success(document));
    const value& root = document.get_root();
    (void)root;
}
