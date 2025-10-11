cmake_minimum_required(VERSION 3.5)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_CROSSCOMPILING TRUE)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(bare-metal-arm LANGUAGES C CXX ASM)

set(CMAKE_C_COMPILER "arm-none-eabi-gcc")
set(CMAKE_CXX_COMPILER "arm-none-eabi-g++")
set(CMAKE_ASM_COMPILER "arm-none-eabi-gcc")
set(CMAKE_OBJCOPY "arm-none-eabi-objcopy")
set(CMAKE_OBJDUMP "arm-none-eabi-objdump")

find_package(Python3 COMPONENTS Interpreter)
set(PYTHON_EXECUTABLE "python3")

set(ARCH "-mcpu=cortex-m0 -mthumb")
set(OPT "-Os -g -Wall")
set(STACK_USAGE "-fstack-usage")
set(EMBED "-ffreestanding -nostartfiles")
set(EMBED_CXX "-fno-exceptions -fno-unwind-tables -fno-rtti -std=c++20 -fno-use-cxa-atexit")
set(NEWLIB "--specs=nano.specs --specs=nosys.specs")
set(DEAD_CODE "-ffunction-sections -fdata-sections")

set(ALL_FLAGS "${ARCH} ${OPT} ${STACK_USAGE}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ALL_FLAGS} ${EMBED} ${DEAD_CODE}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ALL_FLAGS} ${EMBED} ${EMBED_CXX} ${DEAD_CODE}")
SET(CMAKE_ASM_FLAGS "${CFLAGS} ${ARCH}")

set(LINKSCRIPT "${CMAKE_SOURCE_DIR}/src/linker.ld")
set(CMAKE_EXE_LINKER_FLAGS "-T ${LINKSCRIPT} -lgcc -lm -Wl,--gc-sections")

file(GLOB_RECURSE CXX_SRC "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE C_SRC "${CMAKE_SOURCE_DIR}/src/*.c")
file(GLOB_RECURSE ASM_SRC "${CMAKE_SOURCE_DIR}/src/*.s")

include_directories(include)
set(KERNEL_SRC ${CXX_SRC} ${CXX_SUB_SRC} ${C_SRC} ${ASM_SRC})


set(TEST_SUITES "" CACHE STRING "List of tests to include in the build")

# Replace spaces with comma or semicolon if needed
separate_arguments(TEST_SUITES)

# Generate a header file with the test names
set(GENERATED_TEST_HEADER "${CMAKE_SOURCE_DIR}/src/core/enabled_test_suites.h")

file(WRITE ${GENERATED_TEST_HEADER} "#pragma once\n\n")
file(APPEND ${GENERATED_TEST_HEADER} "inline const char* test_suites[] = {\n")
foreach(test ${TEST_SUITES})
    file(APPEND ${GENERATED_TEST_HEADER} "    \"${test}\",\n")
endforeach()
file(APPEND ${GENERATED_TEST_HEADER} "    };\n")

 
option(COPY_HEX "Copy" OFF)

function(add_custom_executable target_name target_dir)
  file(GLOB_RECURSE TARGET_SRC "${target_dir}/*.cpp")
  add_executable(${target_name} ${KERNEL_SRC} ${TARGET_SRC})

  add_custom_target(${target_name}_delete ALL
    COMMAND rm -f ${target_name}.elf
  )
  add_dependencies(${target_name} ${target_name}_delete)

  set_target_properties(${target_name} PROPERTIES OUTPUT_NAME "${target_name}.elf")

  add_custom_command(
    TARGET ${target_name}
    PRE_BUILD
    COMMAND cpp -P "${LINKSCRIPT}.in" -o "${LINKSCRIPT}"
  )

  add_custom_command(
    TARGET ${target_name}
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${target_name}.elf ${target_name}.hex
    COMMENT "Converting ELF to hex"
  )

  add_custom_command(
    TARGET ${target_name}
    POST_BUILD
    COMMAND ${CMAKE_OBJDUMP} -SC ${target_name}.elf > ${target_name}.S
    COMMENT "Converting ELF to assembly"
  )

  add_custom_command(
    TARGET ${target_name}
    POST_BUILD
    COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_SOURCE_DIR}/scripts/stack_usage_analysis.py" CMakeFiles/${target_name}.dir ${target_name} 
    COMMENT "Analyzing stack usage"
    VERBATIM
  )

  if (COPY_HEX) 
    add_custom_command(
      TARGET ${target_name}
      POST_BUILD
      COMMAND sh "${CMAKE_SOURCE_DIR}/scripts/copy_hex.sh" ${target_name}
      COMMENT "Copy hex to microbit"
      VERBATIM
    )
  endif (COPY_HEX)
endfunction()

set(TARGETS
  apollo apollo
  tests tests/unit_tests
  overflow tests/recovery/overflow
)
list(LENGTH TARGETS num_targets)
math(EXPR num_targets "${num_targets} / 2 - 1")

foreach(i RANGE 0 ${num_targets})
    math(EXPR index "${i} * 2")
    math(EXPR index_next "${index} + 1")
    list(GET TARGETS ${index} EXEC_NAME)
    list(GET TARGETS ${index_next} EXEC_DIR)
    add_custom_executable(${EXEC_NAME} ${EXEC_DIR})
endforeach()

add_custom_target(qemu COMMENT "run qemu")
add_custom_command(
  TARGET qemu
  POST_BUILD
  COMMAND qemu-system-arm -M microbit -device loader,file=apollo.hex
          -serial stdio -monitor telnet::45454,server,nowait -nographic
  COMMENT "Running QEMU...")

add_custom_target(qemu-gdb COMMENT "run qemu with GDB")
add_custom_command(
  TARGET qemu-gdb
  POST_BUILD
  COMMAND sh scripts/gdbscript.sh
  COMMENT "Running QEMU with GDB...")

add_custom_target(tty COMMENT "run minicom")
add_custom_command(
  TARGET tty
  POST_BUILD
  COMMAND minicom -D /dev/ttyACM0 -b 9600
  COMMENT "Running minicom...")
