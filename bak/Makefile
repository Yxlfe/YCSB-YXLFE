CC=g++
CXXFLAGS= -std=c++11 -g -Wall -I./ 
LDFLAGS= -lpthread 


SUBDIRS= core db blockdb

SUBSRCS=$(wildcard core/*.cc) $(wildcard db/*.cc)
YCSB_OBJECTS=$(SUBSRCS:.cc=.o)

##rocksdb
# ROCKSDB_SOURCES= $(wildcard rocksdb/*.cc)
# ROCKSDB_LIBRARY= -lrocksdb -lz    ### -lz -lzstd -llz4 -lsnappy是rocksdb编译的时候如果检测到这些压缩链接库会使用，所以这里也要添加
# ROCKSDB_DEFS= -DYCSB_ROCKSDB
# ROCKSDB_OBJECTS=$(ROCKSDB_SOURCES:.cc=.o)
##

##blockdb
BLOCKDBDB_SOURCES= $(wildcard blockdb/*.cc)
BLOCKDBDB_LIBRARY= -lleveldb -lsnappy -laio
BLOCKDBDB_OBJECTS=$(BLOCKDBDB_SOURCES:.cc=.o)
##

EXEC=ycsbc

all: clean $(SUBDIRS) $(EXEC)
$(SUBDIRS):
	$(MAKE) -C $@

$(EXEC): $(wildcard *.cc) $(YCSB_OBJECTS) $(BLOCKDBDB_OBJECTS)
	$(CC) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(BLOCKDBDB_LIBRARY)


# rocksdb: clean
# 	$(CC) $(CXXFLAGS) $(ROCKSDB_DEFS) ycsbc.cc $(ROCKSDB_OBJECTS) -o $(EXEC) $(LDFLAGS) $(ROCKSDB_LIBRARY)


clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done
	$(RM) $(EXEC)

.PHONY: all clean $(SUBDIRS) $(EXEC)

