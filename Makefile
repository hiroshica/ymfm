####################################################################
TARGET = vgmrender


# ソースコードの位置
SRCROOT   = src examples/vgmrender/
# 中間バイナリファイルの出力ディレクトリ
OBJROOT   = objs
# ソースディレクトリの中を(shellの)findコマンドで走破してサブディレクトリまでリスト化する
SRCDIRS  := $(foreach dir,$(SRCROOT), $(shell find $(dir) -type d))
# ソースディレクトリを元にforeach命令で全cppファイルをリスト化する
SOURCESCPP	= $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.cpp))
SOURCESC	= $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))

# 上記のcppファイルのリストを元にオブジェクトファイル名を決定
OBJSCPP = $(SOURCESCPP:.cpp=.o)
OBJSC   = $(SOURSOURCESC:.c=.o)

OBJECTS = $(OBJSCPP) $(OBJSC)

#############################################
CCFLAGS= -fpermissive -O0 -g
CFLAGS=$(CCFLAGS)

IFLAGS	=	-I./src -I./examples/vgmrender

#############################################
all: $(TARGET)

#############################################
$(TARGET) : $(OBJECTS) 
	g++ -o $@ $(OBJECTS) 

####################################################################
####################################################################
#.cpp.o:
%.o : %.cpp
	g++ $(CFLAGS) $(IFLAGS) -c $< -o $@

.c.o:
	g++ $(CFLAGS) $(IFLAGS) -c $< -o $@

#############################################
clean:
	$(RM) $(OBJECTS) $(TARGET)
