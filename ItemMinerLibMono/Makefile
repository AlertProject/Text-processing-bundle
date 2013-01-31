CC=g++
CFLAGS=-g -fPIC -O0 -DUNIX -MMD -DMONODEVELOP -DNDEBUG
INC=-I"../_Tools/glib" -I"../_Tools/QMiner" -I"/usr/include"

all:
	rm -rf libItemMinerLib.so
	rm -rf *.o
	rm -rf *.d
	g++  -MMD "ItemMinerLib.cpp" $(CFLAGS) $(INC) -c -o "ItemMinerLib.o"
	g++  -MMD "Profile.cpp" $(CFLAGS) $(INC) -c -o "Profile.o"
	g++  -MMD "Stores.cpp" $(CFLAGS) $(INC) -c -o "Stores.o"
	g++  -MMD "xmlParsing.cpp" $(CFLAGS) $(INC) -c -o "xmlParsing.o"
	g++  -MMD "../_Tools/QMiner/qminer.cpp" $(CFLAGS) $(INC) -c -o "qminer.o"
	g++  -MMD "../_Tools/glib/mine.cpp" $(CFLAGS) $(INC) -c -o "mine.o"
	g++  -MMD "../_Tools/glib/base.cpp" $(CFLAGS) $(INC) -c -o "base.o"
	g++ -shared -o "libItemMinerLib.so" "ItemMinerLib.o" "Profile.o" "Stores.o" "xmlParsing.o" "qminer.o" "mine.o" "base.o" -luuid

clean:
	rm -rf *.o
	rm -rf *.d