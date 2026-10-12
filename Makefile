TARGET   := SR2_ModMenu.elf
OBJS     := main.o
CC       := ppu-gcc
CXX      := ppu-g++
CXXFLAGS := -O2 -Wall

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET)

clean:
	rm -f *.o $(TARGET)
