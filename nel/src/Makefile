
BUILD_DIRS = \
misc\
net\
pacs\
memory\
georges\
ligo\
logic\

night: all

all: $(foreach dir,$(BUILD_DIRS),$(dir)-all)

%-all:
	+make -C $* all

clean: $(foreach dir,$(BUILD_DIRS),$(dir)-clean)

%-clean:
	+make -C $* clean

update: $(foreach dir,$(BUILD_DIRS),$(dir)-update)

%-update:
	+make -C $* update

.NOTPARALLEL: night all clean update 
