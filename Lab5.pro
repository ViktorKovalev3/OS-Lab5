TEMPLATE = subdirs

SUBDIRS += Writer Reader

Reader.depends = Writer
