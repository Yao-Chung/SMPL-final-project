#include <Parser.hpp>
#include <Emit.hpp>
#include "helper.hpp"
#include <gtest/gtest.h>
#include <Graphiz.hpp>

TEST(Parser, isLetter_check){
    std::string_view s("a121sdaf");
    EXPECT_TRUE(isLetter(s));
    EXPECT_STREQ("121sdaf", s.data());
    EXPECT_FALSE(isLetter(s));
    EXPECT_STREQ("121sdaf", s.data());
}

TEST(Parser, isDigit_check){
    std::string_view s("9adsfsf");
    EXPECT_TRUE(isDigit(s));
    EXPECT_STREQ("adsfsf", s.data());
    EXPECT_FALSE(isDigit(s));
    EXPECT_STREQ("adsfsf", s.data());
}

TEST(Parser, isRelOp_check){
    std::string_view s(">=12313");
    EXPECT_TRUE(isRelOp(s));
    EXPECT_STREQ("12313", s.data());
    EXPECT_FALSE(isRelOp(s));
    EXPECT_STREQ("12313", s.data());
}

TEST(Parser, isIdent_check){
    std::string_view s("a31ad$fadf");
    EXPECT_TRUE(isIdent(s));
    EXPECT_STREQ("$fadf", s.data());
    EXPECT_FALSE(isIdent(s));
    EXPECT_STREQ("$fadf", s.data());
}

TEST(Parser, isNumber_check){
    std::string_view s("1123ashdkf");
    EXPECT_TRUE(isNumber(s));
    EXPECT_STREQ("ashdkf", s.data());
    EXPECT_FALSE(isNumber(s));
    EXPECT_STREQ("ashdkf", s.data());
}

TEST(Parser, isFactor_check){
    //Part 1: designator
    // initialize_map("a31");
    std::string_view s1("a31");
    EXPECT_TRUE(isFactor(s1));
    EXPECT_STREQ("", s1.data());
    // clear_map();
    // clear_stack();
    //Part 2: number
    // startStatSequence();
    std::string_view s2("321345");
    EXPECT_TRUE(isFactor(s2));
    EXPECT_STREQ("", s2.data());
    // clear_stack();
    // clear_graph();
    //Part 3: (expression)
    std::string_view s3 = "(321345)";
    EXPECT_TRUE(isFactor(s3));
    EXPECT_STREQ("", s3.data());
    //Part 4: funcCall
    std::string_view s4("call f5");
    EXPECT_TRUE(isFactor(s4));
    EXPECT_STREQ("", s4.data());
}

TEST(Parser, isTerm_check){
    std::string_view s1("321432");
    EXPECT_TRUE(isTerm(s1));
    EXPECT_STREQ("", s1.data());
    std::string_view s2("123*456/95");
    EXPECT_TRUE(isTerm(s2));
    EXPECT_STREQ("", s2.data());
    std::string_view s3("123*abc");
    EXPECT_TRUE(isTerm(s3));
    EXPECT_STREQ("", s3.data());
    std::string_view s4("123*==");
    EXPECT_FALSE(isTerm(s4));
    EXPECT_STREQ("123*==", s4.data());
}

TEST(Parser, isExpression_check){
    std::string_view s1("34567");
    EXPECT_TRUE(isExpression(s1));
    EXPECT_STREQ("", s1.data());
    std::string_view s2("231+78979-4565");
    EXPECT_TRUE(isExpression(s2));
    EXPECT_STREQ("", s2.data());
    std::string_view s3("231+abcde");
    EXPECT_TRUE(isExpression(s3));
    EXPECT_STREQ("", s3.data());
    std::string_view s4("231+==");
    EXPECT_FALSE(isExpression(s4));
    EXPECT_STREQ("231+==", s4.data());
}

TEST(Parser, isRelation_check){
    std::string_view s2("3618>=124");
    EXPECT_TRUE(isRelation(s2));
    EXPECT_STREQ("", s2.data());
    std::string_view s3("3618>=!124");
    EXPECT_FALSE(isRelation(s3));
    EXPECT_STREQ("3618>=!124", s3.data());
}

TEST(Parser, isDesignator_check){
    // Naive designator
    std::string_view s1("a31");
    EXPECT_TRUE(isDesignator(s1));
    EXPECT_STREQ("", s1.data());
    
    // L1 Designator
    std::string_view s2("a31[100][1001]");
    EXPECT_TRUE(isDesignator(s2));
    EXPECT_STREQ("", s2.data());

    // L2 Designator 
    std::string_view s3("a31[100][*1001");
    EXPECT_FALSE(isDesignator(s3));
    EXPECT_STREQ("a31[100][*1001", s3.data());
}

