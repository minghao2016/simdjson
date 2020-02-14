#include <benchmark/benchmark.h>
#include "simdjson/document.h"
#include "simdjson/jsonparser.h"
using namespace simdjson;

static void document_parse_take(benchmark::State& state) {
  for (auto _ : state) {
    document doc = document::parse("[]");
  }
}
BENCHMARK(document_parse_take);
static void document_parse_try(benchmark::State& state) {
  for (auto _ : state) {
    auto [doc, error] = document::parse("[]");
    if (error) { return; }
  }
}
BENCHMARK(document_parse_try);
static void build_parsed_json(benchmark::State& state) {
  for (auto _ : state) {
    document::parser parser = simdjson::build_parsed_json(std::string("[]"));
    if (!parser.valid) { return; }
  }
}
BENCHMARK(build_parsed_json);
static void parser_parse(benchmark::State& state) {
  document::parser parser;
  if (!parser.allocate_capacity(2)) { return; }
  for (auto _ : state) {
    document &doc = parser.parse(std::string("[]"));
  }
}
BENCHMARK(parser_parse);
static void parser_parse_try(benchmark::State& state) {
  document::parser parser;
  if (!parser.allocate_capacity(2)) { return; }
  for (auto _ : state) {
    auto [doc, error] = parser.parse(std::string("[]"));
    if (error) { return; }
  }
}
BENCHMARK(parser_parse_try);
static void json_parse(benchmark::State& state) {
  document::parser parser;
  if (!parser.allocate_capacity(2)) { return; }
  for (auto _ : state) {
    if (simdjson::json_parse(std::string("[]"), parser)) { return; }
  }
}
BENCHMARK(json_parse);

BENCHMARK_MAIN();