#include <gtest/gtest.h>
#include <thread>
#include <chrono>

#include <pplib/core/shutdown.h>

namespace
{

class ShutdownTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        pplib::ResetShutdown();
    }

    void TearDown() override
    {
        pplib::UninstallShutdownHandler();
        pplib::ResetShutdown();
    }
};

TEST_F(ShutdownTest, InitialStateNotRequested)
{
    EXPECT_FALSE(pplib::IsShutdownRequested());
}

TEST_F(ShutdownTest, RequestShutdownSetsFlag)
{
    pplib::RequestShutdown();
    EXPECT_TRUE(pplib::IsShutdownRequested());
}

TEST_F(ShutdownTest, ResetShutdownClearsFlag)
{
    pplib::RequestShutdown();
    ASSERT_TRUE(pplib::IsShutdownRequested());

    pplib::ResetShutdown();
    EXPECT_FALSE(pplib::IsShutdownRequested());
}

TEST_F(ShutdownTest, WaitForShutdownTimesOut)
{
    auto start = std::chrono::steady_clock::now();
    bool signaled = pplib::WaitForShutdown(50);
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();

    EXPECT_FALSE(signaled);
    EXPECT_GE(elapsed, 40);
}

TEST_F(ShutdownTest, WaitForShutdownReturnsImmediatelyIfAlreadySignaled)
{
    pplib::RequestShutdown();
    EXPECT_TRUE(pplib::WaitForShutdown(1000));
}

TEST_F(ShutdownTest, WaitForShutdownWokenByAsyncRequest)
{
    std::thread t([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        pplib::RequestShutdown();
    });

    bool signaled = pplib::WaitForShutdown(500);
    EXPECT_TRUE(signaled);
    EXPECT_TRUE(pplib::IsShutdownRequested());

    t.join();
}

TEST_F(ShutdownTest, InstallAndUninstallHandler)
{
    EXPECT_NO_THROW(pplib::InstallShutdownHandler());
    EXPECT_NO_THROW(pplib::UninstallShutdownHandler());
}

} // namespace