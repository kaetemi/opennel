
#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/dynloadlib.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include "src/cpptest.h"

#include "nel/misc/path.h"

using namespace std;

// Test suite for CInstanceCounter
class CFoo1
{
public:
	NL_INSTANCE_COUNTER_DECL(CFoo1);
};
NL_INSTANCE_COUNTER_IMPL(CFoo1);

class CFoo2
{
public:
	NL_INSTANCE_COUNTER_DECL(CFoo2);
};

NL_INSTANCE_COUNTER_IMPL(CFoo2);

class CFoo3 : public CFoo2
{
public:
	NL_INSTANCE_COUNTER_DECL(CFoo3);
};

NL_INSTANCE_COUNTER_IMPL(CFoo3);


class CInstanceCounterTS : public Test::Suite
{
public:
	CInstanceCounterTS(const std::string &workingPath)
	{
		TEST_ADD(CInstanceCounterTS::testInstanceCounter)
		TEST_ADD(CInstanceCounterTS::testInstanceCounterOutput)
	}
	
private:
	void testInstanceCounter()
	{
		sint32 n;
		{
			CFoo1	foo1;

			sint32 n = NL_GET_INSTANCE_COUNTER(CFoo1);
			TEST_ASSERT(n == 1);
			n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
			TEST_ASSERT(n == 1);

			NLMISC::CInstanceCounterManager::getInstance().resetDeltaCounter();

			n = NL_GET_INSTANCE_COUNTER(CFoo1);
			TEST_ASSERT(n == 1);
			n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
			TEST_ASSERT(n == 0);
		}
		n = NL_GET_INSTANCE_COUNTER(CFoo1);
		TEST_ASSERT(n == 0);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
		TEST_ASSERT(n == -1);

		{
			CFoo1	foo1;
			CFoo1	other(foo1);

			sint32 n = NL_GET_INSTANCE_COUNTER(CFoo1);
			TEST_ASSERT(n == 2);
			n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
			TEST_ASSERT(n == 1);

			NLMISC::CInstanceCounterManager::getInstance().resetDeltaCounter();

			n = NL_GET_INSTANCE_COUNTER(CFoo1);
			TEST_ASSERT(n == 2);
			n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
			TEST_ASSERT(n == 0);
		}
		n = NL_GET_INSTANCE_COUNTER(CFoo1);
		TEST_ASSERT(n == 0);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
		TEST_ASSERT(n == -2);

		{
			CFoo1	foo1;
			CFoo1	other;

			foo1 = other;

			sint32 n = NL_GET_INSTANCE_COUNTER(CFoo1);
			TEST_ASSERT(n == 2);
			n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
			TEST_ASSERT(n == 0);

			NLMISC::CInstanceCounterManager::getInstance().resetDeltaCounter();

			n = NL_GET_INSTANCE_COUNTER(CFoo1);
			TEST_ASSERT(n == 2);
			n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
			TEST_ASSERT(n == 0);
		}
		n = NL_GET_INSTANCE_COUNTER(CFoo1);
		TEST_ASSERT(n == 0);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
		TEST_ASSERT(n == -2);

		CFoo1	*foo1s[10];
		CFoo2	*foo2s[10];
		CFoo3	*foo3s[10];
		for (uint i=0; i<10; ++i)
		{
			foo1s[i] = new CFoo1;
			foo2s[i] = new CFoo2;
			foo3s[i] = new CFoo3;

		}
		n = NL_GET_INSTANCE_COUNTER(CFoo1);
		TEST_ASSERT(n == 10);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
		TEST_ASSERT(n == 8);
		n = NL_GET_INSTANCE_COUNTER(CFoo2);
		TEST_ASSERT(n == 20);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo2);
		TEST_ASSERT(n == 20);
		n = NL_GET_INSTANCE_COUNTER(CFoo3);
		TEST_ASSERT(n == 10);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo3);
		TEST_ASSERT(n == 10);

		NLMISC::CInstanceCounterManager::getInstance().resetDeltaCounter();
		n = NL_GET_INSTANCE_COUNTER(CFoo1);
		TEST_ASSERT(n == 10);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
		TEST_ASSERT(n == 0);
		n = NL_GET_INSTANCE_COUNTER(CFoo2);
		TEST_ASSERT(n == 20);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo2);
		TEST_ASSERT(n == 0);
		n = NL_GET_INSTANCE_COUNTER(CFoo3);
		TEST_ASSERT(n == 10);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo3);
		TEST_ASSERT(n == 0);

		for (uint i=0; i<5; ++i)
		{
			delete foo1s[i];
			delete foo2s[i];
			delete foo3s[i];
		}
		n = NL_GET_INSTANCE_COUNTER(CFoo1);
		TEST_ASSERT(n == 5);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
		TEST_ASSERT(n == -5);
		n = NL_GET_INSTANCE_COUNTER(CFoo2);
		TEST_ASSERT(n == 10);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo2);
		TEST_ASSERT(n == -10);
		n = NL_GET_INSTANCE_COUNTER(CFoo3);
		TEST_ASSERT(n == 5);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo3);
		TEST_ASSERT(n == -5);
		for (uint i=5; i<10; ++i)
		{
			delete foo1s[i];
			delete foo2s[i];
			delete foo3s[i];
		}
		n = NL_GET_INSTANCE_COUNTER(CFoo1);
		TEST_ASSERT(n == 0);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
		TEST_ASSERT(n == -10);
		n = NL_GET_INSTANCE_COUNTER(CFoo2);
		TEST_ASSERT(n == 0);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo2);
		TEST_ASSERT(n == -20);
		n = NL_GET_INSTANCE_COUNTER(CFoo3);
		TEST_ASSERT(n == 0);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo3);
		TEST_ASSERT(n == -10);
	}

	void testInstanceCounterOutput()
	{
		NLMISC::CInstanceCounterManager::getInstance().resetDeltaCounter();

		CFoo1	*foo1s[10];
		CFoo2	*foo2s[10];
		CFoo3	*foo3s[10];
		for (uint i=0; i<10; ++i)
		{
			foo1s[i] = new CFoo1;
			foo2s[i] = new CFoo2;
			foo3s[i] = new CFoo3;

		}

		string ref = "Listing 3 Instance counters :\n"
					 "  Class 'CFoo1               ', \t        10 instances, \t        10 delta\n"
					 "  Class 'CFoo2               ', \t        20 instances, \t        20 delta\n"
					 "  Class 'CFoo3               ', \t        10 instances, \t        10 delta\n";

		string ret = NLMISC::CInstanceCounterManager::getInstance().displayCounters();

		TEST_ASSERT(ref == ret);
	}

};


