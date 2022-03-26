JAVA = java
JAVAC = javac
CLASSPATH = .:../lib/lkc.jar:./junit-4.5.jar
CLASSES = $(patsubst %.java,%.class,$(wildcard Test*.java))

test: $(CLASSES)
	$(JAVA) -classpath $(CLASSPATH) TestAll

%.class: %.java
	$(JAVAC) -classpath $(CLASSPATH) $<
