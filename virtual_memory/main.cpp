#include "part.h"
#include <iostream>
#include "vm_declarations.h"
#include "System.h"

int main()
{
	Partition* part = new Partition("p1.ini");
	std::cout << part->getNumOfClusters();

	auto pages = new char[10000 * PAGE_SIZE];
	auto pmtSpace = new char[300 * PAGE_SIZE];

	auto system = new System(pages, 10000, pmtSpace, 300, part);


	delete part;
}
