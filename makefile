LIBPATH=lib/lp_solve_ux64
OUTPATH=bin
SRCPATH=src
CC=g++

all: $(OUTPATH)/crpd

$(OUTPATH)/crpd: $(OUTPATH)/main.o $(OUTPATH)/global.o $(OUTPATH)/uniform_distribution.o $(OUTPATH)/kd.o $(OUTPATH)/lee.o $(OUTPATH)/set_operations.o
	$(CC) $(OUTPATH)/main.o $(OUTPATH)/global.o $(OUTPATH)/set_operations.o $(OUTPATH)/uniform_distribution.o $(OUTPATH)/kd.o $(OUTPATH)/lee.o $(LIBPATH)/liblpsolve55.a -ldl -lm -O3 -o $(OUTPATH)/crpd
	
$(OUTPATH)/main.o: $(SRCPATH)/main.cpp $(SRCPATH)/global.h $(SRCPATH)/uniform_distribution.h $(SRCPATH)/kd.h $(SRCPATH)/lee.h
	$(CC) -c $(SRCPATH)/main.cpp -o $(OUTPATH)/main.o

$(OUTPATH)/uniform_distribution.o: $(SRCPATH)/uniform_distribution.cpp $(SRCPATH)/uniform_distribution.h $(SRCPATH)/global.h
	$(CC) -c $(SRCPATH)/uniform_distribution.cpp -o $(OUTPATH)/uniform_distribution.o

$(OUTPATH)/kd.o: $(SRCPATH)/kd.cpp $(SRCPATH)/kd.h $(SRCPATH)/global.h $(SRCPATH)/set_operations.h
	$(CC) -c $(SRCPATH)/kd.cpp -I $(LIBPATH) -o $(OUTPATH)/kd.o

$(OUTPATH)/lee.o: $(SRCPATH)/lee.cpp $(SRCPATH)/lee.h $(SRCPATH)/global.h $(SRCPATH)/set_operations.h
	$(CC) -c $(SRCPATH)/lee.cpp -I $(LIBPATH) -o $(OUTPATH)/lee.o

$(OUTPATH)/global.o: $(SRCPATH)/global.cpp $(SRCPATH)/global.h
	$(CC) -c $(SRCPATH)/global.cpp -o $(OUTPATH)/global.o

$(OUTPATH)/set_operations.o: $(SRCPATH)/set_operations.cpp $(SRCPATH)/set_operations.h
	$(CC) -c $(SRCPATH)/set_operations.cpp -o $(OUTPATH)/set_operations.o

clean:
	rm -r $(OUTPATH)/*.o $(OUTPATH)/crpd
