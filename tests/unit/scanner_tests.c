#include "../../lib/greatest.h"
#include <stdio.h>
#include <stdbool.h>

#include "../../src/scanner_new.h"

#define REWRITE_BUFF_FILE(str) ASSERT_EQm("file write failed", rewrite_buffer_file((str)), true);

#define SET_INPUT(str) ({\
  ASSERT_EQm("file write failed", rewrite_buffer_file((str)), true);\
  freopen("tests/unit/scanner_files/buffer_file.txt", "r", stdin);\
}) 

bool rewrite_buffer_file(char *str) {
  FILE *f = fopen("tests/unit/scanner_files/buffer_file.txt", "w");
  if (f == NULL) {
    return false;
  }

  fprintf(f, "%s", str);

  fclose(f);
  return true;
}

void start_scanner(void *arg) {
  (void)arg;
  scanner_init();
}

void end_scanner(void *arg) {
  (void)arg;
  scanner_destroy();
}

// parameterized standalone token test
enum greatest_test_res param_single_tok_test(char *in, char *attr, token_type_t expected_type) {
  SET_INPUT(in);
  token_t *tok = scanner_get_next_token();
  fclose(stdin);
  ASSERT_EQ(tok->type, expected_type);
  if (attr != NULL) {
    ASSERT_STR_EQ(attr, tok->attribute);
  }
  scanner_token_destroy(tok);

  PASS();
}

// parameterized token test to be used multiple times on the same file
enum greatest_test_res param_tok_test(char *attr, token_type_t expected_type) {
  token_t *tok = scanner_get_next_token();
  char msg[100] = {'\0'};
  sprintf(msg, "expected_type: %d, got: %d\n", expected_type, tok->type);
  ASSERT_EQm(msg, tok->type, expected_type);
  if (attr != NULL) {
    ASSERT_STR_EQ(attr, tok->attribute);
  }
  scanner_token_destroy(tok);

  PASS();
}

// integer tests
TEST integer_correct_test() {
  CHECK_CALL(param_single_tok_test("1a", "1", TT_INTEGER));

  CHECK_CALL(param_single_tok_test("01", "01", TT_INTEGER));

  CHECK_CALL(param_single_tok_test("61", "61", TT_INTEGER));

  CHECK_CALL(param_single_tok_test("96100123", "96100123", TT_INTEGER));
  
  CHECK_CALL(param_single_tok_test("1234567890", "1234567890", TT_INTEGER));

  CHECK_CALL(param_single_tok_test("  1234567890", "1234567890", TT_INTEGER));

  CHECK_CALL(param_single_tok_test("\n  1234567890\n", "1234567890", TT_INTEGER));

  CHECK_CALL(param_single_tok_test("4455+", "4455", TT_INTEGER));

  CHECK_CALL(param_single_tok_test("\t4455)", "4455", TT_INTEGER));

  CHECK_CALL(param_single_tok_test("\t4455\"", "4455", TT_INTEGER));

  CHECK_CALL(param_single_tok_test("4455#", "4455", TT_INTEGER));

  // test eof report
  SET_INPUT("4455");
  token_t *tok = scanner_get_next_token();
  ASSERT_EQ(tok->type, TT_INTEGER);
  ASSERT_STR_EQ("4455", tok->attribute);
  scanner_token_destroy(tok);

  tok = scanner_get_next_token();
  ASSERT_EQ(tok->type, TT_EOF);
  ASSERT_EQ(NULL, tok->attribute);
  scanner_token_destroy(tok);
  fclose(stdin);
  // end of test of eof

  PASS();
}


