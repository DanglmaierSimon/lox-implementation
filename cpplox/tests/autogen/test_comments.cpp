#include <memory>

#include <gtest/gtest.h>

#include "testhelper.h"

class Comments : public End2EndTest
{
};

TEST_F(Comments, line_at_eof)
{
  run(R";-](
print "ok";  // expect: ok
// comment
);-]");
}

TEST_F(Comments, only_line_comment_and_line)
{
  run(R";-](
// comment
);-]");
}

TEST_F(Comments, only_line_comment)
{
  run(R";-](
// comment
);-]");
}

TEST_F(Comments, unicode)
{
  run(R";-](
// Unicode characters are allowed in comments.
//
// Latin 1 Supplement: £§¶ÜÞ
// Latin Extended-A: ĐĦŋœ
// Latin Extended-B: ƂƢƩǁ
// Other stuff: ឃᢆ᯽₪ℜ↩⊗┺░
// Emoji: ☃☺♣

print "ok";  // expect: ok
);-]");
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}