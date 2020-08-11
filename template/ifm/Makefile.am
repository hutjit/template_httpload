
# set noexpandtab

include ../../buildenv.am

lib_LIBRARIES = libtemplate_ifm.a
libtemplate_ifm_a_SOURCES = \
		socket_event.cpp \
		timer_event.cpp \
		load_command.cpp \
		http1_protocol.cpp \
		kafka_produce.cpp \
		define.cpp

AM_CPPFLAGS = $(COMM_INC)

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status -Wno-deprecated --recheck $(AM_CPPFLAGS)

all:
	cp -rf lib* $(LIB_DIR)

clean:
	rm -rf *.o *.a $(LIB_DIR)/$(lib_LIBRARIES)
