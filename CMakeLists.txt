cmake_minimum_required(VERSION 3.28)
project(shipc C)

set(CMAKE_C_STANDARD 11)

include_directories(shipc)

add_executable(shipc
        shipc/chunk.c
        shipc/chunk.h
        shipc/compiler.c
        shipc/compiler.h
        shipc/debug.c
        shipc/debug.h
        shipc/main.c
        shipc/memory.c
        shipc/memory.h
        shipc/objects.c
        shipc/objects.h
        shipc/table.c
        shipc/table.h
        shipc/token.c
        shipc/token.h
        shipc/value.c
        shipc/value.h
        shipc/vm.c
        shipc/vm.h)
