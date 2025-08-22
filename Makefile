CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
TARGET = bios_config_mapper
SOURCES = bios_config_mapper.c
HEADERS = bios_config_tables.h

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

test: $(TARGET)
	./$(TARGET)

install: $(TARGET)
	sudo mkdir -p /var/backups/redfish
	sudo chmod 755 /var/backups/redfish

.PHONY: clean test install