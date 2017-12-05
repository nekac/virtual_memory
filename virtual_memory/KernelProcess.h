#ifndef  _KERNELPROCESS_H_
#define  _KERNELPROCESS_H_
#include "Preocess.h"
#include "KernelSystem.h"

class KernelProcess
{
public:
	KernelProcess(ProcessId);
	~KernelProcess();

	void init(KernelSystem*);

private:
	ProcessId m_pid;
	KernelSystem* m_kernelSystem;
};


#endif


