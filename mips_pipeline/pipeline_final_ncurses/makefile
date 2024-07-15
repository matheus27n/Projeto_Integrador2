CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lncurses

# Objetos do projeto
OBJS = main.o pipeline.o

# Nome do executável final
EXEC = exec

# Regra padrão para construir o executável
all: $(EXEC)

# Regra para construir o executável
$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC) $(LDFLAGS)

# Regra para compilar main.c
main.o: main.c pipeline.h
	$(CC) $(CFLAGS) -c main.c

# Regra para compilar pipeline.c
pipeline.o: pipeline.c pipeline.h
	$(CC) $(CFLAGS) -c pipeline.c

# Limpeza dos arquivos objetos e executáveis
clean:
	rm -f $(OBJS) $(EXEC)
