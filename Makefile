NAME := scop

CXX := g++
CC := cc

CPPFLAGS += -Iglad/include
CXXFLAGS := -O2 -std=c++17 -fPIE
CFLAGS := -O2 -fPIE

LDFLAGS :=
LDLIBS := -lglfw -lGL -ldl

SRC_DIRS := src src/math src/parser src/mesh
BUILD_DIR := build

SRCS := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))
SRCS += glad/src/glad.c

OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(filter %.cpp,$(SRCS)))
OBJS += $(patsubst %.c,$(BUILD_DIR)/%.o,$(filter %.c,$(SRCS)))
DEPS := $(OBJS:.o=.d)

GLAD_FILES := glad/include/glad/glad.h \
              glad/include/KHR/khrplatform.h \
              glad/src/glad.c

.PHONY: all clean fclean re run debug glad

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

$(OBJS) $(GLAD_FILES): | glad

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

glad:
	@if [ ! -f glad/include/glad/glad.h ] || \
	    [ ! -f glad/include/KHR/khrplatform.h ] || \
	    [ ! -f glad/src/glad.c ]; then \
		python3 -m pip install --user -q glad==0.1.36 && \
		python3 -m glad --quiet --generator=c --out-path=glad \
		    --profile=core --api=gl=3.3 --spec=gl --extensions=""; \
	fi

run: all
	./$(NAME) $(ARGS)

debug: CXXFLAGS += -g3 -O0
debug: re

clean:
	rm -rf $(BUILD_DIR)

fclean: clean
	rm -f $(NAME)
	rm -rf glad

re: fclean all