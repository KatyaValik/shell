all: shell 

clean:
	rm -vf shell

reading:
	gcc reading.c -o reading
	
TARGET = shell	
SOURCES = newstage2.c
	
$(TARGET): $(SOURCES)
	gcc $(SOURCES) -g -o $(TARGET)
	
