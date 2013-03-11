#Any source file name related to our project is ok here.
AC_INIT([ovnis], [0.1],[yoann@pigne.org])
AC_CONFIG_SRCDIR([test/simple-traffic-application-main.cc])


dnl create an config-header
AC_CONFIG_HEADER(src/config.h)

# check for C++ preprocessor and compiler

OLD_CPPFLAGS=$CPPFLAGS
AC_PROG_CXXCPP
AC_PROG_CXX
AC_PROG_CPP
CPPFLAGS=$OLD_CPPFLAGS

AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
#AC_PROG_RANLIB
AM_PROG_LIBTOOL


# We're using automake, so we init it next. The name of the macro
# starts with 'AM' which means that it is related to automake ('AC'
# is related to autoconf).
# Initing automake means more or less generating the .in file from
# the .am file although it can also be generated at other steps.
AM_INIT_AUTOMAKE

AC_CONFIG_MACRO_DIR([m4])

# use the C++ compiler for the following checks
AC_LANG([C++])


# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([string])
AC_CHECK_HEADERS([iostream])


# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T










AC_CHECK_FUNCS([pow]) 
AC_CHECK_FUNCS([socket])
AC_CHECK_FUNCS([sqrt])
AC_CHECK_HEADERS([limits.h]) 
AC_C_CONST 
AC_FUNC_FORK 
AC_FUNC_MALLOC
AC_HEADER_STDBOOL 
AC_TYPE_INT32_T
AC_TYPE_UINT16_T 
AC_TYPE_UINT32_T 
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T








AM_CPPFLAGS="$CPPFLAGS"

AM_LIBS="$LIBS"

AM_LDFLAGS="$LDFLAGS"

##########################################################################
# check for NS3 
##########################################################################
LIB_NS3="ns3.16-antenna-debug"
#libns3.16-antenna-debug.so            libns3.16-flow-monitor-debug.so           libns3.16-propagation-debug.so
#libns3.16-antenna-test-debug.so       libns3.16-flow-monitor-test-debug.so      libns3.16-propagation-test-debug.so
#libns3.16-aodv-debug.so               libns3.16-internet-debug.so               libns3.16-spectrum-debug.so
#libns3.16-aodv-test-debug.so          libns3.16-internet-test-debug.so          libns3.16-spectrum-test-debug.so
#libns3.16-applications-debug.so       libns3.16-lte-debug.so                    libns3.16-stats-debug.so
#libns3.16-applications-test-debug.so  libns3.16-lte-test-debug.so               libns3.16-stats-test-debug.so
#libns3.16-bridge-debug.so             libns3.16-mesh-debug.so                   #libns3.16-tap-bridge-debug.so
#libns3.16-buildings-debug.so          libns3.16-mesh-test-debug.so              libns3.16-test-debug.so
#libns3.16-buildings-test-debug.so     libns3.16-mobility-debug.so               libns3.16-test-test-debug.so
#libns3.16-config-store-debug.so       libns3.16-mobility-test-debug.so          libns3.16-tools-debug.so
#libns3.16-core-debug.so               libns3.16-mpi-debug.so                    libns3.16-tools-test-debug.so
#libns3.16-core-test-debug.so          libns3.16-netanim-debug.so                libns3.16-topology-read-debug.so
#libns3.16-csma-debug.so               libns3.16-netanim-test-debug.so           libns3.16-topology-read-test-debug.so
#libns3.16-csma-layout-debug.so        libns3.16-network-debug.so                libns3.16-uan-debug.so
#libns3.16-dsdv-debug.so               libns3.16-network-test-debug.so           libns3.16-uan-test-debug.so
#libns3.16-dsdv-test-debug.so          libns3.16-nix-vector-routing-debug.so     libns3.16-virtual-net-device-debug.so
#libns3.16-dsr-debug.so                libns3.16-olsr-debug.so                   libns3.16-wifi-debug.so
#libns3.16-dsr-test-debug.so           libns3.16-olsr-test-debug.so              libns3.16-wifi-test-debug.so
#libns3.16-emu-debug.so                libns3.16-point-to-point-debug.so         libns3.16-wimax-debug.so
#libns3.16-energy-debug.so             libns3.16-point-to-point-layout-debug.so  libns3.16-wimax-test-debug.so
#libns3.16-energy-test-debug.so        libns3.16-point-to-point-test-debug.so


