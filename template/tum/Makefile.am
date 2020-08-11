
# set noexpandtab
# Transaction User Module

include ../../buildenv.am

lib_LIBRARIES = libtemplate_tum.a
libtemplate_tum_a_SOURCES = \
		define.cpp \
		session.cpp \
		fsm.cpp \
		fsm_idle.cpp \
		fsm_api_1.cpp \
		fsm_replay_xdr.cpp \
		manager.cpp

DEFS = $(XIDEBUG)
AM_CPPFLAGS = $(COMM_INC)

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status -Wno-deprecated --recheck $(AM_CPPFLAGS)

all:
	cp -rf lib* $(LIB_DIR)

clean:
	rm -rf *.o *.a $(LIB_DIR)/$(lib_LIBRARIES)
