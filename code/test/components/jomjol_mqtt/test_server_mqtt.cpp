#include <unity.h>
#include <server_mqtt.h>

void test_createNodeId()
{
    std::string topic = "watermeter";
    TEST_ASSERT_EQUAL_STRING("watermeter", createNodeId(topic).c_str());

    topic = "/watermeter";
    TEST_ASSERT_EQUAL_STRING("watermeter", createNodeId(topic).c_str());

    topic = "home/test/watermeter";
    TEST_ASSERT_EQUAL_STRING("watermeter", createNodeId(topic).c_str());

    topic = "home/test/subtopic/something/test/watermeter";
    TEST_ASSERT_EQUAL_STRING("watermeter", createNodeId(topic).c_str());
}

void test_mqtt()
{
    test_createNodeId();
}