/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 8 (PPLIB).
 * Web: https://github.com/pfedick/pplib
 *******************************************************************************
 * Copyright (c) 2026, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#include <gtest/gtest.h>
#include <pplib/core/pfpfile.h>
#include <pplib/core/file.h>
#include <pplib/core/memfile.h>
#include <pplib/core/functions.h>
#include <pplib/exceptions.h>
#include "pplib-tests.h"

namespace
{

// ============================================================================
// PFPChunk Tests
// ============================================================================

class PFPChunkTest : public ::testing::Test
{
protected:
    PFPChunkTest()
    {
    }
    ~PFPChunkTest() override
    {
    }
};

TEST_F(PFPChunkTest, DefaultConstructor)
{
    pplib::PFPChunk chunk;
    EXPECT_EQ(pplib::String("UNKN"), chunk.name());
    EXPECT_EQ(0u, chunk.size());
    EXPECT_EQ(nullptr, chunk.data());
}

TEST_F(PFPChunkTest, ConstructorWithNameAndData)
{
    const char payload[] = "Hello Chunk Data";
    pplib::ByteArray ba(payload, sizeof(payload));
    pplib::PFPChunk chunk("test", ba);

    EXPECT_EQ(pplib::String("TEST"), chunk.name());
    EXPECT_EQ(sizeof(payload), chunk.size());
    ASSERT_NE(nullptr, chunk.data());
    EXPECT_NE((const void*)payload, chunk.data()); // setData copies data
    EXPECT_EQ(0, memcmp(payload, chunk.data(), sizeof(payload)));
}

TEST_F(PFPChunkTest, CopyConstructorOwnedMemory)
{
    const char data[] = "MySampleData";
    pplib::PFPChunk c1;
    c1.setName("DATA");
    c1.setData(data, sizeof(data));

    pplib::PFPChunk c2(c1);
    EXPECT_EQ(pplib::String("DATA"), c2.name());
    EXPECT_EQ(sizeof(data), c2.size());
    ASSERT_NE(nullptr, c2.data());
    EXPECT_NE(c1.data(), c2.data()); // deep copy of owned buffer
    EXPECT_EQ(0, memcmp(c1.data(), c2.data(), sizeof(data)));
}

TEST_F(PFPChunkTest, CopyConstructorUnownedMemory)
{
    const char data[] = "ExternalStaticData";
    pplib::PFPChunk c1;
    c1.setName("EXTD");
    c1.useData(data, sizeof(data));

    pplib::PFPChunk c2(c1);
    EXPECT_EQ(pplib::String("EXTD"), c2.name());
    EXPECT_EQ(sizeof(data), c2.size());
    EXPECT_EQ((const void*)data, c2.data()); // shallow pointer copy
}

TEST_F(PFPChunkTest, CopyConstructorEmptyData)
{
    pplib::PFPChunk c1;
    c1.setName("EMPT");

    pplib::PFPChunk c2(c1);
    EXPECT_EQ(pplib::String("EMPT"), c2.name());
    EXPECT_EQ(0u, c2.size());
}

TEST_F(PFPChunkTest, CopyAssignment)
{
    const char data1[] = "Chunk One Buffer";
    const char data2[] = "Chunk Two Much Longer Buffer";

    pplib::PFPChunk c1;
    c1.setName("CHK1");
    c1.setData(data1, sizeof(data1));

    pplib::PFPChunk c2;
    c2.setName("CHK2");
    c2.setData(data2, sizeof(data2));

    c2 = c1;
    EXPECT_EQ(pplib::String("CHK1"), c2.name());
    EXPECT_EQ(sizeof(data1), c2.size());
    ASSERT_NE(nullptr, c2.data());
    EXPECT_NE(c1.data(), c2.data());
    EXPECT_EQ(0, memcmp(c1.data(), c2.data(), sizeof(data1)));

    // Self-assignment
    c2 = c2;
    EXPECT_EQ(pplib::String("CHK1"), c2.name());
    EXPECT_EQ(sizeof(data1), c2.size());
}

TEST_F(PFPChunkTest, CopyAssignmentUnowned)
{
    const char data[] = "UnownedMemory";
    pplib::PFPChunk c1;
    c1.setName("UNOW");
    c1.useData(data, sizeof(data));

    pplib::PFPChunk c2;
    c2.setName("TEMP");
    c2 = c1;
    EXPECT_EQ(pplib::String("UNOW"), c2.name());
    EXPECT_EQ((const void*)data, c2.data());
}

TEST_F(PFPChunkTest, MoveConstructor)
{
    const char data[] = "MoveMeData";
    pplib::PFPChunk c1;
    c1.setName("MOVE");
    c1.setData(data, sizeof(data));
    const void* originalPtr = c1.data();

    pplib::PFPChunk c2(std::move(c1));
    EXPECT_EQ(pplib::String("MOVE"), c2.name());
    EXPECT_EQ(sizeof(data), c2.size());
    EXPECT_EQ(originalPtr, c2.data());

    // c1 should be reset
    EXPECT_EQ(pplib::String("UNKN"), c1.name());
    EXPECT_EQ(0u, c1.size());
    EXPECT_EQ(nullptr, c1.data());
}

TEST_F(PFPChunkTest, MoveAssignment)
{
    const char data1[] = "SourceData";
    const char data2[] = "TargetDataWillBeOverwritten";

    pplib::PFPChunk c1;
    c1.setName("SRC1");
    c1.setData(data1, sizeof(data1));
    const void* originalPtr = c1.data();

    pplib::PFPChunk c2;
    c2.setName("DST1");
    c2.setData(data2, sizeof(data2));

    c2 = std::move(c1);
    EXPECT_EQ(pplib::String("SRC1"), c2.name());
    EXPECT_EQ(sizeof(data1), c2.size());
    EXPECT_EQ(originalPtr, c2.data());

    EXPECT_EQ(pplib::String("UNKN"), c1.name());
    EXPECT_EQ(0u, c1.size());
    EXPECT_EQ(nullptr, c1.data());

    // Self-move assignment
    c2 = std::move(c2);
    EXPECT_EQ(pplib::String("SRC1"), c2.name());
}

TEST_F(PFPChunkTest, SetNameValid)
{
    pplib::PFPChunk chunk;
    chunk.setName("ABCD");
    EXPECT_EQ(pplib::String("ABCD"), chunk.name());

    // Lowercase to uppercase conversion
    chunk.setName("efgh");
    EXPECT_EQ(pplib::String("EFGH"), chunk.name());

    chunk.setName("x1_Z");
    EXPECT_EQ(pplib::String("X1_Z"), chunk.name());
}

TEST_F(PFPChunkTest, SetNameInvalid)
{
    pplib::PFPChunk chunk;
    // Length != 4
    EXPECT_THROW(chunk.setName(""), pplib::IllegalArgumentException);
    EXPECT_THROW(chunk.setName("A"), pplib::IllegalArgumentException);
    EXPECT_THROW(chunk.setName("ABC"), pplib::IllegalArgumentException);
    EXPECT_THROW(chunk.setName("ABCDE"), pplib::IllegalArgumentException);

    // Null pointer
    EXPECT_THROW(chunk.setName(nullptr, 4), pplib::IllegalArgumentException);
    EXPECT_THROW(chunk.setName("ABCD", 3), pplib::IllegalArgumentException);
    EXPECT_THROW(chunk.setName("ABCD", 5), pplib::IllegalArgumentException);

    // Non-ASCII or control characters
    char bad1[4] = {'A', 'B', 'C', 10}; // Line feed (< 32)
    EXPECT_THROW(chunk.setName(bad1, 4), pplib::IllegalArgumentException);

    char bad2[4] = {'A', 'B', 'C', (char)128}; // > 127
    EXPECT_THROW(chunk.setName(bad2, 4), pplib::IllegalArgumentException);
}

TEST_F(PFPChunkTest, SetData)
{
    pplib::PFPChunk chunk;
    const char sample[] = "SomeSampleContent";
    chunk.setData(sample, sizeof(sample));
    EXPECT_EQ(sizeof(sample), chunk.size());
    EXPECT_EQ(0, memcmp(sample, chunk.data(), sizeof(sample)));

    // Clear data with nullptr, 0
    chunk.setData(nullptr, 0);
    EXPECT_EQ(0u, chunk.size());
    EXPECT_EQ(nullptr, chunk.data());

    // Clear data with non-null pointer, size 0
    chunk.setData("abc", 0);
    EXPECT_EQ(0u, chunk.size());
    EXPECT_EQ(nullptr, chunk.data());

    // ByteArrayPtr overload
    pplib::ByteArray ba("ByteArrayContent", 16);
    chunk.setData(ba);
    EXPECT_EQ(16u, chunk.size());
    EXPECT_EQ(0, memcmp("ByteArrayContent", chunk.data(), 16));

    // Invalid arguments
    EXPECT_THROW(chunk.setData(nullptr, 10), pplib::IllegalArgumentException);
}

TEST_F(PFPChunkTest, UseData)
{
    pplib::PFPChunk chunk;
    const char external[] = "DirectMemoryPointer";

    // First set owned data
    chunk.setData("OldOwnedData", 12);

    // Now use unowned data -> must free previous owned memory without leaking
    chunk.useData(external, sizeof(external));
    EXPECT_EQ(sizeof(external), chunk.size());
    EXPECT_EQ((const void*)external, chunk.data());

    // ByteArrayPtr overload
    pplib::ByteArray ba("AnotherBuffer", 13);
    chunk.useData(ba);
    EXPECT_EQ(13u, chunk.size());
    EXPECT_EQ((const void*)ba.ptr(), chunk.data());
}

// ============================================================================
// PFPFile Tests
// ============================================================================

class PFPFileTest : public ::testing::Test
{
protected:
    PFPFileTest()
    {
    }
    ~PFPFileTest() override
    {
    }

