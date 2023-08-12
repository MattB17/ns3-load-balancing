#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv4-letflow-routing-helper.h"
#include "ns3/ipv4-letflow-routing.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/log.h"
#include "ns3/node-container.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/pointer.h"
#include "ns3/simple-channel.h"
#include "ns3/simple-net-device-helper.h"
#include "ns3/simple-net-device.h"
#include "ns3/simulator.h"
#include "ns3/test.h"
#include "ns3/uinteger.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Ipv4LetFlowRoutingTestSuite");

/**
 * \defgroup letflow-routing-tests Tests for letflow-routing
 * \ingroup letflow-routing
 * \ingroup tests
 * 
 * This test suite tests the operation of per flow ECMP routing on a few
 * sample networks to ensure that routes are built correctly.
 * 
 * Link test:
 *    n0 <---------> n1 (point-to-point link)
 * 10.1.1.1       10.1.1.2
 *    Expected routes:
 *       n0: route to 0.0.0.0 gw 10.1.1.2
 *       n1: route to 0.0.0.0 gw 10.1.1.1
 *     Note: these default routes to 0.0.0.0 are generated by the extension
 *           in the global route manager to install default routes via the
 *           peer node on a point-to-point link, when the node is on a stub
 *           link.
 */


/**
 * \ingroup letflow-routing-tests
 * 
 * \brief Ipv4 LetFlow Routing link test.
 */
class LinkTest : public TestCase {
public:
  void DoSetup() override;
  void DoRun() override;
  LinkTest();

private:
  // Nodes used in the test.
  NodeContainer m_nodes;
};

LinkTest::LinkTest()
    : TestCase("ECMP flow routing on a point-to-point link") {}

void LinkTest::DoSetup() {
  m_nodes.Create(2);

  Ptr<SimpleChannel> channel = CreateObject<SimpleChannel>();
  SimpleNetDeviceHelper simpleHelper;
  simpleHelper.SetNetDevicePointToPointMode(true);
  NetDeviceContainer net = simpleHelper.Install(m_nodes, channel);

  InternetStackHelper internet;
  // By default, InternetStackHelper adds a static and global routing
  // implementation. We want ECMP flow-level for this test.
  Ipv4LetFlowRoutingHelper letflowRouting;
  internet.SetRoutingHelper(letflowRouting);
  internet.Install(m_nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer i = ipv4.Assign(net);
}

void LinkTest::DoRun() {
  Ipv4LetFlowRoutingHelper::PopulateRoutingTables();

  Ptr<Ipv4L3Protocol> ip0 = m_nodes.Get(0)->GetObject<Ipv4L3Protocol>();
  NS_TEST_ASSERT_MSG_NE(ip0, nullptr, "Error -- no Ipv4 object");
  Ptr<Ipv4L3Protocol> ip1 = m_nodes.Get(1)->GetObject<Ipv4L3Protocol>();
  NS_TEST_ASSERT_MSG_NE(ip1, nullptr, "Error -- no Ipv4 object");

  Ptr<Ipv4RoutingProtocol> routing0 = ip0->GetRoutingProtocol();
  Ptr<Ipv4LetFlowRouting> letflowRouting0 =
      routing0->GetObject<Ipv4LetFlowRouting>();
  NS_TEST_ASSERT_MSG_NE(
    letflowRouting0, nullptr, "Error -- no Ipv4EcmpFlowRouting object");
  Ptr<Ipv4RoutingProtocol> routing1 = ip1->GetRoutingProtocol();
  Ptr<Ipv4LetFlowRouting> letflowRouting1 =
      routing1->GetObject<Ipv4LetFlowRouting>();
  NS_TEST_ASSERT_MSG_NE(
    letflowRouting1, nullptr, "Error -- no Ipv4EcmpFlowRouting object");

  // Test that the right number of routes have been found.
  uint32_t nRoutes0 = letflowRouting0->GetNRoutes();
  NS_LOG_DEBUG("LinkTest nRoutes0 " << nRoutes0);
  NS_TEST_ASSERT_MSG_EQ(nRoutes0, 1, "Error -- not one route");
  Ipv4RoutingTableEntry* route = letflowRouting0->GetRoute(0);
  NS_LOG_DEBUG(
    "Entry dest " << route->GetDest() << " gw " << route->GetGateway());
  NS_TEST_ASSERT_MSG_EQ(
    route->GetDest(), Ipv4Address("0.0.0.0"), "Error - wrong destination");
  NS_TEST_ASSERT_MSG_EQ(
    route->GetGateway(), Ipv4Address("10.1.1.2"), "Error - wrong gateway");

  // Test that the right number of routes have been found.
  uint32_t nRoutes1 = letflowRouting1->GetNRoutes();
  NS_LOG_DEBUG("LinkTest nRoutes1 " << nRoutes1);
  NS_TEST_ASSERT_MSG_EQ(nRoutes1, 1, "Error -- not one route");
  route = letflowRouting1->GetRoute(0);
  NS_LOG_DEBUG(
    "Entry dest " << route->GetDest() << " gw " << route->GetGateway());
  NS_TEST_ASSERT_MSG_EQ(
    route->GetDest(), Ipv4Address("0.0.0.0"), "Error - wrong destination");
  NS_TEST_ASSERT_MSG_EQ(
    route->GetGateway(), Ipv4Address("10.1.1.1"), "Error - wrong gateway");

  Simulator::Run();
  Simulator::Destroy();
}

class LetflowRoutingTestCase1 : public TestCase
{
  public:
    LetflowRoutingTestCase1();
    virtual ~LetflowRoutingTestCase1();

  private:
    void DoRun() override;
};

// Add some help text to this case to describe what it is intended to test
LetflowRoutingTestCase1::LetflowRoutingTestCase1()
    : TestCase("LetflowRouting test case (does nothing)")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
LetflowRoutingTestCase1::~LetflowRoutingTestCase1()
{
}

//
// This method is the pure virtual method from class TestCase that every
// TestCase must implement
//
void
LetflowRoutingTestCase1::DoRun()
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
 * \ingroup letflow-routing-tests
 * TestSuite for module letflow-routing
 */
class LetflowRoutingTestSuite : public TestSuite
{
  public:
    LetflowRoutingTestSuite();
};

LetflowRoutingTestSuite::LetflowRoutingTestSuite()
    : TestSuite("ipv4-letflow-routing", UNIT)
{
    // TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
    AddTestCase(new LinkTest, TestCase::QUICK);
    AddTestCase(new LetflowRoutingTestCase1, TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
/**
 * \ingroup letflow-routing-tests
 * Static variable for test initialization
 */
static LetflowRoutingTestSuite sletflowRoutingTestSuite;
