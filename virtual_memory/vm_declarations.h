#pragma once


// File: vm_declarations.h 
typedef unsigned long PageNum;
typedef unsigned long VirtualAddress;
typedef void* PhysicalAddress;
typedef unsigned long Time;
typedef unsigned long AccessRight;
enum Status { OK, PAGE_FAULT, TRAP };

// tipovima pristupa sam dodelio vrednosi koje u okviru koda vracaju
enum AccessType { READ=1, WRITE=2, READ_WRITE=4, EXECUTE=8 };

typedef unsigned ProcessId;
#define PAGE_SIZE 1024 // 1KB je velicina stranice

