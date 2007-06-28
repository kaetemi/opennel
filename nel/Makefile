
MAKE_ARGS=-j4
#MAKE_ARGS=-j10
#CXX=distcc CC=distcc
BIN_DIR=/home/nevrax/cvs/code/ryzom/script/linux

night:
	+make $(MAKE_ARGS) -k -C src night
	if [ -e src_nel.tgz ] ; then rm src_nel.tgz; fi	
	if [ -e mktemp ] ; then rm mktemp; fi	
	find src -name "*.cpp" >> mktemp
	find src -name "*.h" >> mktemp
	find src -name "*.dsp" >> mktemp
	find src -name "*.def" >> mktemp
	find src -name "Makefile" >> mktemp
	find include -name "*.h" >> mktemp
	tar cvzf src_nel.tgz -T mktemp
	if [ -e mktemp ] ; then rm mktemp; fi	

all:
	+make $(MAKE_ARGS) -C src all

mono:
	+make -C src all

clean:
	+make -C src clean

update:
	+make -C src update

touch:
	+make -C src touch