// number tests
TEST number_correct_test() {
  CHECK_CALL(param_single_tok_test("1.1", "1.1", TT_NUMBER));

  CHECK_CALL(param_single_tok_test("01.33", "01.33", TT_NUMBER));

  CHECK_CALL(param_single_tok_test("61.0001", "61.0001", TT_NUMBER));

  CHECK_CALL(param_single_tok_test("96100123.959", "96100123.959", TT_NUMBER));
  
  CHECK_CALL(param_single_tok_test("1234567890.0987654321", "1234567890.0987654321", TT_NUMBER));

  CHECK_CALL(param_single_tok_test("  1234567890.5", "1234567890.5", TT_NUMBER));

  /* TODO(filip): is this correct? */
  CHECK_CALL(param_single_tok_test("\n  1234567890.0\n", "1234567890.0", TT_NUMBER));

  CHECK_CALL(param_single_tok_test("4455.4+", "4455.4", TT_NUMBER));

  CHECK_CALL(param_single_tok_test("\t4455.4)", "4455.4", TT_NUMBER));

  CHECK_CALL(param_single_tok_test("\t4455.667\"", "4455.667", TT_NUMBER));

  CHECK_CALL(param_single_tok_test("4455.123#", "4455.123", TT_NUMBER));

  // exp
  CHECK_CALL(param_single_tok_test("4455e2", "4455e2", TT_NUMBER));

  CHECK_CALL(param_single_tok_test("4455e-2", "4455e-2", TT_NUMBER));
  CHECK_CALL(param_single_tok_test("4455e+2", "4455e+2", TT_NUMBER));

  CHECK_CALL(param_single_tok_test("4455e4444", "4455e4444", TT_NUMBER));

  CHECK_CALL(param_single_tok_test("\n4455e4444..", "4455e4444", TT_NUMBER));

  CHECK_CALL(param_single_tok_test("\t  \n223e-123", "223e-123", TT_NUMBER));

  CHECK_CALL(param_single_tok_test("\t  \n223e-123 ", "223e-123", TT_NUMBER));

  CHECK_CALL(param_single_tok_test("\t  \n223e-123+", "223e-123", TT_NUMBER));

  CHECK_CALL(param_single_tok_test("\t  \n223e-123(", "223e-123", TT_NUMBER));

  CHECK_CALL(param_single_tok_test("\t  \n223e-123ahoj", "223e-123", TT_NUMBER));


  // test eof report
  SET_INPUT("4455.4");
  token_t *tok = scanner_get_next_token();
  ASSERT_EQ(tok->type, TT_NUMBER);
  ASSERT_STR_EQ("4455.4", tok->attribute);
  scanner_token_destroy(tok);

  tok = scanner_get_next_token();
  ASSERT_EQ(tok->type, TT_EOF);
  ASSERT_EQ(NULL, tok->attribute);
  scanner_token_destroy(tok);
  fclose(stdin);
  // end of test of eof

  PASS();
}

// kw/id tests
TEST keyword_id_correct_test() {
  CHECK_CALL(param_single_tok_test("ahoj", "ahoj", TT_KEYWORD_ID));

  CHECK_CALL(param_single_tok_test("_ahoj", "_ahoj", TT_KEYWORD_ID));

  CHECK_CALL(param_single_tok_test("\n  ahoj", "ahoj", TT_KEYWORD_ID));

  CHECK_CALL(param_single_tok_test("  integer", "integer", TT_KEYWORD_ID));
  
  CHECK_CALL(param_single_tok_test("\t\t\t\t\r   function", "function", TT_KEYWORD_ID));

  CHECK_CALL(param_single_tok_test("this_is_a_function()", "this_is_a_function", TT_KEYWORD_ID));

  CHECK_CALL(param_single_tok_test("\n  foo0011_another_function(void)\n", "foo0011_another_function", TT_KEYWORD_ID));

  CHECK_CALL(param_single_tok_test("global func", "global", TT_KEYWORD_ID));

  CHECK_CALL(param_single_tok_test("\t_66func\"", "_66func", TT_KEYWORD_ID));

  PASS();
}


// string tests
TEST string_correct_test() {
  /* CHECK_CALL(param_single_tok_test("\"\"", "\"\"", TT_STRING)); */

  CHECK_CALL(param_single_tok_test("\"ahoj\"", "\"ahoj\"", TT_STRING));

  CHECK_CALL(param_single_tok_test("\"hello there this is a string. :) %$#..-+*/\"", "\"hello there this is a string. :) %$#..-+*/\"", TT_STRING));

  CHECK_CALL(param_single_tok_test("\"escape one \\\"another string\\\"\"", "\"escape one \\\"another string\\\"\"", TT_STRING));

  CHECK_CALL(param_single_tok_test("   \"newline \\n in a string\"", "\"newline \\n in a string\"", TT_STRING));

  CHECK_CALL(param_single_tok_test("\"escape code \\123  \"", "\"escape code \\123  \"", TT_STRING));

  PASS();
}

