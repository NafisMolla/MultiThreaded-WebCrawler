# # Makefile, ECE252  
# #Nafis Molla

# CC = gcc       # compiler
# CFLAGS = -Wall -g -std=gnu99 -Ilib# compilation flags
# LD = gcc       # linker
# LDFLAGS = -g   # debugging symbols in build
# LDLIBS = -lz -lcurl -lpthread  # link with libz and lcurl

# OBJ_DIR = obj
# SRC_DIR = lib
# LIB_UTIL = $(OBJ_DIR)/zutil.o $(OBJ_DIR)/crc.o $(OBJ_DIR)/lab_png.o
# SRCS   = findpng2.c crc.c zutil.c lab_png.c
# OBJS_findpng2 = $(OBJ_DIR)/findpng2.o $(LIB_UTIL) 

# TARGETS= findpng2

# all: ${TARGETS}

# findpng2: $(OBJS_findpng2) 
# 	mkdir -p $(OBJ_DIR)
# 	$(LD) -o $@ $^ $(LDLIBS) $(LDFLAGS) 

# $(OBJ_DIR)/findpng2.o: findpng2.c 
# 	mkdir -p $(OBJ_DIR)
# 	$(CC) $(CFLAGS) -c $< -o $@

# $(OBJ_DIR)/%.o: $(SRC_DIR)/%.c 
# 	mkdir -p $(OBJ_DIR)
# 	$(CC) $(CFLAGS) -c $< -o $@

# %.o: %.c 
# 	$(CC) $(CFLAGS) -c $< 

# -include $(SRCS:.c=.d)

# .PHONY: clean
# clean:
# 	rm findpng2 obj/crc.o obj/lab_png.o obj/findpng2.o obj/zutil.o


# CC = gcc
# CFLAGS_XML2 = $(shell xml2-config --cflags)
# CFLAGS_CURL = $(shell curl-config --cflags)
# CFLAGS = -Wall $(CFLAGS_XML2) $(CFLAGS_CURL) -std=gnu99 -g -DDEBUG1_
# LD = gcc
# LDFLAGS = -std=gnu99 -g
# LDLIBS_XML2 = $(shell xml2-config --libs)
# LDLIBS_CURL = $(shell curl-config --libs)
# LDLIBS = $(LDLIBS_XML2) $(LDLIBS_CURL) -pthread

# OBJ_DIR = obj
# # OBJS_FINDPNG2 = $(OBJ_DIR)/paster2.o $(LIB_UTIL) 

# SRCS   = findpng2.c
# OBJS3  = findpng2.o
# TARGETS= findpng2

# all: ${TARGETS}

# findpng2: $(OBJS3)
# 	$(LD) -o $@ $^ $(LDLIBS) $(LDFLAGS)

# # %.o: %.c
# # 	$(CC) $(CFLAGS) -c $<

# # %.d: %.c
# # 	gcc -MM -MF $@ $<

# -include $(SRCS:.c=.d)

# .PHONY: clean
# clean:
# 	rm -f *~ *.d *.o $(TARGETS) *.png *.html *.txt


CC = gcc
CFLAGS_XML2 = $(shell xml2-config --cflags)
CFLAGS_CURL = $(shell curl-config --cflags)
CFLAGS = -Wall $(CFLAGS_XML2) $(CFLAGS_CURL) -g -std=gnu99
LD = gcc
LDFLAGS = -std=gnu99 -g
LDLIBS_XML2 = $(shell xml2-config --libs)
LDLIBS_CURL = $(shell curl-config --libs)
LDLIBS = -lcurl -lpthread -lz $(LDLIBS_XML2) $(LDLIBS_CURL)

SRCS   = findpng.h
TARGETS = findpng2

all: $(TARGETS)

findpng2: findpng2.o 
	$(LD) -o $@ $^ $(LDLIBS) $(LDFLAGS)

lib/%.o: %.c 
	$(CC) $(CFLAGS) -c $< 

%.d: %.c
	gcc -MM -MF $@ $<

-include $(SRCS:.c=.d)

.PHONY: clean
clean:
	rm -f *.d *.o lib/*.d lib/*.o *.txt $(TARGETS)
