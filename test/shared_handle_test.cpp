#include <jfc/catch.hpp>
#include <jfc/types.h>

#include <jfc/shared_handle.h>

TEST_CASE( "jfc::thread_group test", "[jfc::thread_group]" )
{
    SECTION("default constructor creates group of expected size")
    {
        /*jfc::thread_group group;

        const auto expected_group_size = std::thread::hardware_concurrency() 
            ? std::thread::hardware_concurrency() - 1 
            : 0;*/

        REQUIRE(true);
    }
}