TEST string_error_test() {
  CHECK_CALL(param_single_tok_test("\"ahoj", NULL, TT_ERROR));

  CHECK_CALL(param_single_tok_test("\"ahoj\n\"", NULL, TT_ERROR));
  
  CHECK_CALL(param_single_tok_test("\"ahoj\\a\"", NULL, TT_ERROR));


  PASS();
}

TEST one_char_op_sep_test() {
  CHECK_CALL(param_single_tok_test("+3", NULL, TT_MOP_PLUS));
  CHECK_CALL(param_single_tok_test("  +3", NULL, TT_MOP_PLUS));
  CHECK_CALL(param_single_tok_test("+", NULL, TT_MOP_PLUS));

  CHECK_CALL(param_single_tok_test("*", NULL, TT_MOP_MUL));
  CHECK_CALL(param_single_tok_test("*4", NULL, TT_MOP_MUL));
  CHECK_CALL(param_single_tok_test("\n\t*ahoj", NULL, TT_MOP_MUL));

  CHECK_CALL(param_single_tok_test(":", NULL, TT_COLON));
  CHECK_CALL(param_single_tok_test(" :", NULL, TT_COLON));
  CHECK_CALL(param_single_tok_test(":hello", NULL, TT_COLON));

  CHECK_CALL(param_single_tok_test(",", NULL, TT_COMMA));
  CHECK_CALL(param_single_tok_test(",another item", NULL, TT_COMMA));
  CHECK_CALL(param_single_tok_test(", another item", NULL, TT_COMMA));

  CHECK_CALL(param_single_tok_test("(", NULL, TT_LPAR));
  CHECK_CALL(param_single_tok_test("\n (integer num)", NULL, TT_LPAR));
  CHECK_CALL(param_single_tok_test(")", NULL, TT_RPAR));
  CHECK_CALL(param_single_tok_test(") global", NULL, TT_RPAR));

  CHECK_CALL(param_single_tok_test("#", NULL, TT_SOP_LENGTH));
  CHECK_CALL(param_single_tok_test(" #string_id", NULL, TT_SOP_LENGTH));


  PASS();
}

TEST one_char_possible_other() {
  CHECK_CALL(param_single_tok_test("=", NULL, TT_ASSIGN));
  CHECK_CALL(param_single_tok_test("  = 3", NULL, TT_ASSIGN));
  CHECK_CALL(param_single_tok_test("\n  =3", NULL, TT_ASSIGN));
  CHECK_CALL(param_single_tok_test("=\"ahoj\"", NULL, TT_ASSIGN));

  CHECK_CALL(param_single_tok_test(">", NULL, TT_COP_GT));
  CHECK_CALL(param_single_tok_test("\t>3", NULL, TT_COP_GT));
  CHECK_CALL(param_single_tok_test("\t> 4", NULL, TT_COP_GT));
  CHECK_CALL(param_single_tok_test("\t>hhh 4", NULL, TT_COP_GT));

  CHECK_CALL(param_single_tok_test("<", NULL, TT_COP_LT));
  CHECK_CALL(param_single_tok_test("\t<3", NULL, TT_COP_LT));
  CHECK_CALL(param_single_tok_test("\t< 4", NULL, TT_COP_LT));
  CHECK_CALL(param_single_tok_test("\t<hhh 4", NULL, TT_COP_LT));

  CHECK_CALL(param_single_tok_test("/", NULL, TT_MOP_DIV));
  CHECK_CALL(param_single_tok_test(" / 3", NULL, TT_MOP_DIV));
  CHECK_CALL(param_single_tok_test(" /3", NULL, TT_MOP_DIV));
  CHECK_CALL(param_single_tok_test("\n\n\n\n /3.3 ahoj", NULL, TT_MOP_DIV));

  CHECK_CALL(param_single_tok_test("-", NULL, TT_MOP_MINUS));
  CHECK_CALL(param_single_tok_test(" -3", NULL, TT_MOP_MINUS));
  CHECK_CALL(param_single_tok_test(" -.", NULL, TT_MOP_MINUS));
  CHECK_CALL(param_single_tok_test("\n -a", NULL, TT_MOP_MINUS));


  PASS();
}

