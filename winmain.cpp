#include <stdio.h>
#include "server.h"
#include "nx/cnmt.h"

bool appletMainLoop()
{
	return true;
}

int main()
{
	Cnmt cnmt;
	cnmt.open(string("C:\\Users\\bwarner\\Desktop\\nba2k19\\890e546667bc9a12c88fc06203c874c2.cnmt.nca"));
	//Server server;

	//server.run();
}
