#include <stdio.h>
#include "server.h"
#include "nx/cnmt.h"
#include "nx/crypto.h"
#include "log.h"
#include "nx/integer.h"
#include "nx/sddirectory.h"
#include "nx/curldirectory.h"

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

	CurlDirectory dir(string("ftp://localhost/switch/nsp/titles/"));

	print("scheme: %s, host: %s\n", dir.dirPath().scheme().c_str(), dir.dirPath().host().c_str());

	/*for (auto& f : dir.files())
	{
		print("file: %s\n", f->name());
	}*/

	
	/*Pfs0 nsp;
	if (nsp.open(string("gunbird.nsp")))
	{
		nsp.install();
	}*/
	

	return 0;
}
