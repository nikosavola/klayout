DESTDIR = $$OUT_PWD/../..
TARGET = db_parallel_benchmarks

include($$PWD/../../klayout.pri)

TEMPLATE = app
CONFIG += console link_pkgconfig
mac: CONFIG -= app_bundle
equals(HAVE_QT, "0"): CONFIG -= qt

PKGCONFIG += benchmark

SOURCES = dbParallelBenchmarks.cc
INCLUDEPATH += $$TL_INC $$DB_INC $$GSI_INC
DEPENDPATH += $$TL_INC $$DB_INC $$GSI_INC
LIBS += -L$$DESTDIR -lklayout_db -lklayout_tl -lklayout_gsi
