cmake_minimum_required(VERSION 2.8)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_CROSSCOMPILING TRUE)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(bare-metal-arm LANGUAGES C CXX ASM)

set(CMAKE_C_COMPILER "arm-none-eabi-gcc")
set(CMAKE_CXX_COMPILER "arm-none-eabi-g++")
set(CMAKE_ASM_COMPILER "arm-none-eabi-as")
set(CMAKE_OBJCOPY "arm-none-eabi-objcopy")
set(CMAKE_OBJDUMP "arm-none-eabi-objdump")

file(GLOB LINKSCRIPT "src/device.ld")
file(GLOB CXX_SRC "src/*.cpp")
file(GLOB CXX_SUB_SRC "src/*/*.cpp")
file(GLOB C_SRC "src/*.c")
file(GLOB ASM_SRC "src/*.s")
set(OUTPUT_FILE "bare-metal")

include_directories(include)

set(ARCH "-mcpu=cortex-m0 -mthumb")
set(OPT "-O3 -g -Wall")
set(STACK_USAGE "-fstack-usage -fdump-tree-optimized-graph")
set(EMBED "-ffreestanding -nostartfiles")
set(EMBED_CXX "-fno-exceptions -fno-unwind-tables -fno-rtti -std=c++20")
set(NEWLIB "--specs=nano.specs --specs=nosys.specs")

set(ALL_FLAGS "${ARCH} ${OPT} ${STACK_USAGE}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ALL_FLAGS} ${EMBED}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ALL_FLAGS} ${EMBED} ${EMBED_CXX}")
SET(CMAKE_ASM_FLAGS "${CFLAGS} ${ARCH}")
set(CMAKE_EXE_LINKER_FLAGS "-T ${LINKSCRIPT} -lgcc -lm")

add_executable(bare-metal ${C_SRC} ${CXX_SRC} ${CXX_SUB_SRC} ${ASM_SRC})
set_target_properties(bare-metal PROPERTIES OUTPUT_NAME "${OUTPUT_FILE}.elf")

add_custom_command(
  TARGET bare-metal
  POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O ihex ${OUTPUT_FILE}.elf ${OUTPUT_FILE}.hex
  COMMENT "Converting ELF to hex")

add_custom_command(
  TARGET bare-metal
  POST_BUILD
  COMMAND ${CMAKE_OBJDUMP} -S ${OUTPUT_FILE}.elf > ${OUTPUT_FILE}.S
  COMMENT "Converting ELF to assembly")

add_custom_target(copy COMMENT "copy hex to microbit")
add_custom_command(
  TARGET copy
  POST_BUILD
  COMMAND sh copy_hex.sh
  COMMENT "Copy hex to microbit")

add_custom_target(qemu COMMENT "run qemu")
add_custom_command(
  TARGET qemu
  POST_BUILD
  COMMAND qemu-system-arm -M microbit -device loader,file=${OUTPUT_FILE}.hex
          -serial stdio -monitor telnet::45454,server,nowait -nographic
  COMMENT "Running QEMU...")

add_custom_target(qemu-gdb COMMENT "run qemu with GDB")
add_custom_command(
  TARGET qemu-gdb
  POST_BUILD
  COMMAND sh gdbscript.sh
  COMMENT "Running QEMU with GDB...")

add_custom_target(tty COMMENT "run minicom")
add_custom_command(
  TARGET tty
  POST_BUILD
  COMMAND sudo minicom -D /dev/ttyACM0 -b 9600
  COMMENT "Running minicom...")

