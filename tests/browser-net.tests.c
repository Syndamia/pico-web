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
