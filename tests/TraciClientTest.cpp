#include "gtest/gtest.h"
#include "traci/traci-client.h"

using namespace traciclient;
namespace {

  class TraciClientTest : public ::testing::Test {

  protected:
    Ptr<TraciClient> traciClient;
  };

  TEST_F(TraciClientTest, TraciClient) {
    ASSERT_TRUE(true);
  }

}
