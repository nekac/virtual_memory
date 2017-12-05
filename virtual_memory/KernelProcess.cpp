#include "KernelProcess.h"



KernelProcess::KernelProcess(ProcessId pid){
	m_pid = pid;
}


KernelProcess::~KernelProcess()
{
}

void KernelProcess::init(KernelSystem* kernelSystem){
	m_kernelSystem = kernelSystem;
}

