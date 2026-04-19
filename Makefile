CC = gcc
CFLAGS = -Wall -Wextra -g

# Все исходники
SRCS := $(wildcard *.c)
HDRS := $(wildcard *.h)

# Тестовые исходники
TEST_SRCS := $(wildcard *_test.c)

# Тестовые бинарники
TEST_BINS := $(patsubst %_test.c,%_test,$(TEST_SRCS))

.PHONY: all clean test format check-format


all: format test
# Сборка
%: %.c
	$(CC) $(CFLAGS) $< -o $@ -lm

# Запуск всех тестов
test: clean $(TEST_BINS)
	for test in $(TEST_BINS); do ./$$test; done

# Форматирование
check-format:
	clang-format --dry-run -Werror $(SRCS) $(HDRS)

format:
	clang-format -i $(SRCS) $(HDRS)

# Очистка
clean:
	rm -f $(TEST_BINS)