TEST(Parser, isAssignment_check){
    std::string_view s1("let a1[100]<-38");
    EXPECT_TRUE(isAssignment(s1));
    EXPECT_STREQ("", s1.data());

    std::string_view s2("let a2[100]*10");
    EXPECT_FALSE(isAssignment(s2));
    EXPECT_STREQ("let a2[100]*10", s2.data());

    std::string_view s3("let a10[20]<-3+56*709");
    EXPECT_TRUE(isAssignment(s3));
    EXPECT_STREQ("", s3.data());

    std::string_view s4("let a10[20]<-");
    EXPECT_FALSE(isAssignment(s4));
    EXPECT_STREQ("let a10[20]<-", s4.data());
}

TEST(Parser, isFuncCall_check){
    // 1. function with no parameter and with parantheses
    std::string_view s1("call f5()");
    EXPECT_TRUE(isFuncCall(s1));
    EXPECT_STREQ("", s1.data());
    // 2. function with no parameter and no parantheses
    std::string_view s2("call f5");
    EXPECT_TRUE(isFuncCall(s2));
    EXPECT_STREQ("", s2.data());
    // 3. function with multiple parameters
    std::string_view s3("call f5(10,a1,a[100])");
    EXPECT_TRUE(isFuncCall(s3));
    EXPECT_STREQ("", s3.data());
    // 4. invalid function call
    std::string_view s4("call f5(10,abc,a[100]");
    EXPECT_FALSE(isFuncCall(s4));
    EXPECT_STREQ("call f5(10,abc,a[100]", s4.data());
}

TEST(Parser, isIfStatement_check){
    // No else condition
    std::string_view s1("if 2>=1 then let a1<-38;return 0; fi");
    EXPECT_TRUE(isIfStatement(s1));
    EXPECT_STREQ("", s1.data());
    // With else condition
    std::string_view s2("if 2>=1 then let a1<-38;return 0; else let a1<-10;return 1; fi");
    EXPECT_TRUE(isIfStatement(s2));
    EXPECT_STREQ("", s2.data());
}

TEST(Parser, isWhileStatement_check){
    std::string_view s1("while 3>1 do let a1<-38;return 0; od");
    EXPECT_TRUE(isWhileStatement(s1));
    EXPECT_STREQ("", s1.data());
}

TEST(Parser, isReturnStatement_check){
    std::string_view s1("return 0");
    EXPECT_TRUE(isReturnStatement(s1));
    EXPECT_STREQ("", s1.data());

    std::string_view s2("return >=");
    EXPECT_TRUE(isReturnStatement(s2));
    EXPECT_STREQ(" >=", s2.data());
    
    std::string_view s3("return");
    EXPECT_TRUE(isReturnStatement(s3));
    EXPECT_STREQ("", s3.data());
}

TEST(Parser, isStatement_check){
    // 1. statement is assignment
    std::string_view s1("let a1[100]<-38");
    EXPECT_TRUE(isStatement(s1));
    EXPECT_STREQ("", s1.data());
    // 2. statement is function call
    std::string_view s2("call f5(10,a1,a[100])");
    EXPECT_TRUE(isStatement(s2));
    EXPECT_STREQ("", s2.data());
    // 3. statement is returnStatement
    std::string_view s3("return 0");
    EXPECT_TRUE(isStatement(s3));
    EXPECT_STREQ("", s3.data());
    // 4. statement is ifStatement
    std::string_view s4("if 2>=1 then let a1<-38;return 0; else let a1<-10;return 1; fi");
    EXPECT_TRUE(isStatement(s4));
    EXPECT_STREQ("", s4.data());
    // 5. statement is whileStatement
    std::string_view s5("while 3>1 do let a1<-38;return 0; od");
    EXPECT_TRUE(isStatement(s5));
    EXPECT_STREQ("", s5.data());
    // 6. invalid test case
    std::string_view s6("a100");
    EXPECT_FALSE(isStatement(s6));
    EXPECT_STREQ("a100", s6.data());
}

TEST(Parser, isStatSequence_check){
    // Single statement with no semicolon
    std::string_view s1("let a1[100]<-38");
    EXPECT_TRUE(isStatSequence(s1));
    EXPECT_STREQ("", s1.data());
    // Single statement with semicolon in the end
    std::string_view s2("let a1[100]<-38;");
    EXPECT_TRUE(isStatSequence(s2));
    EXPECT_STREQ("", s2.data());
    // Multiple statements with semicolon in the end
    std::string_view s3("let a1[100]<-38;return 0;");
    EXPECT_TRUE(isStatSequence(s3));
    EXPECT_STREQ("", s3.data());
    // Multiple statements with no semicolon in the end
    std::string_view s4("let a1[100]<-38;return 0");
    EXPECT_TRUE(isStatSequence(s4));
    EXPECT_STREQ("", s4.data());
}

