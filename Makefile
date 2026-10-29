CC      := gcc
CFLAGS  := -Wall -Wextra -std=c11 -Iinclude -MMD -MP
TARGET  := scheduler
BUILD   := build

SRCS := $(wildcard src/*.c)
OBJS := $(SRCS:src/%.c=$(BUILD)/%.o)
DEPS := $(OBJS:.o=.d)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# -MMD -MP sinh build/<tên>.d liệt kê các header mà .o phụ thuộc,
# nên sửa scheduler.h sẽ tự biên dịch lại mọi file include nó.
$(BUILD)/%.o: src/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD):
	mkdir -p $@

# make test: chạy tests/test_all.c với mọi module trừ main.c
test: $(filter-out $(BUILD)/main.o,$(OBJS)) tests/test_all.c | $(BUILD)
	$(CC) $(CFLAGS) $^ -o $(BUILD)/test_all
	./$(BUILD)/test_all

clean:
	rm -rf $(BUILD) $(TARGET)

.PHONY: clean test

-include $(DEPS)
