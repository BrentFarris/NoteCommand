UNAME		:=	$(shell uname)
rwildcard	=	$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))
CC			=	clang
NOWARNS		:=	-Wno-unused-parameter					\
				-Wno-missing-braces						\
				-Wbad-function-cast						\
				-Wno-address-of-temporary				\
				-Wno-reorder-ctor						\
				-Wno-deprecated-enum-enum-conversion	\
				-Wno-deprecated-volatile
NCNAME		:=	NoteCommand
NCPATH		:=	./$(NCNAME)
NCINC		:=	-I./src
NCLIBS		:=	-ldl		\
				-lm			\
				-lpthread	\
				-lncurses
NC_DEFINES	:=	-DLUA_USE_LINUX					\
				-DSQLITE_ENABLE_FTS5
NC_C		:=	$(call rwildcard,./src/,*.c)
NC_SRC 		:=	$(NC_C)
NC_OBJS		:=	$(NC_C:.c=.o)
DEBUG_DEFINES	:=	-D_DEBUG
RELEASE_DEFINES :=	-DNDEBUG

################################################################################
# Compilation options                                                          #
################################################################################
CFLAGS = -O0 -g -W -Wall -Werror $(NOWARNS) -fPIC $(CXXFLAGS) $(DEBUG_DEFINES) $(DEFINES)
#CFLAGS = -O2 -W -Wall -Werror $(NOWARNS) -fPIC $(CXXFLAGS) $(RELEASE_DEFINES) $(DEFINES)

.c.o:
	$(CC) $(CFLAGS) -std=gnu17 $(INCLUDES) -c $< -o $@

.PHONY: all clean debug

################################################################################
# Build targets                                                                #
################################################################################
debug: INCLUDES = $(NCINC)
debug: DEFINES = $(NC_DEFINES)
debug: $(NC_OBJS) $(NC_SRC)
	$(CC) $(NC_OBJS) $(NCINC) $(NCLIBS) -o $(NCPATH)

# Cleaning rule
clean:
	$(RM) $(NC_OBJS)
	$(RM) $(NCPATH)
	$(RM) *~

all: clean debug
