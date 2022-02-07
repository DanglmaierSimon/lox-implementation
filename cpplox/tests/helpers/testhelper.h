#pragma once

#include <gtest/gtest.h>

class End2EndTest : public ::testing::Test
{
protected:
  void run(const char* source);
};
