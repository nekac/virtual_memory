#ifndef  _VM_DECLARATIONS_H_
#define  _VM_DECLARATIONS_H_


// File: vm_declarations.h 
typedef unsigned long PageNum;
typedef unsigned long VirtualAddress;
typedef void* PhysicalAddress;
typedef unsigned long Time;
typedef unsigned long AccessRight;
enum Status { OK, PAGE_FAULT, TRAP, ERROR }; // novi tip statusa geske dodat

// tipovima pristupa sam dodelio vrednosi koje u okviru koda vracaju
enum AccessType { READ=1, WRITE=2, READ_WRITE=4, EXECUTE=8 };

typedef unsigned ProcessId;
#define PAGE_SIZE 1024 // 1KB

#endif