    void TearDown() override
    {
        pplib::File::unlink("tmp/test_uncompressed.pfp");
        pplib::File::unlink("tmp/test_zlib.pfp");
        pplib::File::unlink("tmp/test_bzip2.pfp");
        pplib::File::unlink("tmp/test_empty.pfp");
        pplib::File::unlink("tmp/test_dup.pfp");
    }
};

TEST_F(PFPFileTest, DefaultState)
{
    pplib::PFPFile file;
    EXPECT_EQ(pplib::String("UNKN"), file.getID());
    EXPECT_EQ(0, file.getMainVersion());
    EXPECT_EQ(0, file.getSubVersion());
    EXPECT_EQ(pplib::Compression::Algo_NONE, file.getCompression());
    EXPECT_EQ(pplib::String(), file.getName());
    EXPECT_EQ(pplib::String(), file.getAuthor());
    EXPECT_EQ(pplib::String(), file.getDescription());
    EXPECT_EQ(pplib::String(), file.getCopyright());
    EXPECT_EQ(file.begin(), file.end());
}

TEST_F(PFPFileTest, SetIdValid)
{
    pplib::PFPFile file;
    file.setId("TEST");
    EXPECT_EQ(pplib::String("TEST"), file.getID());

    file.setId("FONT");
    EXPECT_EQ(pplib::String("FONT"), file.getID());
}

TEST_F(PFPFileTest, SetIdInvalid)
{
    pplib::PFPFile file;
    EXPECT_THROW(file.setId(""), pplib::IllegalArgumentException);
    EXPECT_THROW(file.setId("ABC"), pplib::IllegalArgumentException);
    EXPECT_THROW(file.setId("ABCDE"), pplib::IllegalArgumentException);

    // Invalid ASCII
    pplib::String badChar("AB\x01\x43");
    EXPECT_THROW(file.setId(badChar), pplib::IllegalArgumentException);
}

TEST_F(PFPFileTest, SetVersionAndGetVersion)
{
    pplib::PFPFile file;
    file.setVersion(3, 14);

    EXPECT_EQ(3, file.getMainVersion());
    EXPECT_EQ(14, file.getSubVersion());

    int main = 0, sub = 0;
    file.getVersion(&main, &sub);
    EXPECT_EQ(3, main);
    EXPECT_EQ(14, sub);

    int mainRef = 0, subRef = 0;
    file.getVersion(mainRef, subRef);
    EXPECT_EQ(3, mainRef);
    EXPECT_EQ(14, subRef);

    // Boundary values
    file.setVersion(0, 255);
    EXPECT_EQ(0, file.getMainVersion());
    EXPECT_EQ(255, file.getSubVersion());

    // Invalid ranges
    EXPECT_THROW(file.setVersion(-1, 0), pplib::IllegalArgumentException);
    EXPECT_THROW(file.setVersion(256, 0), pplib::IllegalArgumentException);
    EXPECT_THROW(file.setVersion(0, -1), pplib::IllegalArgumentException);
    EXPECT_THROW(file.setVersion(0, 256), pplib::IllegalArgumentException);
}

TEST_F(PFPFileTest, SetCompressionAndGetCompression)
{
    pplib::PFPFile file;

    file.setCompression(pplib::Compression::Algo_NONE);
    EXPECT_EQ(pplib::Compression::Algo_NONE, file.getCompression());

    file.setCompression(pplib::Compression::Algo_ZLIB);
    EXPECT_EQ(pplib::Compression::Algo_ZLIB, file.getCompression());

    file.setCompression(pplib::Compression::Algo_BZIP2);
    EXPECT_EQ(pplib::Compression::Algo_BZIP2, file.getCompression());

    // Invalid compression algorithm
    EXPECT_THROW(file.setCompression((pplib::Compression::Algorithm)99), pplib::UnknownCompressionMethodException);
}

TEST_F(PFPFileTest, PredefinedChunks)
{
    pplib::PFPFile file;

    file.setName("MyTestFile");
    file.setAuthor("Patrick Fedick");
    file.setDescription("A unit test description");
    file.setCopyright("(c) 2026 PPL");

    EXPECT_EQ(pplib::String("MyTestFile"), file.getName());
    EXPECT_EQ(pplib::String("Patrick Fedick"), file.getAuthor());
    EXPECT_EQ(pplib::String("A unit test description"), file.getDescription());
    EXPECT_EQ(pplib::String("(c) 2026 PPL"), file.getCopyright());

    // Overwriting updates without duplicate chunks
    file.setName("UpdatedName");
    EXPECT_EQ(pplib::String("UpdatedName"), file.getName());

    // Count how many "NAME" chunks exist
    size_t nameChunkCount = 0;
    for (const auto& chunk : file) {
        if (chunk.name() == "NAME") nameChunkCount++;
    }
    EXPECT_EQ(1u, nameChunkCount);

    // Test trailing null stripping in predefined chunks
    pplib::PFPFile fileWithNulls;
    const char nameWithNull[] = "FontName\0";
    const char authorWithNull[] = "AuthorName\0";
    const char descWithNull[] = "Description\0";
    const char copyWithNull[] = "Copyright\0";
    fileWithNulls.addChunk(pplib::PFPChunk("NAME", pplib::ByteArrayPtr(nameWithNull, sizeof(nameWithNull))));
    fileWithNulls.addChunk(pplib::PFPChunk("AUTH", pplib::ByteArrayPtr(authorWithNull, sizeof(authorWithNull))));
    fileWithNulls.addChunk(pplib::PFPChunk("DESC", pplib::ByteArrayPtr(descWithNull, sizeof(descWithNull))));
    fileWithNulls.addChunk(pplib::PFPChunk("COPY", pplib::ByteArrayPtr(copyWithNull, sizeof(copyWithNull))));

    EXPECT_EQ(pplib::String("FontName"), fileWithNulls.getName());
    EXPECT_EQ(pplib::String("AuthorName"), fileWithNulls.getAuthor());
    EXPECT_EQ(pplib::String("Description"), fileWithNulls.getDescription());
    EXPECT_EQ(pplib::String("Copyright"), fileWithNulls.getCopyright());
}

TEST_F(PFPFileTest, AddChunkLvalueAndRvalue)
{
    pplib::PFPFile file;

    pplib::PFPChunk c1("DAT1", pplib::ByteArray("DataOne", 7));
    pplib::PFPChunk& ref1 = file.addChunk(c1);
    EXPECT_EQ(pplib::String("DAT1"), ref1.name());

    pplib::PFPChunk c2("DAT2", pplib::ByteArray("DataTwo", 7));
    pplib::PFPChunk& ref2 = file.addChunk(std::move(c2));
    EXPECT_EQ(pplib::String("DAT2"), ref2.name());

    // Cannot add chunk with name "UNKN"
    pplib::PFPChunk badChunk;
    EXPECT_THROW(file.addChunk(badChunk), pplib::IllegalArgumentException);
    EXPECT_THROW(file.addChunk(std::move(badChunk)), pplib::IllegalArgumentException);
}

TEST_F(PFPFileTest, DeleteChunkByPointer)
{
    pplib::PFPFile file;
    pplib::PFPChunk c1("CHK1", pplib::ByteArray("1", 1));
    pplib::PFPChunk c2("CHK2", pplib::ByteArray("2", 1));
    pplib::PFPChunk c3("CHK3", pplib::ByteArray("3", 1));

    file.addChunk(c1);
    file.addChunk(c2);
    file.addChunk(c3);

    pplib::PFPFile::Iterator it;
    pplib::PFPChunk* p2 = file.findFirstChunk(it, "CHK2");
    ASSERT_NE(nullptr, p2);

    file.deleteChunk(p2);

    EXPECT_EQ(nullptr, file.findFirstChunk(it, "CHK2"));
    EXPECT_NE(nullptr, file.findFirstChunk(it, "CHK1"));
    EXPECT_NE(nullptr, file.findFirstChunk(it, "CHK3"));

    // Deleting nullptr or chunk not in list does not crash
    file.deleteChunk(nullptr);
    pplib::PFPChunk external("EXTR", pplib::ByteArray("X", 1));
    file.deleteChunk(&external);
}

TEST_F(PFPFileTest, DeleteChunkByName)
{
    pplib::PFPFile file;
    file.addChunk(pplib::PFPChunk("MULT", pplib::ByteArray("1", 1)));
    file.addChunk(pplib::PFPChunk("KEEP", pplib::ByteArray("K", 1)));
    file.addChunk(pplib::PFPChunk("MULT", pplib::ByteArray("2", 1)));

    // Case-insensitive deletion
    file.deleteChunk("mult");

    pplib::PFPFile::Iterator it;
    EXPECT_EQ(nullptr, file.findFirstChunk(it, "MULT"));
    EXPECT_NE(nullptr, file.findFirstChunk(it, "KEEP"));

    // Invalid length does nothing
    file.deleteChunk("TOOLONG");
    file.deleteChunk("SH");
}

TEST_F(PFPFileTest, Clear)
{
    pplib::PFPFile file;
    file.setId("TEST");
    file.setVersion(2, 5);
    file.setCompression(pplib::Compression::Algo_ZLIB);
    file.setName("Something");
    file.addChunk(pplib::PFPChunk("DATA", pplib::ByteArray("abc", 3)));

    file.clear();

    EXPECT_EQ(pplib::String("UNKN"), file.getID());
    EXPECT_EQ(0, file.getMainVersion());
    EXPECT_EQ(0, file.getSubVersion());
    EXPECT_EQ(pplib::Compression::Algo_NONE, file.getCompression());
    EXPECT_EQ(pplib::String(), file.getName());
    EXPECT_EQ(file.begin(), file.end());
}

TEST_F(PFPFileTest, IteratorAndRangeFor)
{
    pplib::PFPFile file;
    pplib::PFPFile::Iterator it;

    // Empty file
    EXPECT_EQ(nullptr, file.getFirst(it));
    EXPECT_EQ(nullptr, file.getNext(it));

    file.addChunk(pplib::PFPChunk("AAA1", pplib::ByteArray("1", 1)));
    file.addChunk(pplib::PFPChunk("AAA2", pplib::ByteArray("2", 1)));
    file.addChunk(pplib::PFPChunk("AAA3", pplib::ByteArray("3", 1)));

    // Using getFirst and getNext
    pplib::PFPChunk* c = file.getFirst(it);
    ASSERT_NE(nullptr, c);
    EXPECT_EQ(pplib::String("AAA1"), c->name());

    c = file.getNext(it);
    ASSERT_NE(nullptr, c);
    EXPECT_EQ(pplib::String("AAA2"), c->name());

    c = file.getNext(it);
    ASSERT_NE(nullptr, c);
    EXPECT_EQ(pplib::String("AAA3"), c->name());

    c = file.getNext(it);
    EXPECT_EQ(nullptr, c);

    // Calling getNext on finished iterator stays nullptr
    EXPECT_EQ(nullptr, file.getNext(it));

    // Reset restarts
    file.reset(it);
    c = file.getFirst(it);
    ASSERT_NE(nullptr, c);
    EXPECT_EQ(pplib::String("AAA1"), c->name());

    // Range-for
    size_t count = 0;
    for (const auto& chunk : file) {
        count++;
        EXPECT_TRUE(chunk.name().startsWith("AAA"));
    }
    EXPECT_EQ(3u, count);
}

TEST_F(PFPFileTest, FindFirstAndNextChunk)
{
    pplib::PFPFile file;
    file.addChunk(pplib::PFPChunk("ITEM", pplib::ByteArray("Item 1", 6)));
    file.addChunk(pplib::PFPChunk("OTHR", pplib::ByteArray("Other", 5)));
    file.addChunk(pplib::PFPChunk("ITEM", pplib::ByteArray("Item 2", 6)));

    pplib::PFPFile::Iterator it;

    // Case-insensitive findFirstChunk
    pplib::PFPChunk* c1 = file.findFirstChunk(it, "item");
    ASSERT_NE(nullptr, c1);
    EXPECT_EQ(0, memcmp("Item 1", c1->data(), 6));

    pplib::PFPChunk* c2 = file.findNextChunk(it, "ITEM");
    ASSERT_NE(nullptr, c2);
    EXPECT_EQ(0, memcmp("Item 2", c2->data(), 6));

    pplib::PFPChunk* c3 = file.findNextChunk(it, "ITEM");
    EXPECT_EQ(nullptr, c3);

    // findNextChunk with empty chunkname continues searching for previous chunk
    pplib::PFPFile::Iterator it2;
    pplib::PFPChunk* n1 = file.findFirstChunk(it2, "ITEM");
    ASSERT_NE(nullptr, n1);
    pplib::PFPChunk* n2 = file.findNextChunk(it2, "");
    ASSERT_NE(nullptr, n2);
    EXPECT_EQ(0, memcmp("Item 2", n2->data(), 6));

    // Non-existent chunk
    EXPECT_EQ(nullptr, file.findFirstChunk(it, "NONE"));

    // Invalid name length throws
    EXPECT_THROW(file.findFirstChunk(it, "BAD"), pplib::IllegalArgumentException);
    EXPECT_THROW(file.findFirstChunk(it, "TOOLONG"), pplib::IllegalArgumentException);
}

TEST_F(PFPFileTest, SaveAndLoadUncompressed)
{
    const pplib::String filename("tmp/test_uncompressed.pfp");
    {
        pplib::PFPFile file;
        file.setId("DEMO");
        file.setVersion(1, 2);
        file.setCompression(pplib::Compression::Algo_NONE);
        file.setName("Demo File");
        file.setAuthor("Patrick");
        file.setDescription("A test file without compression");
        file.setCopyright("PPLIB (c) 2026");

        const char customData[] = "ArbitraryBinaryPayload\x00\x01\x02\xff";
        file.addChunk(pplib::PFPChunk("CUST", pplib::ByteArrayPtr(customData, sizeof(customData))));

        ASSERT_NO_THROW(file.save(filename));
    }

    ASSERT_TRUE(pplib::File::exists(filename));

    // Test ident methods
    {
        pplib::PFPFile identFile;
        EXPECT_TRUE(identFile.ident(filename));
        EXPECT_EQ(pplib::String("DEMO"), identFile.getID());
        EXPECT_EQ(1, identFile.getMainVersion());
        EXPECT_EQ(2, identFile.getSubVersion());
        EXPECT_EQ(pplib::Compression::Algo_NONE, identFile.getCompression());

        pplib::File ff(filename, pplib::File::FileMode::READ);
        EXPECT_TRUE(identFile.ident(ff));

        pplib::ByteArray fileData;
        ff.load(fileData);
        EXPECT_TRUE(identFile.ident(fileData));
    }

    // Test load(filename)
    {
        pplib::PFPFile file;
        ASSERT_NO_THROW(file.load(filename));

        EXPECT_EQ(pplib::String("DEMO"), file.getID());
        EXPECT_EQ(1, file.getMainVersion());
        EXPECT_EQ(2, file.getSubVersion());
        EXPECT_EQ(pplib::Compression::Algo_NONE, file.getCompression());
        EXPECT_EQ(pplib::String("Demo File"), file.getName());
        EXPECT_EQ(pplib::String("Patrick"), file.getAuthor());
        EXPECT_EQ(pplib::String("A test file without compression"), file.getDescription());
        EXPECT_EQ(pplib::String("PPLIB (c) 2026"), file.getCopyright());

        pplib::PFPFile::Iterator it;
        pplib::PFPChunk* cust = file.findFirstChunk(it, "CUST");
        ASSERT_NE(nullptr, cust);
        const char expectedPayload[] = "ArbitraryBinaryPayload\x00\x01\x02\xff";
        EXPECT_EQ(sizeof(expectedPayload), cust->size());
        EXPECT_EQ(0, memcmp(expectedPayload, cust->data(), sizeof(expectedPayload)));
    }

    // Test load(FileObject&)
    {
        pplib::PFPFile file;
        pplib::File ff(filename, pplib::File::FileMode::READ);
        ASSERT_NO_THROW(file.load(ff));
        EXPECT_EQ(pplib::String("Demo File"), file.getName());
    }
}

TEST_F(PFPFileTest, SaveAndLoadZlib)
{
    const pplib::String filename("tmp/test_zlib.pfp");
    {
        pplib::PFPFile file;
        file.setId("ZLIB");
        file.setVersion(3, 0);
        file.setCompression(pplib::Compression::Algo_ZLIB);
        file.setName("Compressed Zlib");
        file.setAuthor("Author Z");

        // Repeat data to test compression efficiency
        pplib::String largeText;
        for (int i = 0; i < 50; ++i) {
            largeText += "Line of compressible text for PFPFile test.\n";
        }
        file.addChunk(pplib::PFPChunk("TEXT", pplib::ByteArrayPtr(largeText.c_str(), largeText.size())));

        ASSERT_NO_THROW(file.save(filename));
    }

    ASSERT_TRUE(pplib::File::exists(filename));

    pplib::PFPFile file;
    ASSERT_NO_THROW(file.load(filename));
    EXPECT_EQ(pplib::String("ZLIB"), file.getID());
    EXPECT_EQ(3, file.getMainVersion());
    EXPECT_EQ(0, file.getSubVersion());
    EXPECT_EQ(pplib::Compression::Algo_ZLIB, file.getCompression());
    EXPECT_EQ(pplib::String("Compressed Zlib"), file.getName());
    EXPECT_EQ(pplib::String("Author Z"), file.getAuthor());

    pplib::PFPFile::Iterator it;
    pplib::PFPChunk* chunk = file.findFirstChunk(it, "TEXT");
    ASSERT_NE(nullptr, chunk);
    EXPECT_GT(chunk->size(), 1000u);
}

TEST_F(PFPFileTest, SaveAndLoadBzip2)
{
    const pplib::String filename("tmp/test_bzip2.pfp");
    {
        pplib::PFPFile file;
        file.setId("BZ2_");
        file.setVersion(4, 1);
        file.setCompression(pplib::Compression::Algo_BZIP2);
        file.setName("Compressed Bzip2");
        file.setDescription("Testing Bzip2 compression in PFPFile");

        pplib::String largeText;
        for (int i = 0; i < 50; ++i) {
            largeText += "Bzip2 compression test line repeating content.\n";
        }
        file.addChunk(pplib::PFPChunk("DATA", pplib::ByteArrayPtr(largeText.c_str(), largeText.size())));

        ASSERT_NO_THROW(file.save(filename));
    }

    ASSERT_TRUE(pplib::File::exists(filename));

    pplib::PFPFile file;
    ASSERT_NO_THROW(file.load(filename));
    EXPECT_EQ(pplib::String("BZ2_"), file.getID());
    EXPECT_EQ(4, file.getMainVersion());
    EXPECT_EQ(1, file.getSubVersion());
    EXPECT_EQ(pplib::Compression::Algo_BZIP2, file.getCompression());
    EXPECT_EQ(pplib::String("Compressed Bzip2"), file.getName());
    EXPECT_EQ(pplib::String("Testing Bzip2 compression in PFPFile"), file.getDescription());

    pplib::PFPFile::Iterator it;
    pplib::PFPChunk* chunk = file.findFirstChunk(it, "DATA");
    ASSERT_NE(nullptr, chunk);
    EXPECT_GT(chunk->size(), 1000u);
}

TEST_F(PFPFileTest, SaveAndLoadEmptyFile)
{
    const pplib::String filename("tmp/test_empty.pfp");
    {
        pplib::PFPFile file;
        file.setId("VOID");
        file.setVersion(0, 0);
        ASSERT_NO_THROW(file.save(filename));
    }

    ASSERT_TRUE(pplib::File::exists(filename));

    pplib::PFPFile file;
    ASSERT_NO_THROW(file.load(filename));
    EXPECT_EQ(pplib::String("VOID"), file.getID());
    EXPECT_EQ(0, file.getMainVersion());
    EXPECT_EQ(0, file.getSubVersion());
    EXPECT_EQ(file.begin(), file.end());
}

TEST_F(PFPFileTest, PredefinedChunksOrderingAndNoOverwrite)
{
    const pplib::String filename("tmp/test_dup.pfp");
    {
        pplib::PFPFile file;
        file.setId("ORDR");
        file.setName("NameChunk");
        file.setAuthor("AuthorChunk");
        file.setDescription("DescChunk");
        file.setCopyright("CopyChunk");
        file.addChunk(pplib::PFPChunk("USER", pplib::ByteArray("UserContent", 11)));

        ASSERT_NO_THROW(file.save(filename));
    }

    pplib::PFPFile file;
    ASSERT_NO_THROW(file.load(filename));

    // Verify all four predefined chunks were saved without overwriting each other
    EXPECT_EQ(pplib::String("NameChunk"), file.getName());
    EXPECT_EQ(pplib::String("AuthorChunk"), file.getAuthor());
    EXPECT_EQ(pplib::String("DescChunk"), file.getDescription());
    EXPECT_EQ(pplib::String("CopyChunk"), file.getCopyright());

    pplib::PFPFile::Iterator it;
    EXPECT_NE(nullptr, file.findFirstChunk(it, "USER"));
}

TEST_F(PFPFileTest, IdentInvalidFiles)
{
    pplib::PFPFile file;
    EXPECT_FALSE(file.ident("nonexisting_file_which_does_not_exist.pfp"));

    // Buffer too small (< 24 bytes)
    char tiny[23] = {0};
    EXPECT_FALSE(file.ident(pplib::ByteArrayPtr(tiny, sizeof(tiny))));

    // Wrong magic
    char badMagic[24] = "NOT_PFP-File";
    badMagic[8] = 3;
    EXPECT_FALSE(file.ident(pplib::ByteArrayPtr(badMagic, 24)));

    // Wrong version (!= 3)
    char badVersion[24] = "PFP-File";
    badVersion[8] = 2; // version 2
    badVersion[9] = 24;
    EXPECT_FALSE(file.ident(pplib::ByteArrayPtr(badVersion, 24)));

    // Null buffer
    EXPECT_FALSE(file.ident(pplib::ByteArrayPtr(nullptr, 0)));

    // MemFile with invalid magic
    pplib::MemFile mf1((void*)badMagic, 24);
    EXPECT_FALSE(file.ident(mf1));

    // MemFile with invalid version
    pplib::MemFile mf2((void*)badVersion, 24);
    EXPECT_FALSE(file.ident(mf2));

    // MemFile too short
    char shortBuf[10] = {0};
    pplib::MemFile mf3((void*)shortBuf, sizeof(shortBuf));
    EXPECT_FALSE(file.ident(mf3));
}

TEST_F(PFPFileTest, UseMemoryInvalidFormats)
{
    pplib::PFPFile file;

    // Buffer too short
    char buf23[23] = {0};
    EXPECT_THROW(file.useMemory(pplib::ByteArrayPtr(buf23, sizeof(buf23))), pplib::InvalidFormatException);

    // Bad magic
    char badMagic[30] = "BADMAGIC";
    badMagic[8] = 3;
    badMagic[9] = 24;
    EXPECT_THROW(file.useMemory(pplib::ByteArrayPtr(badMagic, 30)), pplib::InvalidFormatException);

    // Bad version
    char badVer[30] = "PFP-File";
    badVer[8] = 4;
    badVer[9] = 24;
    EXPECT_THROW(file.useMemory(pplib::ByteArrayPtr(badVer, 30)), pplib::InvalidFormatException);

    // Header size larger than buffer
    char badHsize[30] = "PFP-File";
    badHsize[8] = 3;
    badHsize[9] = 40; // > 30
    EXPECT_THROW(file.useMemory(pplib::ByteArrayPtr(badHsize, 30)), pplib::InvalidFormatException);

    // Header size < 24
    badHsize[9] = 20;
    EXPECT_THROW(file.useMemory(pplib::ByteArrayPtr(badHsize, 30)), pplib::InvalidFormatException);

    // Compressed flag set, but buffer too small for compression header (< hsize + 8)
    char compTrunc[28] = "PFP-File";
    compTrunc[8] = 3;
    compTrunc[9] = 24;
    compTrunc[16] = (char)pplib::Compression::Algo_ZLIB;
    EXPECT_THROW(file.useMemory(pplib::ByteArrayPtr(compTrunc, 28)), pplib::InvalidFormatException);

    // Test load(FileObject&) exceptions
    pplib::MemFile mfShort((void*)buf23, sizeof(buf23));
    EXPECT_THROW(file.load(mfShort), pplib::InvalidFormatException);

    pplib::MemFile mfBadMagic((void*)badMagic, sizeof(badMagic));
    EXPECT_THROW(file.load(mfBadMagic), pplib::InvalidFormatException);

    pplib::MemFile mfBadVer((void*)badVer, sizeof(badVer));
    EXPECT_THROW(file.load(mfBadVer), pplib::InvalidFormatException);
}

TEST_F(PFPFileTest, UseMemoryIgnoresCorruptedChunks)
{
    // Build a buffer with:
    // Header (24 bytes)
    // 1st chunk: invalid name (contains control character 0x01)
    // 2nd chunk: valid "GOOD" chunk
    // ENDF chunk
    pplib::ByteArray buf;
    char* p = (char*)buf.malloc(24 + 16 + 12 + 8);
    memcpy(p, "PFP-File", 8);
    pplib::Poke8(p + 8, 3);
    pplib::Poke8(p + 9, 24);
    memcpy(p + 10, "TEST", 4);
    pplib::Poke8(p + 14, 0);
    pplib::Poke8(p + 15, 1);
    pplib::Poke8(p + 16, 0); // No compression

    // Corrupted chunk with illegal ASCII character in name
    size_t offset = 24;
    p[offset + 0] = 'B';
    p[offset + 1] = 0x01; // Invalid ASCII
    p[offset + 2] = 'A';
    p[offset + 3] = 'D';
    pplib::Poke32(p + offset + 4, 16); // 8 header + 8 data
    memcpy(p + offset + 8, "12345678", 8);
    offset += 16;

    // Valid chunk
    memcpy(p + offset, "GOOD", 4);
    pplib::Poke32(p + offset + 4, 12); // 8 header + 4 data
    memcpy(p + offset + 8, "DATA", 4);
    offset += 12;

    // ENDF
    memcpy(p + offset, "ENDF", 4);
    pplib::Poke32(p + offset + 4, 0);
    offset += 8;

    pplib::PFPFile file;
    // Should NOT throw, but ignore the corrupted chunk and successfully parse GOOD
    ASSERT_NO_THROW(file.useMemory(pplib::ByteArrayPtr(p, offset)));

    pplib::PFPFile::Iterator it;
    pplib::PFPChunk* good = file.findFirstChunk(it, "GOOD");
    ASSERT_NE(nullptr, good);
    EXPECT_EQ(4u, good->size());
    EXPECT_EQ(0, memcmp("DATA", good->data(), 4));
}

TEST_F(PFPFileTest, LoadRealWorldFontFile)
{
    // Try finding liberationsans2.fnt6
    pplib::String path;
    if (pplib::File::exists("../resource/liberationsans2.fnt6")) {
        path = "../resource/liberationsans2.fnt6";
    } else if (pplib::File::exists("resource/liberationsans2.fnt6")) {
        path = "resource/liberationsans2.fnt6";
    }

    if (path.isEmpty()) {
        GTEST_SKIP() << "resource/liberationsans2.fnt6 not found";
    }

    pplib::PFPFile file;
    EXPECT_TRUE(file.ident(path));
    EXPECT_EQ(pplib::String("FONT"), file.getID());
    EXPECT_EQ(6, file.getMainVersion());
    EXPECT_EQ(0, file.getSubVersion());
    EXPECT_EQ(pplib::Compression::Algo_ZLIB, file.getCompression());

    ASSERT_NO_THROW(file.load(path));

    // Verify chunks can be found
    pplib::PFPFile::Iterator it;
    pplib::PFPChunk* face = file.findFirstChunk(it, "FACE");
    EXPECT_NE(nullptr, face);
    if (face) {
        EXPECT_GT(face->size(), 0u);
    }
}

TEST_F(PFPFileTest, LoadTruncatedFontFileThrows)
{
    const pplib::String path("testdata/fonts/freesans4.fnt5");
    if (!pplib::File::exists(path)) {
        GTEST_SKIP() << "testdata/fonts/freesans4.fnt5 not found";
    }

    pplib::PFPFile file;
    EXPECT_TRUE(file.ident(path));
    EXPECT_EQ(pplib::String("FONT"), file.getID());
    EXPECT_EQ(5, file.getMainVersion());

    // freesans4.fnt5 is truncated (stored sizecomp exceeds file length)
    // -> load must detect this and throw InvalidFormatException
    EXPECT_THROW(file.load(path), pplib::InvalidFormatException);
}

TEST_F(PFPFileTest, ListFunctionality)
{
    pplib::PFPFile file;
    file.setId("LIST");
    file.setVersion(1, 0);
    file.setName("Listable File");
    file.setAuthor("Author L");
    file.setDescription("A file to test list output");
    file.setCopyright("PPL (c) 2026");
    file.addChunk(pplib::PFPChunk("CHK1", pplib::ByteArray("val1", 4)));

    // Should run smoothly without crashes
    testing::internal::CaptureStdout();
    file.list();
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, output.find("PFP-File Version 3"));
    EXPECT_NE(std::string::npos, output.find("Listable File"));
    EXPECT_NE(std::string::npos, output.find("CHK1"));

