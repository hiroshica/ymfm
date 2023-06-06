####################################################################
TARGET = vgmrender


# 中間バイナリファイルの出力ディレクトリ
OBJROOT   = objs
# ソースディレクトリを元にforeach命令で全cppファイルをリスト化する
SOURCESCPP	= examples/vgmrender/vgmrender.cpp examples/vgmrender/em_inflate.cpp \
	src/ymfm_adpcm.cpp src/ymfm_misc.cpp\
	src/ymfm_opl.cpp src/ymfm_opm.cpp src/ymfm_opn.cpp src/ymfm_opq.cpp src/ymfm_opz.cpp src/ymfm_pcm.cpp src/ymfm_ssg.cpp

SOURCESC	= 
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
