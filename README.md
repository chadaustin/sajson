Single-Allocation JSON Parser

sajson is an in-place, DOM-style JSON parser written in C++.  Given a
JSON document, it allocates and fills a contiguous parse tree of size
`input_length * sizeof(size_t)`.

That is, on 32-bit platforms, sajson allocates 4 bytes per input
character.  On 64-bit platforms, sajson allocates 8 bytes per input character.

Its performance is similar to rapidjson and vjson.

Implementation details are available at: http://chadaustin.me/tag/sajson/

sajson does not support UTF-16 or UTF-32.  To use sajson on non-UTF-8
documents, transcode to UTF-8 first.