TEST(Parser, isTypeDecl_check){
    // // One dim
    std::string_view s1("var");
    EXPECT_TRUE(isTypeDecl(s1));
    EXPECT_STREQ("", s1.data());
    // clear_stack();
    
    std::string_view s2("array[100]");
    EXPECT_TRUE(isTypeDecl(s2));
    EXPECT_STREQ("", s2.data());
    // clear_stack();

    // Two dim
    std::string_view s3("array[10][10]");
    EXPECT_TRUE(isTypeDecl(s3));
    EXPECT_STREQ("", s3.data());
    // clear_stack();

    // Invalid test case
    std::string_view s4("var10");
    EXPECT_TRUE(isTypeDecl(s4));
    EXPECT_STREQ("10", s4.data());
    // clear_stack();
}

TEST(Parser, isVarDecl_check){
    // One identifier after typeDecl
    std::string_view s1("vara1;");
    EXPECT_TRUE(isVarDecl(s1));
    EXPECT_STREQ("", s1.data());
    // clear_stack();
    // clear_map();

    // Multiple identifiers after typeDecl
    std::string_view s2("array[100]a1,b2,c3;");
    EXPECT_TRUE(isVarDecl(s2));
    EXPECT_STREQ("", s2.data());
    // clear_stack();
    // clear_map();

    // Invalid test case 1
    std::string_view s3("array[100]>=10;");
    EXPECT_FALSE(isVarDecl(s3));
    EXPECT_STREQ("array[100]>=10;", s3.data());
    // clear_stack();
    // clear_map();

    // Invalid test case 2
    std::string_view s4("array[100]a1,b2,c3");
    EXPECT_FALSE(isVarDecl(s4));
    EXPECT_STREQ("array[100]a1,b2,c3", s4.data());
//     clear_stack();
//     clear_map();
}

TEST(Parser, isFuncDecl_check){
    std::string_view s1("functionf1(a1,a2,b10,c100,z10);{};");
    EXPECT_TRUE(isFuncDecl(s1));
    EXPECT_STREQ("", s1.data());

    std::string_view s2("voidfunctionf1(a1,a2,b10,c100,z10);{let a8<-78;return 0;};");
    EXPECT_TRUE(isFuncDecl(s2));
    EXPECT_STREQ("", s2.data());

    // Invalid test case
    std::string_view s3("voidfunctionf1(a1,a2,b10);{let a8<-78;return 0;}");
    EXPECT_FALSE(isFuncDecl(s3));
    EXPECT_STREQ("voidfunctionf1(a1,a2,b10);{let a8<-78;return 0;}", s3.data());
}

TEST(Parser, isFormalParam_check){
    // No ident
    std::string_view s1("()");
    EXPECT_TRUE(isFormalParam(s1));
    EXPECT_STREQ("", s1.data());
    
    // One ident
    std::string_view s2("(a100)");
    EXPECT_TRUE(isFormalParam(s2));
    EXPECT_STREQ("", s2.data());

    // Multiple idents
    std::string_view s3("(a1,a2,b1,b2)");
    EXPECT_TRUE(isFormalParam(s3));
    EXPECT_STREQ("", s3.data());

    // Invalid test case
    std::string_view s4("(a1,a3");
    EXPECT_FALSE(isFormalParam(s4));
    EXPECT_STREQ("(a1,a3", s4.data());
}

TEST(Parser, isFuncBody_check){
    // Empty function with no name
    std::string_view s1("{}");
    EXPECT_TRUE(isFuncBody(s1));
    EXPECT_STREQ("", s1.data());

    // Empty function with name
    std::string_view s2("vara3;{}");
    EXPECT_TRUE(isFuncBody(s2));
    EXPECT_STREQ("", s2.data());

    // Function with name and statSequence
    std::string_view s3("array[10]f1,f2;{let a8<-78;return 0;}");
    EXPECT_TRUE(isFuncBody(s3));
    EXPECT_STREQ("", s3.data());

    // Invalid test case
    std::string_view s4("array[10]f1,f2;{let a8<-78return 0}");
    EXPECT_FALSE(isFuncBody(s4));
    EXPECT_STREQ("array[10]f1,f2;{let a8<-78return 0}", s4.data());
}

TEST(Parser, isComputation_check){
    // Main funtion with no varDecl and funcDecl
    std::string_view s1("main{let a8<-78;return 0;}.");
    EXPECT_TRUE(isComputation(s1));
    EXPECT_STREQ("", s1.data());

    // Main function with VarDecl
    std::string_view s2("mainvara1;{let a8<-78;return 0;}.");
    EXPECT_TRUE(isComputation(s2));
    EXPECT_STREQ("", s2.data());

    // Main function with VarDecl and funcDecl
    std::string_view s3("mainarray[100]a1;functionf1(a1,a2,b10,c100,z10);{};{let a8<-78;return 0;}.");
    EXPECT_TRUE(isComputation(s3));
    EXPECT_STREQ("", s3.data());

    // Invalid test case
    std::string_view s4("mainarray[100]a1;functionf1(a1,a2,b10,c100,z10);{};{let a8<-78;return 0;");
    EXPECT_FALSE(isComputation(s4));
    EXPECT_STREQ("mainarray[100]a1;functionf1(a1,a2,b10,c100,z10);{};{let a8<-78;return 0;", s4.data());
}