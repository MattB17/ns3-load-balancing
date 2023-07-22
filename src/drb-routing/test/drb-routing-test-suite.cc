
// Include a header file from your module to test.
#include "ns3/drb-routing.h"

// An essential include is test.h
#include "ns3/test.h"

// Do not put your test classes in namespace ns3.  You may find it useful
// to use the using directive to access the ns3 namespace directly
using namespace ns3;

// Add a doxygen group for tests.
// If you have more than one test, this should be in only one of them.
/**
 * \defgroup drb-routing-tests Tests for drb-routing
 * \ingroup drb-routing
 * \ingroup tests
 */

// This is an example TestCase.
/**
 * \ingroup drb-routing-tests
 * Test case for feature 1
 */
class DrbRoutingTestCase1 : public TestCase
{
  public:
    DrbRoutingTestCase1();
    virtual ~DrbRoutingTestCase1();

  private:
    void DoRun() override;
};

// Add some help text to this case to describe what it is intended to test
DrbRoutingTestCase1::DrbRoutingTestCase1()
    : TestCase("DrbRouting test case (does nothing)")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
DrbRoutingTestCase1::~DrbRoutingTestCase1()
{
}

//
// This method is the pure virtual method from class TestCase that every
// TestCase must implement
//
void
DrbRoutingTestCase1::DoRun()
{
    // A wide variety of test macros are available in src/core/test.h
    NS_TEST_ASSERT_MSG_EQ(true, true, "true doesn't equal true for some reason");
    // Use this one for floating point comparisons
    NS_TEST_ASSERT_MSG_EQ_TOL(0.01, 0.01, 0.001, "Numbers are not equal within tolerance");
}

// The TestSuite class names the TestSuite, identifies what type of TestSuite,
// and enables the TestCases to be run.  Typically, only the constructor for
// this class must be defined

/**
 * \ingroup drb-routing-tests
 * TestSuite for module drb-routing
 */
class DrbRoutingTestSuite : public TestSuite
{
  public:
    DrbRoutingTestSuite();
};

DrbRoutingTestSuite::DrbRoutingTestSuite()
    : TestSuite("drb-routing", UNIT)
{
    // TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
    AddTestCase(new DrbRoutingTestCase1, TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
/**
 * \ingroup drb-routing-tests
 * Static variable for test initialization
 */
static DrbRoutingTestSuite sdrbRoutingTestSuite;
