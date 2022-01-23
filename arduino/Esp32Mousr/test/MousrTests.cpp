#include <unity.h>

#include "Log.h"
#include "Mousr.h"

void test_connectionStatusMap()
{
    string expected = "Unknown";
    string actual = s_MousrConnectionStatusToStringMap[MousrConnectionStatus::Unknown];
    TEST_ASSERT_EQUAL_STRING(expected.c_str(), actual.c_str());

    int count = 0;

    // Add 1 to max to get an invalid value and make sure it's handled gracefully
    for (; count <= (int)MousrConnectionStatus::Max + 1; count++)
    {
        actual = getMousrConnectionStatusString((MousrConnectionStatus)count);
        s_writeLogF("%d -> %s\n", count, actual.c_str());
    }

    TEST_ASSERT_EQUAL_INT32((int)MousrConnectionStatus::Max, count - 2);
}

void test_convertToBytes()
{
    float f = 174.9593;
    string expected = "0x95f52e43";
    uint8_t *b = MousrData::toBytes(f);
    string actual = MousrData::toString(b, sizeof(f));
    s_writeLogF("%f -> %s\n", f, actual.c_str());
    TEST_ASSERT_EQUAL_STRING(expected.c_str(), actual.c_str());

    float ff;
    MousrData::fromBytes(b, ff);
    s_writeLogF("%f -> %f\n", f, ff);
    TEST_ASSERT_EQUAL(f, ff);
    free(b);

    unsigned long l = 123456789123456789;
    expected = "0x155fd0ac4b9bb601";
    b = MousrData::toBytes(l);
    actual = MousrData::toString(b, sizeof(l));
    s_writeLogF("%llu -> %s\n", l, actual.c_str());
    TEST_ASSERT_EQUAL_STRING(expected.c_str(), actual.c_str());

    unsigned long ll;
    MousrData::fromBytes(b, ll);
    s_writeLogF("%llu -> %llu\n", l, ll);
    TEST_ASSERT_EQUAL(l, ll);
    free(b);

    int i = -123456;
    expected = "0xc01dfeff";
    b = MousrData::toBytes(i);
    actual = MousrData::toString(b, sizeof(i));
    s_writeLogF("%d -> %s\n", i, actual.c_str());
    TEST_ASSERT_EQUAL_STRING(expected.c_str(), actual.c_str());

    int ii;
    MousrData::fromBytes(b, ii);
    s_writeLogF("%d -> %d\n", i, ii);
    TEST_ASSERT_EQUAL(i, ii);
    free(b);
}

void test_mousrAlloc()
{
    s_writeLogLn("test_mousrAlloc()");
    string data = "0x307b3c0b3fce824a3ebd45933f00030000000000";
    MousrData d(data);
    d = MousrData(data);
    d = MousrData(data);
    d = MousrData(data);
}

void test_getRawData()
{
    s_writeLogLn("test_getRawData()");
    string data = "0x307b3c0b3fce824a3ebd45933f00030000000000";
    MousrData d(data);
    uint8_t *raw;
    size_t length;
    d.getRawMessageData(&raw, length);
    TEST_ASSERT_EQUAL(d.getMessageLength(), length);
    s_writeLogF("Output: %s\n", MousrData::toString(raw, length).c_str());
    TEST_ASSERT_EQUAL_STRING(d.toString().c_str(), MousrData::toString(raw, length).c_str());
    free(raw);
}

void test_createFromRaw()
{
    float f1 = 0.2312536;
    float f2 = 0.0;
    float f3 = 93.02859;

    // 30bdcd6c3e00000000a40eba420200
    // 30becd6c3e00000000a30eba420200

    string expected = "0x30becd6c3e00000000a30eba420200";
    vector<uint8_t> data;
    MousrData::append(data, (uint8_t)MousrMessage::ROBOT_POSE);
    MousrData::append(data, f1);
    MousrData::append(data, f2);
    MousrData::append(data, f3);
    MousrData::append(data, (uint8_t)MousrCommand::MOVE);
    MousrData::append(data, false);

    TEST_ASSERT_EQUAL(15, data.size());
    string actual = MousrData::toString(data);
    s_writeLogF("Expected: %s Actual: %s\n", expected.c_str(), actual.c_str());
    TEST_ASSERT_EQUAL_STRING(expected.c_str(), actual.c_str());
}

void test_ParseMessage()
{
    // ROBOT_POSE message
    {
        string data = "0x307b3c0b3fce824a3ebd45933f00030000000000";
        MousrData d(data);
        TEST_ASSERT_EQUAL(MousrMessage::ROBOT_POSE, d.getMessageKind());
        TEST_ASSERT_EQUAL(20, d.getMessageLength());

        auto cooked = d.getMessageData();
        TEST_ASSERT_EQUAL(MousrMessage::ROBOT_POSE, cooked->msg);
        TEST_ASSERT_EQUAL_FLOAT(1.150566, cooked->movement.angle);
        TEST_ASSERT_EQUAL_FLOAT(0.197765, cooked->movement.held);
        TEST_ASSERT_EQUAL_FLOAT(0.543892, cooked->movement.speed);
    }

    {
        // BATTERY_VOLTAGE
        string data = "0x625c000000002015000000000000000000000000";
        MousrData d = MousrData(data);
        TEST_ASSERT_EQUAL(MousrMessage::BATTERY_VOLTAGE, d.getMessageKind());
        TEST_ASSERT_EQUAL(20, d.getMessageLength());

        auto cooked = d.getMessageData();
        TEST_ASSERT_EQUAL(MousrMessage::BATTERY_VOLTAGE, cooked->msg);
        TEST_ASSERT_EQUAL(92, cooked->battery.voltage);
    }
}

void test_toHexString()
{
    string data = "0x307b3c0b3fce824a3ebd45933f00030000000000";
    MousrData d(data);
    string actual = d.toString();
    TEST_ASSERT_EQUAL_STRING(data.c_str(), actual.c_str());
}