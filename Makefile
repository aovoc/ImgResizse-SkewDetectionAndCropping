#mushi.lm makefile template

PROJECT := SkewDetectionAndRotation
CXX := /usr/bin/g++

OPENCV_DIR=opencv-2.4.9
TEST_DIR=test

INC_DIRS :=  $(PWD)/include $(PWD)/lib/$(OPENCV_DIR)/include 
 			
LIB_DIRS := $(PWD)/lib/$(OPENCV_DIR)/lib

         
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
TEST_DIR := test

LIBRARIES := opencv_core opencv_highgui opencv_imgproc opencv_ml opencv_objdetect

	
COMMON_FLAGS := -DNDEBUG -O2 $(foreach includedir,$(INC_DIRS),-I$(includedir))
CXXFLAGS += -pthread -fPIC $(COMMON_FLAGS)
LDFLAGS= -Wl,--rpath=$(PWD)/lib/opencv-2.4.9/lib -Wl,--rpath=$(PWD)/bin/
LDFLAGS += $(foreach librarydir,$(LIB_DIRS),-L$(librarydir)) \
		$(foreach library,$(LIBRARIES),-l$(library)) \

		
CXX_SRCS := $(shell find $(SRC_DIR) -name "*.cpp")
TEST_SRCS := $(shell find $(TEST_DIR) -name "*.cpp")
CXX_OBJS := $(addprefix $(OBJ_DIR)/,$(subst $(SRC_DIR)/, ,${CXX_SRCS:.cpp=.o}))
TEST_OBJS := $(addprefix $(BIN_DIR)/,$(subst $(TEST_DIR)/, ,${TEST_SRCS:.cpp=.o}))
TEST_BINS := ${TEST_OBJS:.o=.bin}
		

		
##############################
# Define build targets
##############################
REC_SO=$(BIN_DIR)/lib$(PROJECT).so
all: $(REC_SO) $(TEST_BINS)

#link
$(REC_SO):$(CXX_OBJS) 
	$(CXX) $(CXX_OBJS) -o $@ -shared $(CXXFLAGS) $(LDFLAGS) $(STATIC_LIBS)

$(TEST_BINS): %.bin : %.o $(STATIC_NAME)
	$(CXX) $< $(STATIC_NAME) -o $@ $(LINKFLAGS) $(LDFLAGS) -l$(PROJECT) -L./bin

#compile
$(OBJ_DIR)/%.o:$(SRC_DIR)/%.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS) $(LDFLAGS)

$(BIN_DIR)/%.o:$(TEST_DIR)/%.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS) $(LDFLAGS)
	
clean:
	rm -fr $(CXX_OBJS) $(TEST_OBJS) $(REC_SO) $(TEST_BINS)
	
