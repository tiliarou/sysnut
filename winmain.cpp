#include <stdio.h>
#include "server.h"
#include "nx/cnmt.h"
#include "nx/crypto.h"
#include "log.h"

bool appletMainLoop()
{
	return true;
}

int main()
{
	Pfs0 nsp;
	nsp.open(string("C:\\Users\\bwarner\\Desktop\\switch\\GUNBIRD2 for Nintendo Switch[0100BCB00AE98000][v0].nsp"));
	
	auto& fs = nsp.files().where([](sptr<FileEntry>& f) -> bool {return f->name().endsWith(string(".xml")); });
	for (auto& f : *fs)
	{
		print("file: %s\n", f->name().c_str());
	}

	return 0;
}
