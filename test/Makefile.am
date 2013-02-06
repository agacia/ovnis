# additional include paths necessary to compile the C++ programs
AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src  @AM_CPPFLAGS@

bin_PROGRAMS = ovnisSample ovnisSample_static 

ovnisSample_SOURCES = ./my-constants.h ./applications/FceApplication.cpp ./applications/FceApplication.h  ./simple-traffic-application-main.cc

# the additional libraries needed to link helloWorld
ovnisSample_LDADD =  $(top_builddir)/src/libovnis.la $(AM_LDFLAGS)   

# Create a statically linked version of hell.
ovnisSample_static_SOURCES = ${ovnisSample_SOURCES}
ovnisSample_static_LDADD = $(top_builddir)/src/libovnis.la $(AM_LDFLAGS)
ovnisSample_static_LDFLAGS = -static

