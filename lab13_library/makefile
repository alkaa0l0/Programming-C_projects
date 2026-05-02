.PHONY: clean

CC = gcc
CFLAGS = -Wall  
TARGET = crypto_program
INCLUDES = -I./include


all: $(TARGET)

# Цель: берем все .o файлы и линкуем в таргет
$(TARGET): src/crypto.o src/main.o
	$(CC) $(CFLAGS) $^ -o $@


# Универсальное правило для объектных файлов
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.o $(TARGET)