TEST multi_char_op_test() {
  CHECK_CALL(param_single_tok_test("//", NULL, TT_MOP_INT_DIV));
  CHECK_CALL(param_single_tok_test(" //33", NULL, TT_MOP_INT_DIV));

  CHECK_CALL(param_single_tok_test(">=", NULL, TT_COP_GE));
  CHECK_CALL(param_single_tok_test(">=3", NULL, TT_COP_GE));
  CHECK_CALL(param_single_tok_test(" >= 3", NULL, TT_COP_GE));

  CHECK_CALL(param_single_tok_test("<=", NULL, TT_COP_LE));
  CHECK_CALL(param_single_tok_test("<=3.3", NULL, TT_COP_LE));
  CHECK_CALL(param_single_tok_test("\n<= 3.3", NULL, TT_COP_LE));

  CHECK_CALL(param_single_tok_test("==", NULL, TT_COP_EQ));
  CHECK_CALL(param_single_tok_test("==()", NULL, TT_COP_EQ));
  CHECK_CALL(param_single_tok_test("\t== ()", NULL, TT_COP_EQ));

  CHECK_CALL(param_single_tok_test("~=", NULL, TT_COP_NEQ));
  CHECK_CALL(param_single_tok_test("~=\n", NULL, TT_COP_NEQ));
  CHECK_CALL(param_single_tok_test("\n~=..", NULL, TT_COP_NEQ));
  CHECK_CALL(param_single_tok_test("\r~=ahoj", NULL, TT_COP_NEQ));
  CHECK_CALL(param_single_tok_test("~= ahoj", NULL, TT_COP_NEQ));

  CHECK_CALL(param_single_tok_test("..", NULL, TT_SOP_CONCAT));
  CHECK_CALL(param_single_tok_test("..\"ahoj\"", NULL, TT_SOP_CONCAT));
  CHECK_CALL(param_single_tok_test("   ..\"ahoj\"", NULL, TT_SOP_CONCAT));
  CHECK_CALL(param_single_tok_test("   ..33", NULL, TT_SOP_CONCAT));
  CHECK_CALL(param_single_tok_test("\n \n .. 33", NULL, TT_SOP_CONCAT));

  PASS();
}


// comment tests
TEST comments_correct_test() {
  CHECK_CALL(param_single_tok_test("--comment\n..", NULL, TT_SOP_CONCAT));
  CHECK_CALL(param_single_tok_test("--comment", NULL, TT_EOF));

  CHECK_CALL(param_single_tok_test("-- 123123123\n33", "33", TT_INTEGER));
  CHECK_CALL(param_single_tok_test("-- \t\r..$%^_+#@!123123123\n33 44", "33", TT_INTEGER));


  CHECK_CALL(param_single_tok_test("--[[ \t\r..$%^_+#@!123123123\n33]] 44", "44", TT_INTEGER));
  CHECK_CALL(param_single_tok_test("--[[] \t\r..$%^_+#@!123123123\n33]]", NULL, TT_EOF));
  CHECK_CALL(param_single_tok_test("--[[] \t\r..$%^_+#@!123123123\n33", NULL, TT_EOF));

  // this is a line comment
  CHECK_CALL(param_single_tok_test("--[ \t\r..$%^_+#@!123123123\n33]] 44", "33", TT_INTEGER));

  PASS();
}


