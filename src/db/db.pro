
TEMPLATE = subdirs
SUBDIRS = db unit_tests

unit_tests.depends += db

equals(HAVE_BENCHMARK, "1") {
  SUBDIRS += benchmarks
  benchmarks.depends += db
}
