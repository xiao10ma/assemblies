#include <iostream>
#include <gtest/gtest.h>
#include "parse.h"

// Subject Verb Obj
TEST(testCase1, subject_verb_obj) {
    testing::internal::CaptureStdout();
    parse_main("cats chase mice");
    std::string output = testing::internal::GetCapturedStdout();
    std::string expected_output = "Dependency Parsing Result:\n"
                                "chase->cats (SUBJ)\n"
                                "chase->mice (OBJ)\n"
                                "\nParsing Tree:\n"
                                "chase\n"
                                "    └── cats\n"
                                "    └── mice\n";
    EXPECT_EQ(output, expected_output);
}

// Subject Verb Obk
TEST(testCase2, subject_verb_obj) {
    testing::internal::CaptureStdout();
    parse_main("people love dog");
    std::string output = testing::internal::GetCapturedStdout();
    std::string expected_output = 
        "Dependency Parsing Result:\n"
        "love->people (SUBJ)\n"
        "love->dog (OBJ)\n"
        "\nParsing Tree:\n"
        "love\n"
        "    └── people\n"
        "    └── dog\n";
    EXPECT_EQ(output, expected_output);
}

// Det Adj Subj Det Obj Adverb
TEST(testCase3, Det_Adj_Subj_Det_Obj_Adverb) {
    testing::internal::CaptureStdout();
    parse_main("the big dog chase the mice quickly");
    std::string output = testing::internal::GetCapturedStdout();
    std::string expected_output = 
        "Dependency Parsing Result:\n"
        "chase->dog (SUBJ)\n"
        "chase->mice (OBJ)\n"
        "chase->quickly (ADVERB)\n"
        "mice->the (DET)\n"
        "dog->the (DET)\n"
        "dog->big (ADJ)\n"
        "\nParsing Tree:\n"
        "chase\n"
        "    └── dog\n"
        "        └── the\n"
        "        └── big\n"
        "    └── mice\n"
        "        └── the\n"
        "    └── quickly\n";
    EXPECT_EQ(output, expected_output);
}

// Det Adj Obj Verb Adv
TEST(testCase4, Subj_Verb_Obj_Prep_Det_Adj_Obj) {
    testing::internal::CaptureStdout();
    parse_main("my small bird sing slowly");
    std::string output = testing::internal::GetCapturedStdout();
    std::string expected_output = 
        "Dependency Parsing Result:\n"
        "sing->bird (SUBJ)\n"
        "sing->slowly (ADVERB)\n"
        "bird->my (DET)\n"
        "bird->small (ADJ)\n"
        "\nParsing Tree:\n"
        "sing\n"
        "    └── bird\n"
        "        └── my\n"
        "        └── small\n"
        "    └── slowly\n";
    EXPECT_EQ(output, expected_output);
}

// Subj Verb Obj Prep Det Adj Obj
TEST(testCase5, Subj_Verb_Obj_Prep_Det_Adj_Obj) {
    testing::internal::CaptureStdout();
    parse_main("people love cats in the big house");
    std::string output = testing::internal::GetCapturedStdout();
    std::string expected_output = 
        "Dependency Parsing Result:\n"
        "love->people (SUBJ)\n"
        "love->cats (OBJ)\n"
        "cats->house (DET)\n"
        "\nParsing Tree:\n"
        "love\n"
        "    └── people\n"
        "        └── big\n"
        "    └── cats\n"
        "        └── big\n";
    EXPECT_EQ(output, expected_output);
}


int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}