CXXFLAGS += -std='c++23' -Wall -Wextra -Wno-missing-field-initializers -Wsign-compare -fmax-errors=2 -pedantic -g
# Sur Mac, supprimer la ligne suivante
CXXFLAGS += -Walloc-zero -Wctor-dtor-privacy -Wdeprecated-copy-dtor -Wduplicated-branches -Wduplicated-cond -Wextra-semi -Wfloat-equal -Wformat-signedness -Winit-self -Wlogical-op -Wnon-virtual-dtor -Wnull-dereference -Wold-style-cast -Woverloaded-virtual -Wsign-promo -Wstrict-null-sentinel -Wsuggest-attribute=const -Wsuggest-override -Wswitch-default -Wswitch -Wundef -Wuseless-cast -Wvolatile -Wzero-as-null-pointer-constant -fmax-errors=2 -Wformat=2 -fsanitize=undefined,address,leak
CXX = g++-12

# Compile project

SRC_DIR = src/
BUILD_DIR = build/

CMN_SOURCES = $(wildcard ${SRC_DIR}/common/*.cc)

.PHONY: default
default: client_terminal server

.PHONY: terminal
terminal: client_terminal server

.PHONY: gui
gui: client_gui server

# Compile client only

CLT_DIR = ${SRC_DIR}/client
CLT_SOURCES = $(wildcard ${CLT_DIR}/*.cc)
CLT_OBJECTS = $(patsubst ${SRC_DIR}%.cc,${BUILD_DIR}%.o,${CLT_SOURCES})
CLT_DEPENDS = $(patsubst ${SRC_DIR}%.cc,${BUILD_DIR}%.d,$(CLT_SOURCES))

GUI_DIR = ${SRC_DIR}/client/gui
GUI_SOURCES = $(wildcard ${GUI_DIR}/*.cc)
GUI_OBJECTS = $(patsubst ${SRC_DIR}%.cc,${BUILD_DIR}%.o,${GUI_SOURCES})
GUI_DEPENDS = $(patsubst ${SRC_DIR}%.cc,${BUILD_DIR}%.d,$(GUI_SOURCES))

	
client_terminal: ${CLT_OBJECTS} ${CMN_SOURCES}
	${CXX} ${CXXFLAGS} ${LDFLAGS} $^ -o $@ ${LOADLIBES} ${LDLIBS}

client_gui: CXXFLAGS += -DGUI
client_gui: ${GUI_OBJECTS} ${CLT_OBJECTS} ${CMN_SOURCES}
	${CXX} ${CXXFLAGS} ${LDFLAGS} $^ -o $@ ${LOADLIBES} ${LDLIBS} -lsfml-graphics -lsfml-window -lsfml-system

# Compile server only

SRV_DIR = ${SRC_DIR}/server
SRV_SOURCES = $(wildcard ${SRV_DIR}/*.cc)
SRV_OBJECTS = $(patsubst ${SRC_DIR}%.cc,${BUILD_DIR}%.o,${SRV_SOURCES})
SRV_DEPENDS = $(patsubst ${SRC_DIR}%.cc,${BUILD_DIR}%.d,$(SRV_SOURCES))

server:	${SRV_OBJECTS} ${CMN_SOURCES}
	${CXX} ${CXXFLAGS} ${LDFLAGS} $^ -o $@ ${LOADLIBES} ${LDLIBS}

-include $(CLT_DEPENDS)
-include $(SRV_DEPENDS)
-include $(GUI_DEPENDS)

${BUILD_DIR}%.o: ${SRC_DIR}%.cc Makefile
	@mkdir -p $(dir $@)
	${CXX} ${CXXFLAGS} ${LDFLAGS} -MMD -MP -c $< -o $@ ${LOADLIBES} ${LDLIBS}

# Run
.PHONY: run
run:
	@touch /tmp/battleship-err
	@echo
	@echo "I will now run ``./battleship`` with errors redirected to /tmp/battleship-err"
	@echo "You should use ``tail -f /tmp/battleship-err`` in another terminal to see the error messages"
	@echo "Press Enter to run battleship"
	@echo
	@read line
	./battleship 2> /tmp/battleship-err 

# make clean supprime les fichiers objets et dépendances
.PHONY: clean
clean:
	-rm ${BUILD_DIR}client/*.o
	-rm ${BUILD_DIR}client/*.d
	-rm ${BUILD_DIR}client/gui/*.o
	-rm ${BUILD_DIR}client/gui/*.d
	-rm ${BUILD_DIR}server/*.o
	-rm ${BUILD_DIR}server/*.d

# make mrclean supprime les fichiers objets et les exécutables
.PHONY: mrclean
mrclean: clean
	-rm client_gui client_terminal
