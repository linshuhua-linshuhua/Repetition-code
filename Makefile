CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
INCLUDES = -I.

# Source files
SOURCES = fru_parser_impl.cpp
HEADERS = fru_parser.h

# Targets
TARGETS = fru_parser test_fru_parser

.PHONY: all clean test

all: $(TARGETS)

# Main executable (single file version)
fru_parser: fru_parser.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

# Test executable (modular version)
test_fru_parser: test_fru_parser.cpp fru_parser_impl.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Run tests
test: test_fru_parser
	./test_fru_parser

# Clean build artifacts
clean:
	rm -f $(TARGETS)

# Install dependencies (if needed)
install-deps:
	@echo "Make sure ipmitool is installed on your system:"
	@echo "  Ubuntu/Debian: sudo apt-get install ipmitool"
	@echo "  RHEL/CentOS: sudo yum install ipmitool"
	@echo "  OpenBMC: ipmitool should be available by default"