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
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Debug-GNU-LINUX
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/AlgoidCom/libs/lib_json/jRead.o \
	${OBJECTDIR}/AlgoidCom/libs/lib_json/jWrite.o \
	${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/Clients.o \
	${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/Heap.o \
	${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/LinkedList.o \
	${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/Log.o \
	${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/MQTTClient.o \
	${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/MQTTPacket.o \
	${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/MQTTPacketOut.o \
	${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/MQTTPersistence.o \
	${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/MQTTPersistenceDefault.o \
	${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/MQTTProtocolClient.o \
	${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/MQTTProtocolOut.o \
	${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/Messages.o \
	${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/SSLSocket.o \
	${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/Socket.o \
	${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/SocketBuffer.o \
	${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/StackTrace.o \
	${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/Thread.o \
	${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/Tree.o \
	${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/utf-8.o \
	${OBJECTDIR}/AlgoidCom/linux_JSON.o \
	${OBJECTDIR}/AlgoidCom/messagesManager.o \
	${OBJECTDIR}/AlgoidCom/mqttProtocol.o \
	${OBJECTDIR}/AlgoidCom/udpPublish.o \
	${OBJECTDIR}/algobot_main.o \
	${OBJECTDIR}/asyncTools/asnycLED.o \
	${OBJECTDIR}/asyncTools/asyncMOTOR.o \
	${OBJECTDIR}/asyncTools/asyncPWM.o \
	${OBJECTDIR}/asyncTools/asyncSERVO.o \
	${OBJECTDIR}/asyncTools/asyncTools.o \
	${OBJECTDIR}/fileIO.o \
	${OBJECTDIR}/hwControl/boardHWctrl.o \
	${OBJECTDIR}/hwControl/boardHWsimu.o \
	${OBJECTDIR}/hwControl/hwManager.o \
	${OBJECTDIR}/hwControl/libs/i2c/onion-i2c.o \
	${OBJECTDIR}/hwControl/libs/onion-debug/onion-debug.o \
	${OBJECTDIR}/timerManager.o \
	${OBJECTDIR}/tools.o


# C Compiler Flags
CFLAGS=-pthread -lm

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/algobot_onionomega

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/algobot_onionomega: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/algobot_onionomega ${OBJECTFILES} ${LDLIBSOPTIONS} -pthread -lm

${OBJECTDIR}/AlgoidCom/libs/lib_json/jRead.o: AlgoidCom/libs/lib_json/jRead.c
	${MKDIR} -p ${OBJECTDIR}/AlgoidCom/libs/lib_json
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AlgoidCom/libs/lib_json/jRead.o AlgoidCom/libs/lib_json/jRead.c

${OBJECTDIR}/AlgoidCom/libs/lib_json/jWrite.o: AlgoidCom/libs/lib_json/jWrite.c
	${MKDIR} -p ${OBJECTDIR}/AlgoidCom/libs/lib_json
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AlgoidCom/libs/lib_json/jWrite.o AlgoidCom/libs/lib_json/jWrite.c

${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/Clients.o: AlgoidCom/libs/lib_mqtt/Clients.c
	${MKDIR} -p ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/Clients.o AlgoidCom/libs/lib_mqtt/Clients.c

${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/Heap.o: AlgoidCom/libs/lib_mqtt/Heap.c
	${MKDIR} -p ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/Heap.o AlgoidCom/libs/lib_mqtt/Heap.c

${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/LinkedList.o: AlgoidCom/libs/lib_mqtt/LinkedList.c
	${MKDIR} -p ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/LinkedList.o AlgoidCom/libs/lib_mqtt/LinkedList.c

${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/Log.o: AlgoidCom/libs/lib_mqtt/Log.c
	${MKDIR} -p ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/Log.o AlgoidCom/libs/lib_mqtt/Log.c

${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/MQTTClient.o: AlgoidCom/libs/lib_mqtt/MQTTClient.c
	${MKDIR} -p ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/MQTTClient.o AlgoidCom/libs/lib_mqtt/MQTTClient.c

${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/MQTTPacket.o: AlgoidCom/libs/lib_mqtt/MQTTPacket.c
	${MKDIR} -p ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/MQTTPacket.o AlgoidCom/libs/lib_mqtt/MQTTPacket.c

${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/MQTTPacketOut.o: AlgoidCom/libs/lib_mqtt/MQTTPacketOut.c
	${MKDIR} -p ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/MQTTPacketOut.o AlgoidCom/libs/lib_mqtt/MQTTPacketOut.c

${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/MQTTPersistence.o: AlgoidCom/libs/lib_mqtt/MQTTPersistence.c
	${MKDIR} -p ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/MQTTPersistence.o AlgoidCom/libs/lib_mqtt/MQTTPersistence.c

${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/MQTTPersistenceDefault.o: AlgoidCom/libs/lib_mqtt/MQTTPersistenceDefault.c
	${MKDIR} -p ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/MQTTPersistenceDefault.o AlgoidCom/libs/lib_mqtt/MQTTPersistenceDefault.c

${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/MQTTProtocolClient.o: AlgoidCom/libs/lib_mqtt/MQTTProtocolClient.c
	${MKDIR} -p ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/MQTTProtocolClient.o AlgoidCom/libs/lib_mqtt/MQTTProtocolClient.c

${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/MQTTProtocolOut.o: AlgoidCom/libs/lib_mqtt/MQTTProtocolOut.c
	${MKDIR} -p ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/MQTTProtocolOut.o AlgoidCom/libs/lib_mqtt/MQTTProtocolOut.c

${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/Messages.o: AlgoidCom/libs/lib_mqtt/Messages.c
	${MKDIR} -p ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/Messages.o AlgoidCom/libs/lib_mqtt/Messages.c

${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/SSLSocket.o: AlgoidCom/libs/lib_mqtt/SSLSocket.c
	${MKDIR} -p ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/SSLSocket.o AlgoidCom/libs/lib_mqtt/SSLSocket.c

${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/Socket.o: AlgoidCom/libs/lib_mqtt/Socket.c
	${MKDIR} -p ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/Socket.o AlgoidCom/libs/lib_mqtt/Socket.c

${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/SocketBuffer.o: AlgoidCom/libs/lib_mqtt/SocketBuffer.c
	${MKDIR} -p ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/SocketBuffer.o AlgoidCom/libs/lib_mqtt/SocketBuffer.c

${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/StackTrace.o: AlgoidCom/libs/lib_mqtt/StackTrace.c
	${MKDIR} -p ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/StackTrace.o AlgoidCom/libs/lib_mqtt/StackTrace.c

${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/Thread.o: AlgoidCom/libs/lib_mqtt/Thread.c
	${MKDIR} -p ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/Thread.o AlgoidCom/libs/lib_mqtt/Thread.c

${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/Tree.o: AlgoidCom/libs/lib_mqtt/Tree.c
	${MKDIR} -p ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/Tree.o AlgoidCom/libs/lib_mqtt/Tree.c

${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/utf-8.o: AlgoidCom/libs/lib_mqtt/utf-8.c
	${MKDIR} -p ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AlgoidCom/libs/lib_mqtt/utf-8.o AlgoidCom/libs/lib_mqtt/utf-8.c

${OBJECTDIR}/AlgoidCom/linux_JSON.o: AlgoidCom/linux_JSON.c
	${MKDIR} -p ${OBJECTDIR}/AlgoidCom
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AlgoidCom/linux_JSON.o AlgoidCom/linux_JSON.c

${OBJECTDIR}/AlgoidCom/messagesManager.o: AlgoidCom/messagesManager.c
	${MKDIR} -p ${OBJECTDIR}/AlgoidCom
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AlgoidCom/messagesManager.o AlgoidCom/messagesManager.c

${OBJECTDIR}/AlgoidCom/mqttProtocol.o: AlgoidCom/mqttProtocol.c
	${MKDIR} -p ${OBJECTDIR}/AlgoidCom
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AlgoidCom/mqttProtocol.o AlgoidCom/mqttProtocol.c

${OBJECTDIR}/AlgoidCom/udpPublish.o: AlgoidCom/udpPublish.c
	${MKDIR} -p ${OBJECTDIR}/AlgoidCom
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AlgoidCom/udpPublish.o AlgoidCom/udpPublish.c

${OBJECTDIR}/algobot_main.o: algobot_main.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/algobot_main.o algobot_main.c

${OBJECTDIR}/asyncTools/asnycLED.o: asyncTools/asnycLED.c
	${MKDIR} -p ${OBJECTDIR}/asyncTools
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/asyncTools/asnycLED.o asyncTools/asnycLED.c

${OBJECTDIR}/asyncTools/asyncMOTOR.o: asyncTools/asyncMOTOR.c
	${MKDIR} -p ${OBJECTDIR}/asyncTools
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/asyncTools/asyncMOTOR.o asyncTools/asyncMOTOR.c

${OBJECTDIR}/asyncTools/asyncPWM.o: asyncTools/asyncPWM.c
	${MKDIR} -p ${OBJECTDIR}/asyncTools
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/asyncTools/asyncPWM.o asyncTools/asyncPWM.c

${OBJECTDIR}/asyncTools/asyncSERVO.o: asyncTools/asyncSERVO.c
	${MKDIR} -p ${OBJECTDIR}/asyncTools
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/asyncTools/asyncSERVO.o asyncTools/asyncSERVO.c

${OBJECTDIR}/asyncTools/asyncTools.o: asyncTools/asyncTools.c
	${MKDIR} -p ${OBJECTDIR}/asyncTools
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/asyncTools/asyncTools.o asyncTools/asyncTools.c

${OBJECTDIR}/fileIO.o: fileIO.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/fileIO.o fileIO.c

${OBJECTDIR}/hwControl/boardHWctrl.o: hwControl/boardHWctrl.c
	${MKDIR} -p ${OBJECTDIR}/hwControl
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/hwControl/boardHWctrl.o hwControl/boardHWctrl.c

${OBJECTDIR}/hwControl/boardHWsimu.o: hwControl/boardHWsimu.c
	${MKDIR} -p ${OBJECTDIR}/hwControl
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/hwControl/boardHWsimu.o hwControl/boardHWsimu.c

${OBJECTDIR}/hwControl/hwManager.o: hwControl/hwManager.c
	${MKDIR} -p ${OBJECTDIR}/hwControl
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/hwControl/hwManager.o hwControl/hwManager.c

${OBJECTDIR}/hwControl/libs/i2c/onion-i2c.o: hwControl/libs/i2c/onion-i2c.c
	${MKDIR} -p ${OBJECTDIR}/hwControl/libs/i2c
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/hwControl/libs/i2c/onion-i2c.o hwControl/libs/i2c/onion-i2c.c

${OBJECTDIR}/hwControl/libs/onion-debug/onion-debug.o: hwControl/libs/onion-debug/onion-debug.c
	${MKDIR} -p ${OBJECTDIR}/hwControl/libs/onion-debug
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/hwControl/libs/onion-debug/onion-debug.o hwControl/libs/onion-debug/onion-debug.c

${OBJECTDIR}/timerManager.o: timerManager.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/timerManager.o timerManager.c

${OBJECTDIR}/tools.o: tools.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -DI2CSIMU -IhwControl/libs/i2c -IhwControl/libs/onion-debug -IAlgoidCom/libs/lib_json -IAlgoidCom/libs/lib_mqtt -IAlgoidCom -IhwControl -IasyncTools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tools.o tools.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
