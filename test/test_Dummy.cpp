#include <boost/test/unit_test.hpp>
#include <dsp1760/Dummy.hpp>

using namespace dsp1760;

BOOST_AUTO_TEST_CASE(it_should_not_crash_when_welcome_is_called)
{
    dsp1760::DummyClass dummy;
    dummy.welcome();
}
