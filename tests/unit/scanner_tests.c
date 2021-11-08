#include "../../lib/greatest.h"
#include <stdio.h>
#include <stdbool.h>

#include "../../src/scanner.c"

#define REWRITE_BUFF_FILE(str) ASSERT_EQm("file write failed", rewrite_buffer_file((str)), true);

#define SET_INPUT(str) do {\
  ASSERT_EQm("file write failed", rewrite_buffer_file((str)), true);\
  freopen("tests/unit/scanner_input_files/buffer_file.txt", "r", stdin);\
} while(0)

bool rewrite_buffer_file(char *str) {
  FILE *f = fopen("tests/unit/scanner_input_files/buffer_file.txt", "w");
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


TEST get_keyword_type_from_string_true_test() {
  dynstr_t str_buffer;
  // BAD! only for tests. Otherwise should not access the dynstr struct directly
  // It works only if the function doesn't need a valid dynstr, but only needs a valid c string
  /* TODO(filip): implement dynstr_set_string to overwrite the string with proper reallocation, ... */
  str_buffer.str = malloc(sizeof(char) * 200);
  if (str_buffer.str == NULL) {
    FAILm("str buffer alloc failed in get_keyword_type_from_string_true_test");
  }


  /* scanner_get_keyword_type */
  strcpy(str_buffer.str, "local");
  ASSERT_EQ(TT_K_LOCAL, scanner_get_keyword_type(&str_buffer));

  strcpy(str_buffer.str, "integer");
  ASSERT_EQ(TT_K_INTEGER, scanner_get_keyword_type(&str_buffer));

  strcpy(str_buffer.str, "number");
  ASSERT_EQ(TT_K_NUMBER, scanner_get_keyword_type(&str_buffer));

  strcpy(str_buffer.str, "if");
  ASSERT_EQ(TT_K_IF, scanner_get_keyword_type(&str_buffer));

  strcpy(str_buffer.str, "then");
  ASSERT_EQ(TT_K_THEN, scanner_get_keyword_type(&str_buffer));

  strcpy(str_buffer.str, "else");
  ASSERT_EQ(TT_K_ELSE, scanner_get_keyword_type(&str_buffer));

  strcpy(str_buffer.str, "do");
  ASSERT_EQ(TT_K_DO, scanner_get_keyword_type(&str_buffer));

  strcpy(str_buffer.str, "while");
  ASSERT_EQ(TT_K_WHILE, scanner_get_keyword_type(&str_buffer));

  strcpy(str_buffer.str, "string");
  ASSERT_EQ(TT_K_STRING, scanner_get_keyword_type(&str_buffer));

  strcpy(str_buffer.str, "end");
  ASSERT_EQ(TT_K_END, scanner_get_keyword_type(&str_buffer));

  strcpy(str_buffer.str, "function");
  ASSERT_EQ(TT_K_FUNCTION, scanner_get_keyword_type(&str_buffer));

  strcpy(str_buffer.str, "global");
  ASSERT_EQ(TT_K_GLOBAL, scanner_get_keyword_type(&str_buffer));

  strcpy(str_buffer.str, "nil");
  ASSERT_EQ(TT_K_NIL, scanner_get_keyword_type(&str_buffer));

  strcpy(str_buffer.str, "return");
  ASSERT_EQ(TT_K_RETURN, scanner_get_keyword_type(&str_buffer));

  strcpy(str_buffer.str, "require");
  ASSERT_EQ(TT_K_REQUIRE, scanner_get_keyword_type(&str_buffer));

  free(str_buffer.str);
  PASS();
}

TEST get_keyword_type_from_string_false_test() {
  dynstr_t str_buffer;
  // BAD! only for tests. Otherwise should not access the dynstr struct directly
  // It works only if the function doesn't need a valid dynstr, but only needs a valid c string
  /* TODO(filip): implement dynstr_set_string to overwrite the string with proper reallocation, ... */
  str_buffer.str = malloc(sizeof(char) * 200);
  if (str_buffer.str == NULL) {
    FAILm("str buffer alloc failed in get_keyword_type_from_string_false_test");
  }


  strcpy(str_buffer.str, "integer0");
  ASSERT_EQ(TT_ID, scanner_get_keyword_type(&str_buffer));

  strcpy(str_buffer.str, "_");
  ASSERT_EQ(TT_ID, scanner_get_keyword_type(&str_buffer));

  strcpy(str_buffer.str, "name");
  ASSERT_EQ(TT_ID, scanner_get_keyword_type(&str_buffer));

  strcpy(str_buffer.str, "name123");
  ASSERT_EQ(TT_ID, scanner_get_keyword_type(&str_buffer));

  strcpy(str_buffer.str, "then_");
  ASSERT_EQ(TT_ID, scanner_get_keyword_type(&str_buffer));

  strcpy(str_buffer.str, "ifelse");
  ASSERT_EQ(TT_ID, scanner_get_keyword_type(&str_buffer));

  strcpy(str_buffer.str, "do_this_");
  ASSERT_EQ(TT_ID, scanner_get_keyword_type(&str_buffer));

  strcpy(str_buffer.str, "while32");
  ASSERT_EQ(TT_ID, scanner_get_keyword_type(&str_buffer));

  strcpy(str_buffer.str, "STRING");
  ASSERT_EQ(TT_ID, scanner_get_keyword_type(&str_buffer));

  strcpy(str_buffer.str, "END");
  ASSERT_EQ(TT_ID, scanner_get_keyword_type(&str_buffer));

  free(str_buffer.str);
  PASS();
}

SUITE(scanner_keyword_tests) {
  RUN_TEST(get_keyword_type_from_string_true_test);
  RUN_TEST(get_keyword_type_from_string_false_test);
}

// parameterized standalone token test
enum greatest_test_res param_single_tok_test(char *in, char *attr, token_type_t expected_type) {
  SET_INPUT(in);
  token_t *tok = scanner_get_next_token();
  fclose(stdin);
  ASSERT_EQ(tok->type, expected_type);
  if (attr != NULL) {
    ASSERT_NEQ(NULL, tok->attribute);
    ASSERT_STR_EQ(attr, tok->attribute);
  }
  scanner_token_destroy(tok);

  PASS();
}

// parameterized token test to be used multiple times on the same file
enum greatest_test_res param_tok_test(char *attr, token_type_t expected_type) {
  token_t *tok = scanner_get_next_token();
  char msg[100] = {'\0'};
  if (tok->attribute == NULL && attr != NULL) {
    sprintf(msg, "expected_type: %d, got: %d, expected_attr: %s\n", expected_type, tok->type, attr);
  }
  else {
    sprintf(msg, "expected_type: %d, got: %d, attr: %s\n", expected_type, tok->type, tok->attribute);
  }
  ASSERT_EQm(msg, tok->type, expected_type);
  if (attr != NULL) {
    ASSERT_NEQ(NULL, tok->attribute);
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
  CHECK_CALL(param_single_tok_test("ahoj", "ahoj", TT_ID));

  CHECK_CALL(param_single_tok_test("_ahoj", "_ahoj", TT_ID));

  CHECK_CALL(param_single_tok_test("\n  ahoj", "ahoj", TT_ID));

  CHECK_CALL(param_single_tok_test("  integer", NULL, TT_K_INTEGER));

  CHECK_CALL(param_single_tok_test("\t\t\t\t\r   function", NULL, TT_K_FUNCTION));

  CHECK_CALL(param_single_tok_test("this_is_a_function()", "this_is_a_function", TT_ID));

  CHECK_CALL(param_single_tok_test("\n  foo0011_another_function(void)\n", "foo0011_another_function", TT_ID));

  CHECK_CALL(param_single_tok_test("global func", NULL, TT_K_GLOBAL));

  CHECK_CALL(param_single_tok_test("\t_66func\"", "_66func", TT_ID));

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

TEST one_char_possible_other_test() {
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
  RUN_TEST(one_char_possible_other_test);
  RUN_TEST(multi_char_op_test);
  RUN_TEST(comments_correct_test);
}

// whole program tests
TEST input_file_1_test() {
  freopen("tests/unit/scanner_input_files/test_in_fac.tl", "r", stdin);

  CHECK_CALL(param_tok_test(NULL, TT_K_REQUIRE));
  CHECK_CALL(param_tok_test("\"ifj21\"", TT_STRING));


  CHECK_CALL(param_tok_test(NULL, TT_K_FUNCTION));
  CHECK_CALL(param_tok_test("main", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));

  CHECK_CALL(param_tok_test(NULL, TT_K_LOCAL));
  CHECK_CALL(param_tok_test("a", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COLON));
  CHECK_CALL(param_tok_test(NULL, TT_K_INTEGER ));

  CHECK_CALL(param_tok_test(NULL, TT_K_LOCAL));
  CHECK_CALL(param_tok_test("vysl", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COLON));
  CHECK_CALL(param_tok_test(NULL, TT_K_INTEGER));
  CHECK_CALL(param_tok_test(NULL, TT_ASSIGN));
  CHECK_CALL(param_tok_test("0", TT_INTEGER));

  CHECK_CALL(param_tok_test("write", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test("\"Zadejte cislo pro vypocet faktorialu\\n\"", TT_STRING));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));

  CHECK_CALL(param_tok_test("a", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_ASSIGN));
  CHECK_CALL(param_tok_test("readi", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));

  CHECK_CALL(param_tok_test(NULL, TT_K_IF));
  CHECK_CALL(param_tok_test("a", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COP_EQ));
  CHECK_CALL(param_tok_test(NULL, TT_K_NIL));
  CHECK_CALL(param_tok_test(NULL, TT_K_THEN));

  CHECK_CALL(param_tok_test("write", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test("\"a je nil\\n\"", TT_STRING));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));
  CHECK_CALL(param_tok_test(NULL, TT_K_RETURN));

  CHECK_CALL(param_tok_test(NULL, TT_K_ELSE));
  CHECK_CALL(param_tok_test(NULL, TT_K_END));

  CHECK_CALL(param_tok_test(NULL, TT_K_IF));
  CHECK_CALL(param_tok_test("a", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COP_LT));
  CHECK_CALL(param_tok_test("0", TT_INTEGER));
  CHECK_CALL(param_tok_test(NULL, TT_K_THEN));

  CHECK_CALL(param_tok_test("write", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test("\"Faktorial nelze spocitat\\n\"", TT_STRING));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));

  CHECK_CALL(param_tok_test(NULL, TT_K_ELSE));
  CHECK_CALL(param_tok_test("vysl", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_ASSIGN));
  CHECK_CALL(param_tok_test("1", TT_INTEGER));

  CHECK_CALL(param_tok_test(NULL, TT_K_WHILE));
  CHECK_CALL(param_tok_test("a", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COP_GT));
  CHECK_CALL(param_tok_test("0", TT_INTEGER));
  CHECK_CALL(param_tok_test(NULL, TT_K_DO));

  CHECK_CALL(param_tok_test("vysl", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_ASSIGN));
  CHECK_CALL(param_tok_test("vysl", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_MOP_MUL));
  CHECK_CALL(param_tok_test("a", TT_ID));

  CHECK_CALL(param_tok_test("a", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_ASSIGN));
  CHECK_CALL(param_tok_test("a", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_MOP_MINUS));
  CHECK_CALL(param_tok_test("1", TT_INTEGER));

  CHECK_CALL(param_tok_test(NULL, TT_K_END));

  CHECK_CALL(param_tok_test("write", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test("\"Vysledek je: \"", TT_STRING));
  CHECK_CALL(param_tok_test(NULL, TT_COMMA));
  CHECK_CALL(param_tok_test("vysl", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COMMA));
  CHECK_CALL(param_tok_test("\"\\n\"", TT_STRING));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));

  CHECK_CALL(param_tok_test(NULL, TT_K_END));
  CHECK_CALL(param_tok_test(NULL, TT_K_END));

  CHECK_CALL(param_tok_test("main", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));

  fclose(stdin);
  PASS();
}

