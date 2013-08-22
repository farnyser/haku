# Edit to match your configuration

CPP 		= g++ -DDEBUG=1
CPPFLAGS 	= -O2 -pipe -fomit-frame-pointer -march=native -ffast-math -mno-ieee-fp
LDFLAGS 	= -g -Wl,-O1 -Wl,--as-needed
LIBS 		= $(shell pkg-config --libs gtk+-2.0 webkit-1.0 gthread-2.0) 
INCS		= $(shell pkg-config --cflags gtk+-2.0 webkit-1.0)

OUT			= /usr/bin


# Do not edit below this line

VERSION		= magnesium
SRC     	= $(wildcard *.cpp tools/*.cpp)
OBJ     	= $(SRC:.cpp=.o)


all: project

install: project
	cp haku $(OUT)

uninstall: 
	rm $(OUT)/haku

project: $(OBJ) 
	$(CPP) $(OBJ) -o haku $(LDFLAGS) $(LIBS)

$(OBJ): %.o: %.cpp %.hpp config.hpp
	$(CPP) -DAPP_VERSION='"$(VERSION)"' $(CPPFLAGS) $(INCS) -o $@ -c $< 

.PHONY: clean mrproper

clean:
	@rm -rf $(OBJ)

mrproper: clean
	@rm -rf haku
