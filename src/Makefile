CC = gcc
# CFLAGS = -Wall -Wextra -g -O2 -std=c99
CFLAGS = -Wall -g -std=c99 # Para debug inicial

# Tenta usar pkg-config para encontrar flags do X11
X11_CFLAGS := $(shell pkg-config --cflags x11)
X11_LIBS := $(shell pkg-config --libs x11)

# Fallback se pkg-config não estiver disponível ou não encontrar x11
# (Você pode precisar ajustar esses caminhos para o seu sistema)
ifeq ($(X11_CFLAGS),)
    X11_CFLAGS = -I/usr/X11R6/include
    # ou -I/usr/include/X11 (em sistemas mais novos)
endif
ifeq ($(X11_LIBS),)
    X11_LIBS = -L/usr/X11R6/lib -lX11
    # ou -L/usr/lib/X11 -lX11 ou apenas -lX11
endif

# Adicionar -lm para a biblioteca matemática (sqrt, etc., se usar depois)
LIBS = $(X11_LIBS) -lm

# Arquivos fonte e objeto
SRCS = main.c data_loader.c x11_plotter.c clustering.c
OBJS = $(SRCS:.c=.o)
TARGET = data_visualizer

# Regra padrão
all: $(TARGET)

# Regra para linkar o executável final
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

# Regra para limpar arquivos compilados
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean