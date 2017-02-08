TEMPLATE = subdirs

SUBDIRS += itemframework \
           tests

CONFIG += ordered
tests.depends = itemframework
