# make -d prints debug info
# makefile rules are specified here: https://www.gnu.org/software/make/manual/html_node/Rule-Syntax.html

# use all cores if possible
MAKEFLAGS += -j $(shell nproc)

CXX := clang++
DB := lldb

# directories to search for .cpp or .h files
DIRS := src
SRCS := $(wildcard src/*.cpp)

# create object files and dependancy files in hidden dirs
OBJDIR := .obj
DEPDIR := .dep

LIBS := glfw3 assimp glm vulkan
LIB_CFLAGS := $(shell pkg-config --cflags $(LIBS))
LIB_LDFLAGS := $(shell pkg-config --libs $(LIBS))

# generate dependancy information, and stick it in depdir
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

CFLAGS := -Wall -Wextra -std=c++17 $(LIB_CFLAGS)
LDFLAGS := -fuse-ld=lld $(LIB_LDFLAGS)

# if any word (delimited by whitespace) of SRCS (excluding suffix) matches the wildcard '%', put it in the object or dep directory
OBJS := $(patsubst %,$(OBJDIR)/%.o,$(basename $(SRCS)))
DEPS := $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS)))

# make hidden subdirectories
$(shell mkdir -p $(dir $(OBJS)) > /dev/null)
$(shell mkdir -p $(dir $(DEPS)) > /dev/null)

.PHONY: default clean spv getprofile
BINS := dbg opt small check 

default: dbg

# tuned debug info, basic optimization
dbg: CFLAGS += -g$(DB) -Og

# fastest executable on current machine
opt: CFLAGS += -Ofast -march=native -ffast-math -flto=thin -DNDEBUG
opt: LDFLAGS += -flto=thin

# smaller executable
small: CFLAGS += -Os -DNDEBUG

# clean out .o and executable files
clean:
	@rm -f $(BINS)
	@rm -rf .dep .obj
	@rm -f default.prof* times.txt gmon.out

# build shaders
spv:
	@cd shader && $(MAKE)

# execute a profiling run
# view profile with 'perf report'
# list events to record with 'perf list'
getprofile: dbg
	@perf record --call-graph dwarf ./dbg
	@echo "profile generated"

# link executable together using object files in OBJDIR
$(BINS): $(OBJS)
	@$(CXX) -o $@ $(LDFLAGS) $^
	@echo linked $@

# if a dep file is available, include it as a dependancy
# when including the dep file, don't let the timestamp of the file determine if we remake the target since the dep
# is updated after the target is built
$(OBJDIR)/%.o: %.cpp
$(OBJDIR)/%.o: %.cpp | $(DEPDIR)/%.d
	@$(CXX) -c -o $@ $< $(CFLAGS) $(DEPFLAGS)
	@mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d
	@echo built $(notdir $@)

# dep files are not deleted if make dies
.PRECIOUS: $(DEPDIR)/%.d

# empty dep for deps
$(DEPDIR)/%.d: ;

# read .d files if nothing else matches, ok if deps don't exist...?
-include $(DEPS)
