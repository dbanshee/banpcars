#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=Cygwin-Windows
CND_DLIB_EXT=dll
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/BanPCarsServer/banPCarsServer.o \
	${OBJECTDIR}/src/libs/jSon.o \
	${OBJECTDIR}/src/libs/logger.o \
	${OBJECTDIR}/src/libs/pCarsDump.o \
	${OBJECTDIR}/src/libs/pcarsApi.o \
	${OBJECTDIR}/src/libs/pcarsSource.o \
	${OBJECTDIR}/src/libs/restWS.o \
	${OBJECTDIR}/src/libs/serialwin.o \
	${OBJECTDIR}/src/libs/serversocket.o \
	${OBJECTDIR}/src/libs/simController.o \
	${OBJECTDIR}/src/libs/stringutils.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L../ext-sources/libmicrohttpd-0.9.38/libmicrohttpd-0.9.38/src/microhttpd/.libs -L../ext-sources/jansson-2.7/src/.libs -lmicrohttpd -lcygjansson-4

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/banpcars.exe

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/banpcars.exe: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/banpcars ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/src/BanPCarsServer/banPCarsServer.o: src/BanPCarsServer/banPCarsServer.c 
	${MKDIR} -p ${OBJECTDIR}/src/BanPCarsServer
	${RM} "$@.d"
	$(COMPILE.c) -g -I../ext-sources/libmicrohttpd-0.9.38/libmicrohttpd-0.9.38/src/include -I../ext-sources/jansson-2.7/src -include ../ext-sources/jansson-2.7/src/jansson.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/BanPCarsServer/banPCarsServer.o src/BanPCarsServer/banPCarsServer.c

${OBJECTDIR}/src/libs/jSon.o: src/libs/jSon.c 
	${MKDIR} -p ${OBJECTDIR}/src/libs
	${RM} "$@.d"
	$(COMPILE.c) -g -I../ext-sources/libmicrohttpd-0.9.38/libmicrohttpd-0.9.38/src/include -I../ext-sources/jansson-2.7/src -include ../ext-sources/jansson-2.7/src/jansson.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/libs/jSon.o src/libs/jSon.c

${OBJECTDIR}/src/libs/logger.o: src/libs/logger.c 
	${MKDIR} -p ${OBJECTDIR}/src/libs
	${RM} "$@.d"
	$(COMPILE.c) -g -I../ext-sources/libmicrohttpd-0.9.38/libmicrohttpd-0.9.38/src/include -I../ext-sources/jansson-2.7/src -include ../ext-sources/jansson-2.7/src/jansson.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/libs/logger.o src/libs/logger.c

${OBJECTDIR}/src/libs/pCarsDump.o: src/libs/pCarsDump.c 
	${MKDIR} -p ${OBJECTDIR}/src/libs
	${RM} "$@.d"
	$(COMPILE.c) -g -I../ext-sources/libmicrohttpd-0.9.38/libmicrohttpd-0.9.38/src/include -I../ext-sources/jansson-2.7/src -include ../ext-sources/jansson-2.7/src/jansson.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/libs/pCarsDump.o src/libs/pCarsDump.c

${OBJECTDIR}/src/libs/pcarsApi.o: src/libs/pcarsApi.c 
	${MKDIR} -p ${OBJECTDIR}/src/libs
	${RM} "$@.d"
	$(COMPILE.c) -g -I../ext-sources/libmicrohttpd-0.9.38/libmicrohttpd-0.9.38/src/include -I../ext-sources/jansson-2.7/src -include ../ext-sources/jansson-2.7/src/jansson.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/libs/pcarsApi.o src/libs/pcarsApi.c

${OBJECTDIR}/src/libs/pcarsSource.o: src/libs/pcarsSource.c 
	${MKDIR} -p ${OBJECTDIR}/src/libs
	${RM} "$@.d"
	$(COMPILE.c) -g -I../ext-sources/libmicrohttpd-0.9.38/libmicrohttpd-0.9.38/src/include -I../ext-sources/jansson-2.7/src -include ../ext-sources/jansson-2.7/src/jansson.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/libs/pcarsSource.o src/libs/pcarsSource.c

${OBJECTDIR}/src/libs/restWS.o: src/libs/restWS.c 
	${MKDIR} -p ${OBJECTDIR}/src/libs
	${RM} "$@.d"
	$(COMPILE.c) -g -I../ext-sources/libmicrohttpd-0.9.38/libmicrohttpd-0.9.38/src/include -I../ext-sources/jansson-2.7/src -include ../ext-sources/jansson-2.7/src/jansson.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/libs/restWS.o src/libs/restWS.c

${OBJECTDIR}/src/libs/serialwin.o: src/libs/serialwin.c 
	${MKDIR} -p ${OBJECTDIR}/src/libs
	${RM} "$@.d"
	$(COMPILE.c) -g -I../ext-sources/libmicrohttpd-0.9.38/libmicrohttpd-0.9.38/src/include -I../ext-sources/jansson-2.7/src -include ../ext-sources/jansson-2.7/src/jansson.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/libs/serialwin.o src/libs/serialwin.c

${OBJECTDIR}/src/libs/serversocket.o: src/libs/serversocket.c 
	${MKDIR} -p ${OBJECTDIR}/src/libs
	${RM} "$@.d"
	$(COMPILE.c) -g -I../ext-sources/libmicrohttpd-0.9.38/libmicrohttpd-0.9.38/src/include -I../ext-sources/jansson-2.7/src -include ../ext-sources/jansson-2.7/src/jansson.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/libs/serversocket.o src/libs/serversocket.c

${OBJECTDIR}/src/libs/simController.o: src/libs/simController.c 
	${MKDIR} -p ${OBJECTDIR}/src/libs
	${RM} "$@.d"
	$(COMPILE.c) -g -I../ext-sources/libmicrohttpd-0.9.38/libmicrohttpd-0.9.38/src/include -I../ext-sources/jansson-2.7/src -include ../ext-sources/jansson-2.7/src/jansson.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/libs/simController.o src/libs/simController.c

${OBJECTDIR}/src/libs/stringutils.o: src/libs/stringutils.c 
	${MKDIR} -p ${OBJECTDIR}/src/libs
	${RM} "$@.d"
	$(COMPILE.c) -g -I../ext-sources/libmicrohttpd-0.9.38/libmicrohttpd-0.9.38/src/include -I../ext-sources/jansson-2.7/src -include ../ext-sources/jansson-2.7/src/jansson.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/libs/stringutils.o src/libs/stringutils.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/banpcars.exe

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
