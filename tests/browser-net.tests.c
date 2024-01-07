// browser-net.c
#include "Unity/src/unity.h"
#include "helpers.h"
#include "../src/sds/sds.h"

#include "../src/browser-net.h"
#include "mocks/Mockutil.h"
#include "mocks/Mockbrowser-cli.h"

const char ip[] = "127.0.0.1";
const char port[] = "10800";
const char URL[] = "/test";

// Func_Returns_When

void test_get_page_ReturnsEmptyLine_WhenURLIsBlank(void) {
	streq_ExpectAndReturn("blank", "blank", 1);
	sds page = get_page(ip, port, "blank");

	TEST_ASSERT_EQUAL_STRING(page, "\n");

	sdsfree(page);
}

void test_get_page_ReturnsMessage_WhenCannotConnectToServer(void) {
	streq_ExpectAndReturn(URL, "blank", 0);
	herrc_Expect(3, "socket");
	atop_ExpectAndReturn("0", 0);
	/* aton_ExpectAndReturn("255.255.255.255", NULL, -1); */
	/* aton_IgnoreArg_output(); */
	herrc_Expect(0, "inet_aton");
	herrc_Expect(0, "connect");

	sds page = get_page("255.255.255.255", "0", URL);

	TEST_ASSERT_EQUAL_STRING(page, "Couldn't connect to server!\n");

	sdsfree(page);
}

void test_get_page_ReturnsGivenPage_WhenURLIsCorrect(void) {
	TEST_ASSERT_TRUE(1);
}
