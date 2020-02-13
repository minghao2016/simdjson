#ifndef SIMDJSON_DOCUMENT_H
#define SIMDJSON_DOCUMENT_H

#include <cstring>
#include <memory>
#include "simdjson/common_defs.h"
#include "simdjson/simdjson.h"
#include "simdjson/padded_string.h"

#define JSON_VALUE_MASK 0xFFFFFFFFFFFFFF
#define DEFAULT_MAX_DEPTH 1024 // a JSON document with a depth exceeding 1024 is probably de facto invalid

namespace simdjson {

template<size_t max_depth> class document_iterator;

class document {
public:
  // create a document container with zero capacity, parser will allocate capacity as needed
  document()=default;
  ~document()=default;

  // this is a move only class
  document(document &&p) = default;
  document(const document &p) = delete;
  document &operator=(document &&o) = default;
  document &operator=(const document &o) = delete;

  using iterator = document_iterator<DEFAULT_MAX_DEPTH>;
  class doc_result;

  //
  // Tell whether this document has been parsed, or is just empty.
  //
  bool is_initialized() {
    return tape && string_buf;
  }

  // print the json to std::ostream (should be valid)
  // return false if the tape is likely wrong (e.g., you did not parse a valid
  // JSON).
  WARN_UNUSED
  bool print_json(std::ostream &os, size_t max_depth=DEFAULT_MAX_DEPTH) const;
  WARN_UNUSED
  bool dump_raw_tape(std::ostream &os) const;

  class parser;
  
  //
  // Parse a JSON document.
  //
  // If you will be parsing more than one JSON document, it's recommended to create a
  // document::parser object instead, keeping internal buffers around for efficiency reasons.
  //
  // Throws invalid_json if the JSON is invalid.
  //
  static doc_result parse(const uint8_t *buf, size_t len, bool realloc_if_needed = true);
  static doc_result parse(const char *buf, size_t len, bool realloc_if_needed = true);
  static doc_result parse(const std::string &s, bool realloc_if_needed = true);
  static doc_result parse(const padded_string &s);

  std::unique_ptr<uint64_t[]> tape;
  std::unique_ptr<uint8_t[]> string_buf;// should be at least byte_capacity

private:
  bool set_capacity(size_t len);
};

class document::doc_result {
private:
  doc_result(document &&_value, error_code _error) : value(std::move(_value)), error(_error) { }
  doc_result(document &&_value) : value(std::move(_value)), error(SUCCESS) { }
  doc_result(error_code _error) : value(), error(_error) { }
  friend class document;
public:
  ~doc_result()=default;

  operator bool() noexcept { return error == SUCCESS; }
  operator document() {
    if (!*this) {
      throw invalid_json(error);
    }
    return std::move(value);
  }
  document value;
  error_code error;
};

} // namespace simdjson

#include "simdjson/document/parser.h"
#include "simdjson/document/iterator.h"

// Implementations
namespace simdjson {

inline document::doc_result document::parse(const char *buf, size_t len, bool realloc_if_needed) {
    return parse((const uint8_t *)buf, len, realloc_if_needed);
}
inline document::doc_result document::parse(const std::string &s, bool realloc_if_needed) {
    return parse(s.data(), s.length(), realloc_if_needed);
}
inline document::doc_result document::parse(const padded_string &s) {
    return parse(s.data(), s.length(), false);
}
inline WARN_UNUSED document::doc_result document::parse(const uint8_t *buf, size_t len, bool realloc_if_needed) {
  document::parser parser;
  if (!parser.allocate_capacity(len)) {
    return MEMALLOC;
  }
  auto [doc, error] = parser.parse(buf, len, realloc_if_needed);
  return document::doc_result((document &&)doc, error);
}

} // namespace simdjson

#endif // SIMDJSON_DOCUMENT_H