    // Empty file
    pplib::PFPFile emptyFile;
    testing::internal::CaptureStdout();
    emptyFile.list();
    std::string emptyOut = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, emptyOut.find("Keine Chunks vorhanden"));

    // Zlib file
    pplib::PFPFile zlibFile;
    zlibFile.setCompression(pplib::Compression::Algo_ZLIB);
    testing::internal::CaptureStdout();
    zlibFile.list();
    std::string zlibOut = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, zlibOut.find("Zlib"));

    // Bzip2 file
    pplib::PFPFile bzFile;
    bzFile.setCompression(pplib::Compression::Algo_BZIP2);
    testing::internal::CaptureStdout();
    bzFile.list();
    std::string bzOut = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, bzOut.find("Bzip2"));
}

TEST_F(PFPFileTest, MoveSemantics)
{
    pplib::PFPFile f1;
    f1.setId("MOVE");
    f1.setVersion(2, 3);
    f1.setName("Moving Object");
    f1.addChunk(pplib::PFPChunk("DATA", pplib::ByteArray("MovePayload", 11)));

    // Move constructor
    pplib::PFPFile f2(std::move(f1));
    EXPECT_EQ(pplib::String("MOVE"), f2.getID());
    EXPECT_EQ(2, f2.getMainVersion());
    EXPECT_EQ(3, f2.getSubVersion());
    EXPECT_EQ(pplib::String("Moving Object"), f2.getName());

    pplib::PFPFile::Iterator it;
    EXPECT_NE(nullptr, f2.findFirstChunk(it, "DATA"));

    // Move assignment
    pplib::PFPFile f3;
    f3 = std::move(f2);
    EXPECT_EQ(pplib::String("MOVE"), f3.getID());
    EXPECT_EQ(pplib::String("Moving Object"), f3.getName());
    EXPECT_NE(nullptr, f3.findFirstChunk(it, "DATA"));
}

} // namespace
