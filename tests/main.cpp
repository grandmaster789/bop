#include <iostream>
#include <memory>
#include <gtest/gtest.h>

class ConcreteEnvironment:
	public testing::Environment // from gtest
{
public:
	~ConcreteEnvironment() override {
	}

	void SetUp() override {
	}

	void TearDown() override {
	}
};

int main(int argc, char* argv[]) {
	auto env = std::make_unique<ConcreteEnvironment>();
	::testing::AddGlobalTestEnvironment(env.get());
	::testing::InitGoogleTest(&argc, argv);

	RUN_ALL_TESTS();
}