TEST input_file_2_test() {
  freopen("tests/unit/scanner_input_files/test_in_fac2.tl", "r", stdin);

  CHECK_CALL(param_tok_test(NULL, TT_K_REQUIRE));
  CHECK_CALL(param_tok_test("\"ifj21\"", TT_STRING));

  CHECK_CALL(param_tok_test(NULL, TT_K_FUNCTION));
  CHECK_CALL(param_tok_test("factorial", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test("n", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COLON));
  CHECK_CALL(param_tok_test(NULL, TT_K_INTEGER));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));
  CHECK_CALL(param_tok_test(NULL, TT_COLON));
  CHECK_CALL(param_tok_test(NULL, TT_K_INTEGER));

  CHECK_CALL(param_tok_test(NULL, TT_K_LOCAL));
  CHECK_CALL(param_tok_test("n1", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COLON));
  CHECK_CALL(param_tok_test(NULL, TT_K_INTEGER));
  CHECK_CALL(param_tok_test(NULL, TT_ASSIGN));
  CHECK_CALL(param_tok_test("n", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_MOP_MINUS));
  CHECK_CALL(param_tok_test("1", TT_INTEGER));

  CHECK_CALL(param_tok_test(NULL, TT_K_IF));
  CHECK_CALL(param_tok_test("n", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COP_LT));
  CHECK_CALL(param_tok_test("2", TT_INTEGER));
  CHECK_CALL(param_tok_test(NULL, TT_K_THEN));
  CHECK_CALL(param_tok_test(NULL, TT_K_RETURN));
  CHECK_CALL(param_tok_test("1", TT_INTEGER));

  CHECK_CALL(param_tok_test(NULL, TT_K_ELSE));
  CHECK_CALL(param_tok_test(NULL, TT_K_LOCAL));
  CHECK_CALL(param_tok_test("tmp", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COLON));
  CHECK_CALL(param_tok_test(NULL, TT_K_INTEGER));
  CHECK_CALL(param_tok_test(NULL, TT_ASSIGN));
  CHECK_CALL(param_tok_test("factorial", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test("n1", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));

  CHECK_CALL(param_tok_test(NULL, TT_K_RETURN));
  CHECK_CALL(param_tok_test("n", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_MOP_MUL));
  CHECK_CALL(param_tok_test("tmp", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_K_END));
  CHECK_CALL(param_tok_test(NULL, TT_K_END));

  CHECK_CALL(param_tok_test(NULL, TT_K_FUNCTION));
  CHECK_CALL(param_tok_test("main", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));

  CHECK_CALL(param_tok_test("write", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test("\"Zadejte cislo pro vypocet faktorialu: \"", TT_STRING));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));

  CHECK_CALL(param_tok_test(NULL, TT_K_LOCAL));
  CHECK_CALL(param_tok_test("a", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COLON));
  CHECK_CALL(param_tok_test(NULL, TT_K_INTEGER));
  CHECK_CALL(param_tok_test(NULL, TT_ASSIGN));
  CHECK_CALL(param_tok_test("readi", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));


  CHECK_CALL(param_tok_test(NULL, TT_K_IF));
  CHECK_CALL(param_tok_test("a", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COP_NEQ));
  CHECK_CALL(param_tok_test(NULL, TT_K_NIL));
  CHECK_CALL(param_tok_test(NULL, TT_K_THEN));

  CHECK_CALL(param_tok_test(NULL, TT_K_IF));
  CHECK_CALL(param_tok_test("a", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COP_LT));
  CHECK_CALL(param_tok_test("0", TT_INTEGER));
  CHECK_CALL(param_tok_test(NULL, TT_K_THEN));

  CHECK_CALL(param_tok_test("write", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test("\"Faktorial nejde spocitat!\"", TT_STRING));
  CHECK_CALL(param_tok_test(NULL, TT_COMMA));
  CHECK_CALL(param_tok_test("\"\\n\"", TT_STRING));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));

  CHECK_CALL(param_tok_test(NULL, TT_K_ELSE));
  CHECK_CALL(param_tok_test(NULL, TT_K_LOCAL));
  CHECK_CALL(param_tok_test("vysl", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COLON));
  CHECK_CALL(param_tok_test(NULL, TT_K_INTEGER));
  CHECK_CALL(param_tok_test(NULL, TT_ASSIGN));
  CHECK_CALL(param_tok_test("factorial", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test("a", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));

  CHECK_CALL(param_tok_test("write", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test("\"Vysledek je \"", TT_STRING));
  CHECK_CALL(param_tok_test(NULL, TT_COMMA));
  CHECK_CALL(param_tok_test("vysl", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COMMA));
  CHECK_CALL(param_tok_test("\"\\n\"", TT_STRING));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));


  CHECK_CALL(param_tok_test(NULL, TT_K_END));
  CHECK_CALL(param_tok_test(NULL, TT_K_ELSE));

  CHECK_CALL(param_tok_test("write", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test("\"Chyba pri nacitani celeho cisla!\\n\"", TT_STRING));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));

  CHECK_CALL(param_tok_test(NULL, TT_K_END));
  CHECK_CALL(param_tok_test(NULL, TT_K_END));

  CHECK_CALL(param_tok_test("main", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));

  fclose(stdin);
  PASS();
}

