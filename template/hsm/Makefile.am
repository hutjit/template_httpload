
# set noexpandtab
# Home Subscriber Module

include ../../buildenv.am

lib_LIBRARIES = libtemplate_hsm.a
libtemplate_hsm_a_SOURCES = \
		replay_xdrs.cpp

DEFS = $(XIDEBUG)
AM_CPPFLAGS = $(COMM_INC)

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status -Wno-deprecated --recheck $(AM_CPPFLAGS)

all:
	cp -rf lib* $(LIB_DIR)

clean:
	rm -rf *.o *.a $(LIB_DIR)/$(lib_LIBRARIES)
