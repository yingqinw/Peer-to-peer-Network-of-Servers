MYDEFS = -g -Wall -std=c++11 -DLOCALHOST=\"127.0.0.1\" -lcrypto -lssl -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib

pa5: pa5.cpp parse_ini_file.o my_socket.o my_readwrite.o my_timestamp.o util.o logging.o create_read_message.o reaper.o timer.o
	g++ ${MYDEFS} $^ -o pa5 -lcrypto -lssl -pthread

lab14b: lab14b.cpp parse_ini_file.o my_socket.o my_readwrite.o my_timestamp.o util.o logging.o create_read_message.o reaper.o timer.o
	g++ ${MYDEFS} $^ -o lab14b -lcrypto -lssl -pthread

timer.o: timer.h timer.cpp my_timestamp.o 
	g++ ${MYDEFS} -c timer.cpp -o $@

reaper.o: reaper.h reaper.cpp connection.h logging.o
	g++ ${MYDEFS} -c reaper.cpp -o $@

create_read_message.o: create_read_message.h create_read_message.cpp connection.h util.o my_readwrite.o
	g++ ${MYDEFS} -c create_read_message.cpp -o $@

logging.o: logging.h logging.cpp my_timestamp.o
	g++ ${MYDEFS} -c logging.cpp -o $@

my_socket.o: my_socket.h my_socket.cpp
	g++ ${MYDEFS} -c my_socket.cpp -o $@

my_readwrite.o: my_readwrite.h my_readwrite.cpp
	g++ ${MYDEFS} -c my_readwrite.cpp -o $@

parse_ini_file.o: parse_ini_file.h parse_ini_file.cpp
	g++ ${MYDEFS} -c parse_ini_file.cpp -o $@

my_timestamp.o: my_timestamp.h my_timestamp.cpp
	g++ ${MYDEFS} -c my_timestamp.cpp -o $@

util.o: util.h util.cpp
	g++ ${MYDEFS} -c util.cpp -o $@


clean:
	rm -f pa5 *.o 


