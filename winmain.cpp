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
	Nca cnmt;
	cnmt.open(string("C:\\Users\\bwarner\\Desktop\\nba2k19\\890e546667bc9a12c88fc06203c874c2.cnmt.nca"));
	Buffer<u8> data;


	for (const auto& fs : cnmt)
	{
		auto fe = fs->files().first();
		sptr<File>& f = fe->open();
		Cnmt c;
		c.open2(f);
		//f->rewind();
		//f->read(data);
		//p->rewind();
		//p->read(data);
		//data.dump();
		print("fs\n");
	}
	//File* f = cnmt.open(0);

	//for (HashedContentRecord record : cnmt)
	//print("%x, %d records\n", cnmt.contentMetaHeader()->titleId, cnmt.contentMetaHeader()->contentCount);

	/*for (int i = 0; i < 0x100; i++)
	{
		print("%2.2x ", (u8)cnmt.buffer().c_str()[i]);
	}
	print("\n");*/
	//cnmt.header().dump(0, 0x400);
	//uhx("AEAAB1CA08ADF9BEF12991F369E3C567D6881E4E4A6A47A51F6E4877062D542D").dump();
	return 0;
	/*for(int i=0; i < cnmt.contentMetaHeader()->contentCount; i++)
	{
		HashedContentRecord* record = cnmt.hashedContentRecord(i);
		print("conteType: %d\n", record->record.contentType);
	}*/
	print("fin\n");
	//Server server;

	//server.run();
}
