# Makefile — primary verified build on macOS (clang++ / C++20).
# The team's Windows workflow uses CMakeLists.txt; this Makefile builds the same
# pure-C++20 core headlessly so P1 can prove everything works without the MFC GUI.
#
#   make            # build tests + runner + seedhunt
#   make test       # build and run the unit tests
#   make run        # build and run the headless demo (seed 42, 365 days, pop 80)
#   make clean

CXX      := clang++
CC       := clang
INCLUDES := -I. -Ithird_party/doctest -Ithird_party
CXXFLAGS := -std=c++20 -O2 -Wall -Wextra -MMD -MP $(INCLUDES)
CFLAGS   := -std=c11 -O2 -Wall -MMD -MP $(INCLUDES)
LDFLAGS  :=

BUILD    := build
CORE_SRC := $(filter-out core/sim/HeadlessMain.cpp,$(wildcard core/*/*.cpp))
CORE_OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(CORE_SRC))
TEST_SRC := $(wildcard tests/unit/*.cpp)
TEST_OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(TEST_SRC))
REF_OBJ  := $(BUILD)/tests/reference/prng_ref.o
ALL_OBJ  := $(CORE_OBJ) $(TEST_OBJ) $(REF_OBJ) $(BUILD)/core/sim/HeadlessMain.o $(BUILD)/tools/SeedHunt.o

.PHONY: all test run clean
all: $(BUILD)/tge_tests $(BUILD)/runner $(BUILD)/seedhunt

$(BUILD)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(REF_OBJ): tests/reference/prng_ref.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/tge_tests: $(CORE_OBJ) $(TEST_OBJ) $(REF_OBJ)
	$(CXX) $(LDFLAGS) $^ -o $@

$(BUILD)/runner: $(CORE_OBJ) $(BUILD)/core/sim/HeadlessMain.o
	$(CXX) $(LDFLAGS) $^ -o $@

$(BUILD)/seedhunt: $(CORE_OBJ) $(BUILD)/tools/SeedHunt.o
	$(CXX) $(LDFLAGS) $^ -o $@

test: $(BUILD)/tge_tests
	./$(BUILD)/tge_tests

run: $(BUILD)/runner
	./$(BUILD)/runner --seed 42 --days 365 --pop 80

clean:
	rm -rf $(BUILD)

# Auto-generated header dependencies (so editing a .h rebuilds dependents).
-include $(ALL_OBJ:.o=.d)
