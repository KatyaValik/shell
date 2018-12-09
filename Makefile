all: shell 

clean:
	rm -vf shell reading

reading: reading.c
	gcc reading.c -g -o reading
	
TARGET = shell	
SOURCES = newstage2.c
	
$(TARGET): $(SOURCES)
	gcc $(SOURCES) -g -o $(TARGET)
	
