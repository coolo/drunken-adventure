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
  f.blink();
  
  // 2 and 1 disappear
  ASSERT_EQ(f.to_string(),
	    "       \n"
	    "       \n"
	    "       \n"
	    "       \n"
	    "       \n"
	    "       \n"
	    "   3 47\n");

  f = f.drop('4', 5);
  f.blink();
  
  // both 4s disappear
  ASSERT_EQ(f.to_string(),
	    "       \n"
	    "       \n"
	    "       \n"
	    "       \n"
	    "       \n"
	    "       \n"
	    "   3  7\n");

  init =
    "       \n"
    "       \n"
    "       \n"
    "       \n"
    "       \n"
    "  4B 5 \n"
    "121165A\n";
  
  f = Field::from_string(init);
  f = f.drop('1', 1);
  f.blink();
  
  ASSERT_EQ(f.to_string(),
	    "       \n"
	    "       \n"
	    "       \n"
	    "       \n"
	    "       \n"
	    "  4B 5 \n"
	    "121165A\n");
  
  f = Field::from_string(init);
  f = f.drop('1', 4);
  f.blink();
  
  ASSERT_EQ(f.to_string(),
	    "       \n"
	    "       \n"
	    "       \n"
	    "       \n"
	    "       \n"
	    "  4A 5 \n"
	    " 21165A\n");

}

TEST_F(FieldTest, Changes) {

  const char *init =
    "       \n"
    "       \n"
    "       \n"
    "       \n"
    "       \n"
    "       \n"
    "2  3 4B\n";
  
  Field f = Field::from_string(init);
  f = f.drop('1', 7);
  f.blink();
  
  // 1 disappears and turns
  ASSERT_EQ(f.to_string(),
	    "       \n"
	    "       \n"
	    "       \n"
	    "       \n"
	    "       \n"
	    "       \n"
	    "2  3 4A\n");
}

TEST_F(FieldTest, Scores) {

  srand(0); // makes the A as a 1
  const char *init =
    "       \n"
    "       \n"
    "       \n"
    "       \n"
    "       \n"
    "   71 6\n"
    "35 76A3\n";
  
  Field f = Field::from_string(init);
  Field f2 = f.drop('7', 7);
  ASSERT_EQ(f2.to_string(),
	    "       \n"
	    "       \n"
	    "       \n"
	    "       \n"
	    "      7\n"
	    "   71 6\n"
	    "35 76A3\n");
  
  f2.blink();
  
  ASSERT_EQ(f2.to_string(),
	    "       \n"
	    "       \n"
	    "       \n"
	    "       \n"
	    "       \n"
	    "   71 7\n"
	    "35 7616\n");

  ASSERT_GT(f.drop('7', 7).rating(),
	    f.drop('7', 3).rating());
  ASSERT_LT(f.drop('7', 3).rating(),
	    f.drop('7', 4).rating());

  init =
    "       \n"
    "       \n"
    "       \n"
    "       \n"
    "       \n"
    "      6\n"
    "35 76A3\n";
  
  f = Field::from_string(init);

  int r7 = f.drop('B', 7).rating();
  int r3 = f.drop('B', 3).rating();
  ASSERT_GT(r7, r3);
  ASSERT_LT(r3, f.drop('B', 4).rating());

}

TEST_F(FieldTest, Scores2) {
    const char *init =
      "       \n"
      "       \n"
      "       \n"
      "       \n"
      "7566 7B\n"
      "AA65 5A\n"
      "AABB6BA\n";

    Field f = Field::from_string(init);

    // the rating of the disappearing drop is better
    ASSERT_LT(f.drop('7', 5).rating(),
	      f.drop('7', 7).rating());

    // but looking deeper, the 7 doesn't do harm
    ASSERT_LT(int(f.drop('7', 5).recursive_rating(3)),
	      int(f.drop('7', 7).recursive_rating(3)));
    
    init =
      "       \n"
      "       \n"
      "       \n"
      "7  B 7 \n"
      "1A 66A5\n"
      "A661BBB\n"
      "B5ABB2A\n";
    
    f = Field::from_string(init);
    f.drop('1', 4);
}
    
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

