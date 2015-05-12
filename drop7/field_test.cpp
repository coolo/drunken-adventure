#include "Field.h"
#include <cassert>
#include <iostream>
#include <gtest/gtest.h>

using namespace std;

// The fixture for testing class Foo.
class FieldTest : public ::testing::Test {};

TEST_F(FieldTest, Basics) {
  Field empty;
  ASSERT_EQ(' ', empty.at(0,0));
  
  const char *field1 =
    "       \n"
    "       \n"
    "       \n"
    "       \n"
    "       \n"
    "       \n"
    "       \n";
  ASSERT_EQ(field1, empty.to_string());
  
  Field f = Field::from_string(field1);
  ASSERT_EQ(field1, f.to_string());

  for (int i = 0; i < 7; ++i)
    f.set(i, i, '1' + i);
  
  const char *field2 =
    "1      \n"
    " 2     \n"
    "  3    \n"
    "   4   \n"
    "    5  \n"
    "     6 \n"
    "      7\n";

  Field f2 = Field::from_string(field2);
  ASSERT_EQ(field2, f2.to_string());

}

TEST_F(FieldTest, Drops) {

  const char *init =
    "       \n"
    "       \n"
    "       \n"
    "       \n"
    "       \n"
    "       \n"
    "2  3 47\n";
  
  Field f = Field::from_string(init);
  f = f.drop('1', 2);
  ASSERT_EQ(f.to_string(),
	    "       \n"
	    "       \n"
	    "       \n"
	    "       \n"
	    "       \n"
	    "       \n"
	    "21 3 47\n");
}
  
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