SUITE(scanner_basic_tests) {
  GREATEST_SET_SETUP_CB(start_scanner, NULL);
  GREATEST_SET_TEARDOWN_CB(end_scanner, NULL);


  RUN_TEST(integer_correct_test);
  RUN_TEST(number_correct_test);
  RUN_TEST(keyword_id_correct_test);
  RUN_TEST(string_correct_test);
  RUN_TEST(string_error_test);
  RUN_TEST(one_char_op_sep_test);
  RUN_TEST(one_char_possible_other);
  RUN_TEST(multi_char_op_test);
  RUN_TEST(comments_correct_test);
}

// whole program tests
TEST input_file_1_test() {
  freopen("tests/unit/scanner_files/test_in_fac.tl", "r", stdin);

  CHECK_CALL(param_tok_test("require", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test("\"ifj21\"", TT_STRING));

  CHECK_CALL(param_tok_test("function", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test("main", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));

  CHECK_CALL(param_tok_test("local", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test("a", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COLON));
  CHECK_CALL(param_tok_test("integer", TT_KEYWORD_ID));

  CHECK_CALL(param_tok_test("local", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test("vysl", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COLON));
  CHECK_CALL(param_tok_test("integer", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test(NULL, TT_ASSIGN));
  CHECK_CALL(param_tok_test("0", TT_INTEGER));

  CHECK_CALL(param_tok_test("write", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test("\"Zadejte cislo pro vypocet faktorialu\\n\"", TT_STRING));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));

  CHECK_CALL(param_tok_test("a", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test(NULL, TT_ASSIGN));
  CHECK_CALL(param_tok_test("readi", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));

  CHECK_CALL(param_tok_test("if", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test("a", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COP_EQ));
  CHECK_CALL(param_tok_test("nil", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test("then", TT_KEYWORD_ID));

  CHECK_CALL(param_tok_test("write", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test("\"a je nil\\n\"", TT_STRING));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));
  CHECK_CALL(param_tok_test("return", TT_KEYWORD_ID));

  CHECK_CALL(param_tok_test("else", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test("end", TT_KEYWORD_ID));
  
  CHECK_CALL(param_tok_test("if", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test("a", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COP_LT));
  CHECK_CALL(param_tok_test("0", TT_INTEGER));
  CHECK_CALL(param_tok_test("then", TT_KEYWORD_ID));

  CHECK_CALL(param_tok_test("write", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test("\"Faktorial nelze spocitat\\n\"", TT_STRING));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));

  CHECK_CALL(param_tok_test("else", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test("vysl", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test(NULL, TT_ASSIGN));
  CHECK_CALL(param_tok_test("1", TT_INTEGER));

  CHECK_CALL(param_tok_test("while", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test("a", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COP_GT));
  CHECK_CALL(param_tok_test("0", TT_INTEGER));
  CHECK_CALL(param_tok_test("do", TT_KEYWORD_ID));

  CHECK_CALL(param_tok_test("vysl", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test(NULL, TT_ASSIGN));
  CHECK_CALL(param_tok_test("vysl", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test(NULL, TT_MOP_MUL));
  CHECK_CALL(param_tok_test("a", TT_KEYWORD_ID));

  CHECK_CALL(param_tok_test("a", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test(NULL, TT_ASSIGN));
  CHECK_CALL(param_tok_test("a", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test(NULL, TT_MOP_MINUS));
  CHECK_CALL(param_tok_test("1", TT_INTEGER));

  CHECK_CALL(param_tok_test("end", TT_KEYWORD_ID));

  CHECK_CALL(param_tok_test("write", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test("\"Vysledek je: \"", TT_STRING));
  CHECK_CALL(param_tok_test(NULL, TT_COMMA));
  CHECK_CALL(param_tok_test("vysl", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COMMA));
  CHECK_CALL(param_tok_test("\"\\n\"", TT_STRING));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));

  CHECK_CALL(param_tok_test("end", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test("end", TT_KEYWORD_ID));

  CHECK_CALL(param_tok_test("main", TT_KEYWORD_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));

  fclose(stdin);
  PASS();
}


SUITE(scanner_input_file_tests) {
  GREATEST_SET_SETUP_CB(start_scanner, NULL);
  GREATEST_SET_TEARDOWN_CB(end_scanner, NULL);


  RUN_TEST(input_file_1_test);
}