// Test suite for CFile behavior
class CFileTS : public Test::Suite
{
public:
	CFileTS(const std::string &workingPath)
	{
		TEST_ADD(CFileTS::copyOneBigFile)
		TEST_ADD(CFileTS::copyDifferentFileSize)
		TEST_ADD(CFileTS::moveOneBigFile)
		TEST_ADD(CFileTS::moveDifferentFileSize)
	}
	
private:
	string	_SrcFile;
	string	_DstFile;

	void setup()
	{
		_SrcFile = "copy_file_src.foo";
		_DstFile = "copy_file_dst.foo";
	}
	
	void tear_down()
	{
	}

	void copyFileSize(uint fileSize)
	{
		// create a source file (using standard c code)
		FILE *fp = fopen(_SrcFile.c_str(), "wb");
		nlverify(fp != NULL);

		for (uint i=0; i<fileSize; ++i)
		{
			uint8 c = uint8(i);
			nlverify(fwrite(&c, 1, 1, fp) == 1);
		}
		fclose(fp);
		fp = NULL;

		NLMISC::CFile::copyFile(_DstFile.c_str(), _SrcFile.c_str(), false);

		// verify the resulting file
		fp = fopen(_DstFile.c_str(), "rb");
		TEST_ASSERT(fp != NULL);
		if (fp)
		{
			for (uint i=0; i<fileSize; ++i)
			{
				uint8 c;
				size_t nbRead = fread(&c, 1,1, fp);
				TEST_ASSERT(nbRead == 1);
				if (nbRead != 1)
					break;
				TEST_ASSERT_MSG(c == uint8(i), "File content changed during copy");
				if (c != uint8(i))
					break;
			}
			fclose(fp);
		}
	}

	
	void copyOneBigFile()
	{
		// check for a big file
		copyFileSize(1024*1024);
	}
		
	void copyDifferentFileSize()
	{
		// check for a series of size
		for (uint i=0; i<10; ++i)
		{
			copyFileSize(i);
		}

		srand(1234);
		for (uint i=0; i<1024; ++i)
		{
			i += rand()%10;
			copyFileSize(i);
		}
	}

	void moveFileSize(size_t fileSize)
	{
		// remove the destination if any
		FILE *fp = fopen(_DstFile.c_str(), "rb");
		if (fp != NULL)
		{
			fclose(fp);
			NLMISC::CFile::deleteFile(_DstFile);
		}

		// create a source file (using standard c code)
		fp = fopen(_SrcFile.c_str(), "wb");
		nlverify(fp != NULL);

		for (uint i=0; i<fileSize; ++i)
		{
			uint8 c = uint8(i);
			nlverify(fwrite(&c, 1, 1, fp) == 1);
		}
		fclose(fp);
		fp = NULL;

		NLMISC::CFile::moveFile(_DstFile.c_str(), _SrcFile.c_str());

		// verify the resulting file
		fp = fopen(_SrcFile.c_str(), "rb");
		TEST_ASSERT_MSG(fp == NULL, "The source file is not removed");
		if (fp)
			fclose(fp);

		fp = fopen(_DstFile.c_str(), "rb");
		TEST_ASSERT(fp != NULL);
		if (fp)
		{
			for (uint i=0; i<fileSize; ++i)
			{
				uint8 c;
				size_t nbRead = fread(&c, 1,1, fp);
				TEST_ASSERT(nbRead == 1);
				if (nbRead != 1)
					break;
				TEST_ASSERT_MSG(c == uint8(i), "File content changed during move");
				if (c != uint8(i))
					break;
			}
			fclose(fp);
		}
	}

	void moveOneBigFile()
	{
		// check for a big file
		moveFileSize(1024*1024);
	}
		
	void moveDifferentFileSize()
	{
		// check for a series of size
		for (uint i=0; i<10; ++i)
		{
			moveFileSize(i);
		}

		srand(1234);
		for (uint i=0; i<1024; ++i)
		{
			i += rand()%10;
			moveFileSize(i);
		}
	}

};


// global test for any misc feature
class CMiscTS : public Test::Suite
{
public:
	CMiscTS(const std::string &workingPath)
	{
		add(auto_ptr<Test::Suite>(new CFileTS(workingPath)));
		add(auto_ptr<Test::Suite>(new CInstanceCounterTS(workingPath)));
	}
	
private:
};

//
//// register the misc test suite
//void registerNelMiscTestSuite(Test::Suite &mainTestSuite)
//{
//	mainTestSuite.add(auto_ptr<Test::Suite>(new CMiscTestSuite));
//}


auto_ptr<Test::Suite> intRegisterTestSuite(const std::string &workingPath)
{
	return static_cast<Test::Suite*>(new CMiscTS(workingPath));
}

NL_LIB_EXPORT_SYMBOL(registerTestSuite, void, intRegisterTestSuite);
