LDLIBS += -lpcap

all: sniff

sniff: sniff.c

clean:
	rm -f sniff
