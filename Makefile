GCC := gcc
SRC_DIR := ./src
OBJ_DIR := ./obj
INC_DIR := -I./include
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))
CFLAGS := -Wall -g

main.exe: $(OBJ_FILES)
	$(GCC) $(LDFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(GCC) $(CFLAGS) $(INC_DIR) -c -o $@ $<

$(OBJ_DIR):
	mkdir -p $@
	
clean:
	rm -f $(OBJ_DIR)/*.o main.exe
