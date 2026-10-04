#include "unity.h"
#include <stdbool.h>
#include <stdlib.h>
#include "../../examples/autotest-validate/autotest-validate.h"
#include "../../assignment-autotest/test/assignment1/username-from-conf-file.h"

void test_validate_my_username()
{
    TEST_ASSERT_EQUAL_STRING_MESSAGE(my_username(), malloc_username_from_conf_file(), "Username from my_username() does not match conf/username.txt");
}
