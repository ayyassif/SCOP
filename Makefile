NAME := scop

CXX := g++
CPPFLAGS := -Iglad/include
CXXFLAGS := -O2 -std=c++17
LDFLAGS :=
LDLIBS := -lglfw -lGL -ldl

SRC_DIRS := . math glad/src
BUILD_DIR := build

SRCS := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))
SRCS += $(wildcard glad/src/*.c)

OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(filter %.cpp,$(SRCS)))
OBJS += $(patsubst %.c,$(BUILD_DIR)/%.o,$(filter %.c,$(SRCS)))
DEPS := $(OBJS:.o=.d)

.PHONY: all clean fclean re run debug

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

run: all
	./$(NAME)

debug: CXXFLAGS += -g3 -O0
debug: re

clean:
	rm -rf $(BUILD_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all
	