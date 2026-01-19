#include <perforator/lib/profile/pprof.h>

#include <library/cpp/testing/gtest/gtest.h>

#include <util/generic/yexception.h>
#include <util/stream/mem.h>
#include <util/stream/zlib.h>

TEST(PProfParserTest, RejectsInvalidGzipData) {
    // Gzip magic bytes followed by garbage (not valid gzip stream)
    const char gzipData[] = "\x1f\x8b\x08\x00\x00\x00\x00\x00\x00\x03some garbage";
    TStringBuf input(gzipData, sizeof(gzipData) - 1);

    NPerforator::NProto::NProfile::Profile profile;

    // Should throw when trying to decompress invalid gzip
    EXPECT_THROW(
        NPerforator::NProfile::ConvertFromPProf(input, &profile),
        yexception
    );
}

TEST(PProfParserTest, RejectsRandomGarbage) {
    // Random bytes that don't form valid protobuf
    const char garbage[] = "\xff\xff\xff\xff\xff\xff\xff\xff";
    TStringBuf input(garbage, sizeof(garbage) - 1);

    NPerforator::NProto::NProfile::Profile profile;

    EXPECT_THROW_MESSAGE_HAS_SUBSTR(
        NPerforator::NProfile::ConvertFromPProf(input, &profile),
        yexception,
        "not a valid protobuf"
    );
}

TEST(PProfParserTest, AcceptsEmptyProfile) {
    // Empty protobuf is valid (all fields optional)
    TStringBuf input;

    NPerforator::NProto::NProfile::Profile profile;

    // Should not throw
    EXPECT_NO_THROW(NPerforator::NProfile::ConvertFromPProf(input, &profile));
}

TEST(PProfParserTest, AcceptsGzipCompressedEmptyProfile) {
    // Create gzip-compressed empty data (valid empty protobuf)
    TString compressed;
    {
        TStringOutput output(compressed);
        TZLibCompress compressor(&output, ZLib::GZip);
        // Write nothing - empty protobuf
        compressor.Finish();
    }

    NPerforator::NProto::NProfile::Profile profile;

    // Should not throw - valid gzip with empty profile inside
    EXPECT_NO_THROW(NPerforator::NProfile::ConvertFromPProf(compressed, &profile));
}