# store current *FLAGS and merge with AM_*FLAGS for compilation and linker check   
OLD_CPPFLAGS="$AM_CPPFLAGS $CPPFLAGS"
LDFLAGS="$AM_LDFLAGS $LDFLAGS"

# ensure the library to check for is covered by the LIBS variable
OLD_LIBS=$LIBS
LIBS="-lns3.16-antenna-debug            -lns3.16-flow-monitor-debug           -lns3.16-propagation-debug -lns3.16-antenna-test-debug       -lns3.16-flow-monitor-test-debug      -lns3.16-propagation-test-debug -lns3.16-aodv-debug               -lns3.16-internet-debug               -lns3.16-spectrum-debug -lns3.16-aodv-test-debug          -lns3.16-internet-test-debug          -lns3.16-spectrum-test-debug -lns3.16-applications-debug       -lns3.16-lte-debug                    -lns3.16-stats-debug -lns3.16-applications-test-debug  -lns3.16-lte-test-debug               -lns3.16-stats-test-debug -lns3.16-bridge-debug             -lns3.16-mesh-debug                 -lns3.16-buildings-debug          -lns3.16-mesh-test-debug              -lns3.16-test-debug -lns3.16-buildings-test-debug     -lns3.16-mobility-debug               -lns3.16-test-test-debug -lns3.16-config-store-debug       -lns3.16-mobility-test-debug          -lns3.16-tools-debug -lns3.16-core-debug               -lns3.16-mpi-debug                    -lns3.16-tools-test-debug -lns3.16-core-test-debug          -lns3.16-netanim-debug                -lns3.16-topology-read-debug -lns3.16-csma-debug               -lns3.16-netanim-test-debug           -lns3.16-topology-read-test-debug -lns3.16-csma-layout-debug        -lns3.16-network-debug                -lns3.16-uan-debug -lns3.16-dsdv-debug               -lns3.16-network-test-debug           -lns3.16-uan-test-debug -lns3.16-dsdv-test-debug          -lns3.16-nix-vector-routing-debug     -lns3.16-virtual-net-device-debug -lns3.16-dsr-debug                -lns3.16-olsr-debug                   -lns3.16-wifi-debug -lns3.16-dsr-test-debug           -lns3.16-olsr-test-debug              -lns3.16-wifi-test-debug                -lns3.16-point-to-point-debug         -lns3.16-wimax-debug -lns3.16-energy-debug             -lns3.16-point-to-point-layout-debug  -lns3.16-wimax-test-debug -lns3.16-energy-test-debug        -lns3.16-point-to-point-test-debug "

AM_LIBS="$LIBS $AM_LIBS"

AC_ARG_WITH([ns3], [AS_HELP_STRING([--with-ns3=DIR],[where NS-3 is installed (libraries and headers in DIR/build/).])])

CPPFLAGS="-I${with_ns3}/src -I${with_ns3}/build "
LDFLAGS="-L${with_ns3}/build "

export CPPFLAGS
export LDFLAGS

AC_CHECK_HEADER([ns3/empty.h], 
   [AM_CPPFLAGS="$CPPFLAGS $AM_CPPFLAGS"], 
   [AC_MSG_ERROR([ns3-includes not found in ${with_ns3}/build nor in ${with_ns3}/src. Did you properly set the --with-ns3=DIR option? See "./configure --help".])])


#AC_CHECK_LIB([$LIB_NS3], [main], [ AM_LDFLAGS="$LDFLAGS $AM_LDFLAGS"],
#      [AC_MSG_ERROR([ns3-lib not found in "${with_ns3}/build". Did you properly set the --with-ns3=DIR option? See "./configure --help".])])
AM_LDFLAGS="$LDFLAGS $AM_LDFLAGS"

