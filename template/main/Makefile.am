
# set noexpandtab

include ../../buildenv.am

bin_PROGRAMS = template
EXEEXT = d
TARGET = $(bin_PROGRAMS)
template_SOURCES = \
		define.cpp \
		statistics.cpp \
		cli.cpp \
		rp_timer.cpp \
		rp_pool.cpp \
		session.cpp \
		interface_hub.cpp \
		load_generator.cpp \
		property.cpp \
		http1_api.cpp \
		main.cpp
                                                          
template_LDADD = $(COMM_LD) \
		-ltemplate_tum -ltemplate_ifm -ltemplate_hsm \
		-lxihttp1 -lxirp -lxiwebsocket -lxi \
		$(SYSLIBS)

AM_CPPFLAGS = $(COMM_INC)
LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status -Wno-deprecated --recheck $(AM_CPPFLAGS)

all:
	mv -f $(TARGET) $(BIN_DIR)

clean:
	rm -rf $(TARGET) *.o
