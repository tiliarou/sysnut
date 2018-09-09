#include <stdio.h>
#include "server.h"
#include "nx/cnmt.h"
#include "nx/crypto.h"
#include "log.h"
#include "nx/integer.h"

bool appletMainLoop()
{
	return true;
}

int main()
{
	/*Nca cnmt;
	cnmt.open(string("C:\\Users\\bwarner\\Desktop\\nba2k19\\890e546667bc9a12c88fc06203c874c2.cnmt.nca"));
	Buffer<u8> data;

	for (const auto& fs : cnmt.directories())
	{
		auto fe = fs->files().first();
		sptr<File>& f = fe->open();
		Cnmt c;
		c.open2(f);
		print("fs\n");
	}*/

	
	Pfs0 nsp;
	if (nsp.open(string("C:\\Users\\bwarner\\Desktop\\switch\\GUNBIRD2 for Nintendo Switch[0100BCB00AE98000][v0].nsp")))
	{
		nsp.install();
	}
	

	return 0;
}