LIBS=$OLD_LIBS
CPPFLAGS=$OLD_CPPFLAGS
LDFLAGS=$OLD_LDFLAGS
#export CPPFLAGS
#export LDFLAGS


##########################################################################
# check for SUMO
##########################################################################
LIB_SUMO="tcpip"

# store current *FLAGS and merge with AM_*FLAGS for compilation and linker check   
OLD_CPPFLAGS=$CPPFLAGS;
OLD_LDFLAGS=$LDFLAGS;

# ensure the library to check for is covered by the LIBS variable
OLD_LIBS=$LIBS
LIBS="-l${LIB_SUMO}"
AM_LIBS="$LIBS $AM_LIBS"

AC_ARG_WITH([sumo], [AS_HELP_STRING([--with-sumo=DIR],[where SUMO is installed (the src/ dir of SUMO should be under this path).])])

CPPFLAGS="-I${with_sumo}/src "
LDFLAGS="-L${with_sumo}/src/foreign/tcpip "

export CPPFLAGS
export LDFLAGS

AC_CHECK_HEADER([foreign/tcpip/storage.h], 
   [AM_CPPFLAGS="$CPPFLAGS $AM_CPPFLAGS"], 
   [AC_MSG_ERROR([sumo-includes not found in "${with_sumo}/src". Did you properly set the --with-sumo=DIR option? See "./configure --help".])])


AC_CHECK_LIB([$LIB_SUMO], [main], [ AM_LDFLAGS="$LDFLAGS $AM_LDFLAGS"],
      [AC_MSG_ERROR([sumo-lib not found in "$LDFLAGS". Did you properly set the --with-sumo=DIR option? See "./configure --help".])])

LIBS=$OLD_LIBS
CPPFLAGS=$OLD_CPPFLAGS
LDFLAGS=$OLD_LDFLAGS
#export CPPFLAGS
#export LDFLAGS





##########################################################################
# check for xerces-c
##########################################################################
LIB_XERCES="xerces-c"

# store current *FLAGS and merge with AM_*FLAGS for compilation and linker check   
OLD_CPPFLAGS=$CPPFLAGS;
OLD_LDFLAGS=$LDFLAGS;

# ensure the library to check for is covered by the LIBS variable
OLD_LIBS=$LIBS
LIBS="-l${LIB_XERCES}"
AM_LIBS="$LIBS $AM_LIBS"

AC_ARG_WITH([xerces], [AS_HELP_STRING([--with-xerces=DIR],[where Xerces-C is installed (libraries in DIR/lib, headers in DIR/include).])])

CPPFLAGS="-I${with_xerces}/include "
LDFLAGS="-L${with_xerces}/lib "

export CPPFLAGS
export LDFLAGS




AC_CHECK_HEADER([xercesc/util/XercesVersion.hpp], 
   [AM_CPPFLAGS="$CPPFLAGS $AM_CPPFLAGS"], 
   [AC_MSG_ERROR([xerces-includes not found in "${with_xerces}/src". Did you properly set the --with-xerces=DIR option? See "./configure --help".])])


AC_CHECK_LIB([$LIB_XERCES], [main], [ AM_LDFLAGS="$LDFLAGS $AM_LDFLAGS"],
      [AC_MSG_ERROR([xeces-lib not found in "$LDFLAGS". Did you properly set the --with-xerces=DIR option? See "./configure --help".])])

#LIBS=$OLD_LIBS

#CPPFLAGS=$OLD_CPPFLAGS
#LDFLAGS=$OLD_LDFLAGS
#export CPPFLAGS
#export LDFLAGS





LIBS="$AM_LIBS"

AC_SUBST([AM_CPPFLAGS]) 
AC_SUBST([AM_LDFLAGS]) 
AC_SUBST([AM_LIBS]) 

# files to generate via autotools (.am or .in source files)
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([test/Makefile])
AC_CONFIG_FILES([tests/Makefile])

# generate the final Makefile etc.
AC_OUTPUT

