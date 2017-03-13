############################################################
## makefile of balancer
###########################################################
.SUFFIXES: .cpp

SOURCES = \
        Common.cpp InnerInitiator.cpp InnerRunner.cpp Log.cpp Main.cpp OuterRunner.cpp Runner.cpp RunnerManager.cpp


HOME_PATH = .
INCL_PATH = -I.
        
LIBS          = 
SYS_INCL_PATH =
SYS_LIB_PATH  =
SYS_LIBS = 
GCOV_FLAG =

CCC=g++
CCFLAGS  = -g -w ${INCL_PATH} ${SYS_INCL_PATH}

AR       = ar
ARFLAGS  = -ruv

TARGET_PATH= .
INSTALL_PATH= /SimpleProxy/bin

OBJECTS=${SOURCES:%.cpp=%.o}

TARGET= ${TARGET_PATH}/SimpleProxy


all: ${TARGET}

${TARGET}: ${OBJECTS}
	if [ ! -d ${TARGET_PATH} ]; then mkdir -p ${TARGET_PATH}; fi
	$(CCC) $(OBJECTS) $(LIBS) $(GCOV_FLAG) -o $(TARGET)

.cpp.o:
	$(CCC) $(CCFLAGS) $(GCOV_FLAG) -o $@ -c $< 
	
install:
	if [ ! -d ${INSTALL_PATH} ]; then mkdir -p ${INSTALL_PATH}; fi
	cp $(TARGET) $(INSTALL_PATH)
	cp ${TARGET_PATH}/config.ini $(INSTALL_PATH)

clean:
	rm -rf ${OBJECTS} ${TARGET}