TEST input_file_3_test() {
  freopen("tests/unit/scanner_input_files/test_in_ws_strings.tl", "r", stdin);

  CHECK_CALL(param_tok_test(NULL, TT_K_REQUIRE));
  CHECK_CALL(param_tok_test("\"ifj21\"", TT_STRING));

  CHECK_CALL(param_tok_test(NULL, TT_K_FUNCTION));
  CHECK_CALL(param_tok_test("main", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));

  CHECK_CALL(param_tok_test(NULL, TT_K_LOCAL));
  CHECK_CALL(param_tok_test("s1", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COLON));
  CHECK_CALL(param_tok_test(NULL, TT_K_STRING));
  CHECK_CALL(param_tok_test(NULL, TT_ASSIGN));
  CHECK_CALL(param_tok_test("\"Toto je nejaky text\"", TT_STRING));

  CHECK_CALL(param_tok_test(NULL, TT_K_LOCAL));
  CHECK_CALL(param_tok_test("s2", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COLON));
  CHECK_CALL(param_tok_test(NULL, TT_K_STRING));
  CHECK_CALL(param_tok_test(NULL, TT_ASSIGN));
  CHECK_CALL(param_tok_test("s1", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_SOP_CONCAT));
  CHECK_CALL(param_tok_test("\", ktery jeste trochu obohatime\"", TT_STRING));

  CHECK_CALL(param_tok_test("write", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test("s1", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COMMA));
  CHECK_CALL(param_tok_test("\"\\010\"", TT_STRING));
  CHECK_CALL(param_tok_test(NULL, TT_COMMA));
  CHECK_CALL(param_tok_test("s2", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));

  CHECK_CALL(param_tok_test(NULL, TT_K_LOCAL));
  CHECK_CALL(param_tok_test("s1len", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COLON));
  CHECK_CALL(param_tok_test(NULL, TT_K_INTEGER));
  CHECK_CALL(param_tok_test(NULL, TT_ASSIGN));
  CHECK_CALL(param_tok_test(NULL, TT_SOP_LENGTH));
  CHECK_CALL(param_tok_test("s1", TT_ID));

  CHECK_CALL(param_tok_test(NULL, TT_K_LOCAL));
  CHECK_CALL(param_tok_test("s1len4", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COLON));
  CHECK_CALL(param_tok_test(NULL, TT_K_INTEGER));
  CHECK_CALL(param_tok_test(NULL, TT_ASSIGN));
  CHECK_CALL(param_tok_test("s1len", TT_ID));

  CHECK_CALL(param_tok_test("s1len", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_ASSIGN));
  CHECK_CALL(param_tok_test("s1len", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_MOP_MINUS));
  CHECK_CALL(param_tok_test("4", TT_INTEGER));

  CHECK_CALL(param_tok_test("s1", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_ASSIGN));
  CHECK_CALL(param_tok_test("substr", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test("s2", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COMMA));
  CHECK_CALL(param_tok_test("s1len", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COMMA));
  CHECK_CALL(param_tok_test("s1len4", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));

  CHECK_CALL(param_tok_test("s1len", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_ASSIGN));
  CHECK_CALL(param_tok_test("s1len", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_MOP_PLUS));
  CHECK_CALL(param_tok_test("1", TT_INTEGER));


  CHECK_CALL(param_tok_test("write", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test("\"4 znaky od\"", TT_STRING));
  CHECK_CALL(param_tok_test(NULL, TT_COMMA));
  CHECK_CALL(param_tok_test("s1len", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COMMA));
  CHECK_CALL(param_tok_test("\". znaku v \\\"\"", TT_STRING));
  CHECK_CALL(param_tok_test(NULL, TT_COMMA));
  CHECK_CALL(param_tok_test("s2", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COMMA));
  CHECK_CALL(param_tok_test("\"\\\":\"", TT_STRING));
  CHECK_CALL(param_tok_test(NULL, TT_COMMA));
  CHECK_CALL(param_tok_test("s1", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COMMA));
  CHECK_CALL(param_tok_test("\"\\n\"", TT_STRING));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));

  CHECK_CALL(param_tok_test("write", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test("\"Zadejte serazenou posloupnost vsech malych pismen a-h, \"", TT_STRING));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));

  CHECK_CALL(param_tok_test("write", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test("\"pricemz se pismena nesmeji v posloupnosti opakovat: \"", TT_STRING));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));

  CHECK_CALL(param_tok_test("s1", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_ASSIGN));
  CHECK_CALL(param_tok_test("reads", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));

  CHECK_CALL(param_tok_test(NULL, TT_K_IF));
  CHECK_CALL(param_tok_test("s1", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COP_NEQ));
  CHECK_CALL(param_tok_test(NULL, TT_K_NIL));
  CHECK_CALL(param_tok_test(NULL, TT_K_THEN));

  CHECK_CALL(param_tok_test(NULL, TT_K_WHILE));
  CHECK_CALL(param_tok_test("s1", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_COP_NEQ));
  CHECK_CALL(param_tok_test("\"abcdefgh\"", TT_STRING));
  CHECK_CALL(param_tok_test(NULL, TT_K_DO));

  CHECK_CALL(param_tok_test("write", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test("\"\\n\"", TT_STRING));
  CHECK_CALL(param_tok_test(NULL, TT_COMMA));
  CHECK_CALL(param_tok_test("\"Spatne zadana posloupnost, zkuste znovu:\"", TT_STRING));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));

  CHECK_CALL(param_tok_test("s1", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_ASSIGN));
  CHECK_CALL(param_tok_test("reads", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));

  CHECK_CALL(param_tok_test(NULL, TT_K_END));
  CHECK_CALL(param_tok_test(NULL, TT_K_ELSE));
  CHECK_CALL(param_tok_test(NULL, TT_K_END));
  CHECK_CALL(param_tok_test(NULL, TT_K_END));

  CHECK_CALL(param_tok_test("main", TT_ID));
  CHECK_CALL(param_tok_test(NULL, TT_LPAR));
  CHECK_CALL(param_tok_test(NULL, TT_RPAR));

  fclose(stdin);
  PASS();
}

SUITE(scanner_input_file_tests) {
  GREATEST_SET_SETUP_CB(start_scanner, NULL);
  GREATEST_SET_TEARDOWN_CB(end_scanner, NULL);


  RUN_TEST(input_file_1_test);
  RUN_TEST(input_file_2_test);
  RUN_TEST(input_file_3_test);
}

