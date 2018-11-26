all: shell 

clean:
	rm -vf shell
	
TARGET = shell	
SOURCES = newstage2.c
	
$(TARGET): $(SOURCES)
	gcc $(SOURCES) -g -o $(TARGET)
	
