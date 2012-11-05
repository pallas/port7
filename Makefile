default: all

PROGS:= port7

.PHONY: all
all: $(PROGS)

test: $(PROGS)
	./port7 & sleep 1
	yes xyzzy | head -n 5 | nc -w 1 localhost 7 &
	seq -w 0 99 | xargs -n 10 echo | nc -w 1 localhost 7 &
	sleep 1 ; killall port7 ; wait

$(PROGS) : LDFLAGS+= -g -levent
$(PROGS) : CXXFLAGS+=

.PHONY: clean
clean:
	rm -rf $(PROGS)

#