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
enum greatest_test_res param_single_tok_test(char *in, char *a_str, int a_int, double a_num, token_type_t expected_type) {
  SET_INPUT(in);
  token_t *tok = scanner_get_next_token();
  fclose(stdin);
  ASSERT_EQ(tok->type, expected_type);
  if (a_str != NULL) { // if a_str != NULL, we expect this token to be of type that has a string as its attribute
    ASSERT_NEQ(NULL, tok->attr.str);
    ASSERT_STR_EQ(a_str, tok->attr.str);
  }
  else if (expected_type == TT_INTEGER) {
    ASSERT_EQ(a_int, tok->attr.int_val);
  }
  else if (expected_type == TT_NUMBER) {
    ASSERT_EQ(a_num, tok->attr.num_val);
  }
  scanner_token_destroy(tok);

  PASS();
}

enum greatest_test_res param_string_tok_test(char *in, char *expected_str) {
  SET_INPUT(in);
  token_t *tok = scanner_get_next_token();
  fclose(stdin);

  printf("got string: %s\n", tok->attr.str);
  ASSERT_EQ(tok->type, TT_STRING);

  ASSERT_NEQ(expected_str, NULL);
  ASSERT_NEQ(tok->attr.str, NULL);

  ASSERT_STR_EQ(expected_str, tok->attr.str);

  scanner_token_destroy(tok);

  PASS();
}

char* remove_quotes(char *new_buf, char *str) {
  if (str[0] == '\"') {
    int len = strlen(str);
    for (int i = 1; i < len; i++) {
      if (str[i] == '\"' && i == len-1) {
        new_buf[i-1] = '\0';
        break;
      }
      new_buf[i-1] = str[i];
    }
    return new_buf;
  }
  return str;
}

// parameterized token test to be used multiple times on the same file
enum greatest_test_res param_tok_test(char *a_str, int a_int, double a_num, token_type_t expected_type) {
  token_t *tok = scanner_get_next_token();
  char msg[100] = {'\0'};
  if (a_str != NULL && tok->attr.str == NULL) {
    sprintf(msg, "expected_type: %d, got: %d, expected_attr: %s\n", expected_type, tok->type, a_str);
  }
  else if (a_str != NULL) {
    sprintf(msg, "expected_type: %d, got: %d, attr: %s\n", expected_type, tok->type, tok->attr.str);
  }
  else {
    sprintf(msg, "expected_type: %d, got: %d", expected_type, tok->type);

  }
  ASSERT_EQm(msg, tok->type, expected_type);

  char no_qoutes[300] = {'\0'};
  if (a_str != NULL) { // if a_str != NULL, we expect this token to be of type that has a string as its attribute
    ASSERT_NEQ(NULL, tok->attr.str);
    ASSERT_STR_EQ(remove_quotes(no_qoutes, a_str), tok->attr.str);
  }
  else if (expected_type == TT_INTEGER) {
    ASSERT_EQ(a_int, tok->attr.int_val);
  }
  else if (expected_type == TT_NUMBER) {
    ASSERT_EQ(a_num, tok->attr.num_val);
  }
  scanner_token_destroy(tok);

  PASS();
}

// integer tests
TEST integer_correct_test() {
  CHECK_CALL(param_single_tok_test("1a", NULL, 1, 0, TT_INTEGER));

  CHECK_CALL(param_single_tok_test("01", NULL, 01, 0, TT_INTEGER));

  CHECK_CALL(param_single_tok_test("61", NULL, 61, 0, TT_INTEGER));

  CHECK_CALL(param_single_tok_test("96100123", NULL, 96100123, 0, TT_INTEGER));

  CHECK_CALL(param_single_tok_test("1234567890", NULL, 1234567890, 0, TT_INTEGER));

  CHECK_CALL(param_single_tok_test("  1234567890", NULL, 1234567890, 0, TT_INTEGER));

  CHECK_CALL(param_single_tok_test("\n  1234567890\n", NULL, 1234567890, 0, TT_INTEGER));

  CHECK_CALL(param_single_tok_test("4455+", NULL, 4455, 0, TT_INTEGER));

  CHECK_CALL(param_single_tok_test("\t4455)", NULL, 4455, 0, TT_INTEGER));

  CHECK_CALL(param_single_tok_test("\t4455\"", NULL, 4455, 0, TT_INTEGER));

  CHECK_CALL(param_single_tok_test("4455#", NULL, 4455, 0, TT_INTEGER));

  // test eof report
  SET_INPUT("4455");
  token_t *tok = scanner_get_next_token();
  ASSERT_EQ(tok->type, TT_INTEGER);
  ASSERT_EQ(4455, tok->attr.int_val);
  scanner_token_destroy(tok);

  tok = scanner_get_next_token();
  ASSERT_EQ(tok->type, TT_EOF);
  ASSERT_EQ(NULL, tok->attr.str);
  scanner_token_destroy(tok);
  fclose(stdin);
  // end of test of eof

  PASS();
}


// number tests
TEST number_correct_test() {
  CHECK_CALL(param_single_tok_test("1.1", NULL, 0, 1.1, TT_NUMBER));

  CHECK_CALL(param_single_tok_test("01.33", NULL, 0, 01.33, TT_NUMBER));

  CHECK_CALL(param_single_tok_test("61.0001", NULL, 0, 61.0001, TT_NUMBER));

  CHECK_CALL(param_single_tok_test("96100123.959", NULL, 0, 96100123.959, TT_NUMBER));

  CHECK_CALL(param_single_tok_test("1234567890.0987654321", NULL, 0, 1234567890.0987654321, TT_NUMBER));

  CHECK_CALL(param_single_tok_test("  1234567890.5", NULL, 0, 1234567890.5, TT_NUMBER));

  CHECK_CALL(param_single_tok_test("\n  1234567890.0\n", NULL, 0, 1234567890.0, TT_NUMBER));

  CHECK_CALL(param_single_tok_test("4455.4+", NULL, 0, 4455.4, TT_NUMBER));

  CHECK_CALL(param_single_tok_test("\t4455.4)", NULL, 0, 4455.4, TT_NUMBER));

  CHECK_CALL(param_single_tok_test("\t4455.667\"", NULL, 0, 4455.667, TT_NUMBER));

  CHECK_CALL(param_single_tok_test("4455.123#", NULL, 0, 4455.123, TT_NUMBER));

  // exp
  CHECK_CALL(param_single_tok_test("4455e2", NULL, 0, 4455e2, TT_NUMBER));
  CHECK_CALL(param_single_tok_test("4455E2", NULL, 0, 4455e2, TT_NUMBER));

  CHECK_CALL(param_single_tok_test("4455.0e2", NULL, 0, 4455.0e2, TT_NUMBER));
  CHECK_CALL(param_single_tok_test("4455.0E2", NULL, 0, 4455.0e2, TT_NUMBER));

  CHECK_CALL(param_single_tok_test("4455.1e2", NULL, 0, 4455.1e2, TT_NUMBER));
  CHECK_CALL(param_single_tok_test("4455.1343e2", NULL, 0, 4455.1343e2, TT_NUMBER));

  CHECK_CALL(param_single_tok_test("4455e-2", NULL, 0, 4455e-2, TT_NUMBER));
  CHECK_CALL(param_single_tok_test("4455e+2", NULL, 0, 4455e+2, TT_NUMBER));
  CHECK_CALL(param_single_tok_test("4455.09e+2", NULL, 0, 4455.09e+2, TT_NUMBER));
  CHECK_CALL(param_single_tok_test("4455.09e-2", NULL, 0, 4455.09e-2, TT_NUMBER));

  CHECK_CALL(param_single_tok_test("4455e44", NULL, 0, 4455e44, TT_NUMBER));
  CHECK_CALL(param_single_tok_test("4455E44", NULL, 0, 4455e44, TT_NUMBER));

  CHECK_CALL(param_single_tok_test("\n4455e44..", NULL, 0, 4455e44, TT_NUMBER));

  CHECK_CALL(param_single_tok_test("\t  \n223e-123", NULL, 0, 223e-123, TT_NUMBER));

  CHECK_CALL(param_single_tok_test("\t  \n223e-123 ", NULL, 0, 223e-123, TT_NUMBER));

  CHECK_CALL(param_single_tok_test("\t  \n223e-123+", NULL, 0, 223e-123, TT_NUMBER));

  CHECK_CALL(param_single_tok_test("\t  \n223e-123(", NULL, 0, 223e-123, TT_NUMBER));

  CHECK_CALL(param_single_tok_test("\t  \n223.3e-123(", NULL, 0, 223.3e-123, TT_NUMBER));

  CHECK_CALL(param_single_tok_test("\t  \n223e-123ahoj", NULL, 0, 223e-123, TT_NUMBER));


  // test eof report
  SET_INPUT("4455.4");
  token_t *tok = scanner_get_next_token();
  ASSERT_EQ(tok->type, TT_NUMBER);
  ASSERT_EQ(4455.4, tok->attr.num_val);
  scanner_token_destroy(tok);

  tok = scanner_get_next_token();
  ASSERT_EQ(tok->type, TT_EOF);
  ASSERT_EQ(NULL, tok->attr.str);
  scanner_token_destroy(tok);
  fclose(stdin);
  // end of test of eof

  PASS();
}

// kw/id tests
TEST keyword_id_correct_test() {
  CHECK_CALL(param_single_tok_test("ahoj", "ahoj", 0, 0, TT_ID));

  CHECK_CALL(param_single_tok_test("_ahoj", "_ahoj", 0, 0, TT_ID));

  CHECK_CALL(param_single_tok_test("\n  ahoj", "ahoj", 0, 0, TT_ID));

  CHECK_CALL(param_single_tok_test("  integer", NULL, 0, 0, TT_K_INTEGER));

  CHECK_CALL(param_single_tok_test("\t\t\t\t\r   function", NULL, 0, 0, TT_K_FUNCTION));

  CHECK_CALL(param_single_tok_test("this_is_a_function()", "this_is_a_function", 0, 0, TT_ID));

  CHECK_CALL(param_single_tok_test("\n  foo0011_another_function(void)\n", "foo0011_another_function", 0, 0, TT_ID));

  CHECK_CALL(param_single_tok_test("global func", NULL, 0, 0, TT_K_GLOBAL));

  CHECK_CALL(param_single_tok_test("\t_66func\"", "_66func", 0, 0, TT_ID));

  PASS();
}


// string tests
TEST string_correct_test() {
  /* CHECK_CALL(param_single_tok_test("\"\"", "\"\"", TT_STRING)); */

  CHECK_CALL(param_string_tok_test("\"ahoj\"", "ahoj"));

  CHECK_CALL(param_string_tok_test("\"hello there this is a string. :) %$#..-+*/\"", "hello\\032there\\032this\\032is\\032a\\032string.\\032:)\\032%$\\035..-+*/"));

  CHECK_CALL(param_string_tok_test("\"escape one \\\"another string\\\"\"", "escape\\032one\\032\"another\\032string\""));

  CHECK_CALL(param_string_tok_test("   \"newline \\n in a string\"", "newline\\032\\010\\032in\\032a\\032string"));

  CHECK_CALL(param_string_tok_test("\"escape code \\123  \"", "escape\\032code\\032\\123\\032\\032"));

  PASS();
}

TEST string_error_test() {
  CHECK_CALL(param_single_tok_test("\"ahoj", NULL, 0, 0, TT_ERROR));

  CHECK_CALL(param_single_tok_test("\"ahoj\n\"", NULL, 0, 0, TT_ERROR));

  CHECK_CALL(param_single_tok_test("\"ahoj\\a\"", NULL, 0, 0, TT_ERROR));

  PASS();
}

TEST one_char_op_sep_test() {
  CHECK_CALL(param_single_tok_test("+3", NULL, 0, 0, TT_MOP_PLUS));
  CHECK_CALL(param_single_tok_test("  +3", NULL, 0, 0, TT_MOP_PLUS));
  CHECK_CALL(param_single_tok_test("+", NULL, 0, 0, TT_MOP_PLUS));

  CHECK_CALL(param_single_tok_test("*", NULL, 0, 0, TT_MOP_MUL));
  CHECK_CALL(param_single_tok_test("*4", NULL, 0, 0, TT_MOP_MUL));
  CHECK_CALL(param_single_tok_test("\n\t*ahoj", NULL, 0, 0, TT_MOP_MUL));

  CHECK_CALL(param_single_tok_test(":", NULL, 0, 0, TT_COLON));
  CHECK_CALL(param_single_tok_test(" :", NULL, 0, 0, TT_COLON));
  CHECK_CALL(param_single_tok_test(":hello", NULL, 0, 0, TT_COLON));

  CHECK_CALL(param_single_tok_test(",", NULL, 0, 0, TT_COMMA));
  CHECK_CALL(param_single_tok_test(",another item", NULL, 0, 0, TT_COMMA));
  CHECK_CALL(param_single_tok_test(", another item", NULL, 0, 0, TT_COMMA));

  CHECK_CALL(param_single_tok_test("(", NULL, 0, 0, TT_LPAR));
  CHECK_CALL(param_single_tok_test("\n (integer num)", NULL, 0, 0, TT_LPAR));
  CHECK_CALL(param_single_tok_test(")", NULL, 0, 0, TT_RPAR));
  CHECK_CALL(param_single_tok_test(") global", NULL, 0, 0, TT_RPAR));

  CHECK_CALL(param_single_tok_test("#", NULL, 0, 0, TT_SOP_LENGTH));
  CHECK_CALL(param_single_tok_test(" #string_id", NULL, 0, 0, TT_SOP_LENGTH));

  PASS();
}

TEST one_char_possible_other_test() {
  CHECK_CALL(param_single_tok_test("=", NULL, 0, 0, TT_ASSIGN));
  CHECK_CALL(param_single_tok_test("  = 3", NULL, 0, 0, TT_ASSIGN));
  CHECK_CALL(param_single_tok_test("\n  =3", NULL, 0, 0, TT_ASSIGN));
  CHECK_CALL(param_single_tok_test("=\"ahoj\"", NULL, 0, 0, TT_ASSIGN));

  CHECK_CALL(param_single_tok_test(">", NULL, 0, 0, TT_COP_GT));
  CHECK_CALL(param_single_tok_test("\t>3", NULL, 0, 0, TT_COP_GT));
  CHECK_CALL(param_single_tok_test("\t> 4", NULL, 0, 0, TT_COP_GT));
  CHECK_CALL(param_single_tok_test("\t>hhh 4", NULL, 0, 0, TT_COP_GT));

  CHECK_CALL(param_single_tok_test("<", NULL, 0, 0, TT_COP_LT));
  CHECK_CALL(param_single_tok_test("\t<3", NULL, 0, 0, TT_COP_LT));
  CHECK_CALL(param_single_tok_test("\t< 4", NULL, 0, 0, TT_COP_LT));
  CHECK_CALL(param_single_tok_test("\t<hhh 4", NULL, 0, 0, TT_COP_LT));

  CHECK_CALL(param_single_tok_test("/", NULL, 0, 0, TT_MOP_DIV));
  CHECK_CALL(param_single_tok_test(" / 3", NULL, 0, 0, TT_MOP_DIV));
  CHECK_CALL(param_single_tok_test(" /3", NULL, 0, 0, TT_MOP_DIV));
  CHECK_CALL(param_single_tok_test("\n\n\n\n /3.3 ahoj", NULL, 0, 0, TT_MOP_DIV));

  CHECK_CALL(param_single_tok_test("-", NULL, 0, 0, TT_MOP_MINUS));
  CHECK_CALL(param_single_tok_test(" -3", NULL, 0, 0, TT_MOP_MINUS));
  CHECK_CALL(param_single_tok_test(" -.", NULL, 0, 0, TT_MOP_MINUS));
  CHECK_CALL(param_single_tok_test("\n -a", NULL, 0, 0, TT_MOP_MINUS));


  PASS();
}

TEST multi_char_op_test() {
  CHECK_CALL(param_single_tok_test("//", NULL, 0, 0, TT_MOP_INT_DIV));
  CHECK_CALL(param_single_tok_test(" //33", NULL, 0, 0, TT_MOP_INT_DIV));

  CHECK_CALL(param_single_tok_test(">=", NULL, 0, 0, TT_COP_GE));
  CHECK_CALL(param_single_tok_test(">=3", NULL, 0, 0, TT_COP_GE));
  CHECK_CALL(param_single_tok_test(" >= 3", NULL, 0, 0, TT_COP_GE));

  CHECK_CALL(param_single_tok_test("<=", NULL, 0, 0, TT_COP_LE));
  CHECK_CALL(param_single_tok_test("<=3.3", NULL, 0, 0, TT_COP_LE));
  CHECK_CALL(param_single_tok_test("\n<= 3.3", NULL, 0, 0, TT_COP_LE));

  CHECK_CALL(param_single_tok_test("==", NULL, 0, 0, TT_COP_EQ));
  CHECK_CALL(param_single_tok_test("==()", NULL, 0, 0, TT_COP_EQ));
  CHECK_CALL(param_single_tok_test("\t== ()", NULL, 0, 0, TT_COP_EQ));

  CHECK_CALL(param_single_tok_test("~=", NULL, 0, 0, TT_COP_NEQ));
  CHECK_CALL(param_single_tok_test("~=\n", NULL, 0, 0, TT_COP_NEQ));
  CHECK_CALL(param_single_tok_test("\n~=..", NULL, 0, 0, TT_COP_NEQ));
  CHECK_CALL(param_single_tok_test("\r~=ahoj", NULL, 0, 0, TT_COP_NEQ));
  CHECK_CALL(param_single_tok_test("~= ahoj", NULL, 0, 0, TT_COP_NEQ));

  CHECK_CALL(param_single_tok_test("..", NULL, 0, 0, TT_SOP_CONCAT));
  CHECK_CALL(param_single_tok_test("..\"ahoj\"", NULL, 0, 0, TT_SOP_CONCAT));
  CHECK_CALL(param_single_tok_test("   ..\"ahoj\"", NULL, 0, 0, TT_SOP_CONCAT));
  CHECK_CALL(param_single_tok_test("   ..33", NULL, 0, 0, TT_SOP_CONCAT));
  CHECK_CALL(param_single_tok_test("\n \n .. 33", NULL, 0, 0, TT_SOP_CONCAT));

  PASS();
}


// comment tests
TEST comments_correct_test() {
  CHECK_CALL(param_single_tok_test("--comment\n..", NULL, 0, 0, TT_SOP_CONCAT));
  CHECK_CALL(param_single_tok_test("--comment", NULL, 0, 0, TT_EOF));
  CHECK_CALL(param_single_tok_test("--comment\n", NULL, 0, 0, TT_EOF));

  CHECK_CALL(param_single_tok_test("-- 123123123\n33", NULL, 33, 0, TT_INTEGER));
  CHECK_CALL(param_single_tok_test("-- \t\r..$%^_+#@!123123123\n33 44", NULL, 33, 0, TT_INTEGER));


  CHECK_CALL(param_single_tok_test("--[[ \t\r..$%^_+#@!123123123\n33]] 44", NULL, 44, 0, TT_INTEGER));
  CHECK_CALL(param_single_tok_test("--[[] \t\r..$%^_+#@!123123123\n33]]", NULL, 0, 0, TT_EOF));
  CHECK_CALL(param_single_tok_test("--[[] \t\r..$%^_+#@!123123123\n33", NULL, 0, 0, TT_EOF));

  // this is a line comment
  CHECK_CALL(param_single_tok_test("--[ \t\r..$%^_+#@!123123123\n33]] 44", NULL, 33, 0, TT_INTEGER));

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

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_REQUIRE));
  CHECK_CALL(param_tok_test("\"ifj21\"", 0, 0, TT_STRING));


  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_FUNCTION));
  CHECK_CALL(param_tok_test("main", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_LPAR));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_RPAR));

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_LOCAL));
  CHECK_CALL(param_tok_test("a", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COLON));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_INTEGER ));

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_LOCAL));
  CHECK_CALL(param_tok_test("vysl", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COLON));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_INTEGER));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_ASSIGN));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_INTEGER));

  CHECK_CALL(param_tok_test("write", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_LPAR));
  CHECK_CALL(param_tok_test("\"Zadejte cislo pro vypocet faktorialu\\n\"", 0, 0, TT_STRING));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_RPAR));

  CHECK_CALL(param_tok_test("a", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_ASSIGN));
  CHECK_CALL(param_tok_test("readi", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_LPAR));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_RPAR));

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_IF));
  CHECK_CALL(param_tok_test("a", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COP_EQ));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_NIL));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_THEN));

  CHECK_CALL(param_tok_test("write", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_LPAR));
  CHECK_CALL(param_tok_test("\"a je nil\\n\"", 0, 0, TT_STRING));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_RPAR));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_RETURN));

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_ELSE));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_END));

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_IF));
  CHECK_CALL(param_tok_test("a", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COP_LT));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_INTEGER));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_THEN));

  CHECK_CALL(param_tok_test("write", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_LPAR));
  CHECK_CALL(param_tok_test("\"Faktorial nelze spocitat\\n\"", 0, 0, TT_STRING));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_RPAR));

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_ELSE));
  CHECK_CALL(param_tok_test("vysl", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_ASSIGN));
  CHECK_CALL(param_tok_test(NULL, 1, 0, TT_INTEGER));

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_WHILE));
  CHECK_CALL(param_tok_test("a", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COP_GT));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_INTEGER));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_DO));

  CHECK_CALL(param_tok_test("vysl", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_ASSIGN));
  CHECK_CALL(param_tok_test("vysl", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_MOP_MUL));
  CHECK_CALL(param_tok_test("a", 0, 0, TT_ID));

  CHECK_CALL(param_tok_test("a", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_ASSIGN));
  CHECK_CALL(param_tok_test("a", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_MOP_MINUS));
  CHECK_CALL(param_tok_test(NULL, 1, 0, TT_INTEGER));

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_END));

  CHECK_CALL(param_tok_test("write", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_LPAR));
  CHECK_CALL(param_tok_test("\"Vysledek je: \"", 0, 0, TT_STRING));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COMMA));
  CHECK_CALL(param_tok_test("vysl", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COMMA));
  CHECK_CALL(param_tok_test("\"\\n\"", 0, 0, TT_STRING));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_RPAR));

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_END));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_END));

  CHECK_CALL(param_tok_test("main", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_LPAR));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_RPAR));

  fclose(stdin);
  PASS();
}

TEST input_file_2_test() {
  freopen("tests/unit/scanner_input_files/test_in_fac2.tl", "r", stdin);

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_REQUIRE));
  CHECK_CALL(param_tok_test("\"ifj21\"", 0, 0, TT_STRING));

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_FUNCTION));
  CHECK_CALL(param_tok_test("factorial", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_LPAR));
  CHECK_CALL(param_tok_test("n", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COLON));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_INTEGER));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_RPAR));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COLON));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_INTEGER));

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_LOCAL));
  CHECK_CALL(param_tok_test("n1", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COLON));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_INTEGER));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_ASSIGN));
  CHECK_CALL(param_tok_test("n", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_MOP_MINUS));
  CHECK_CALL(param_tok_test(NULL, 1, 0, TT_INTEGER));

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_IF));
  CHECK_CALL(param_tok_test("n", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COP_LT));
  CHECK_CALL(param_tok_test(NULL, 2, 0, TT_INTEGER));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_THEN));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_RETURN));
  CHECK_CALL(param_tok_test(NULL, 1, 0, TT_INTEGER));

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_ELSE));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_LOCAL));
  CHECK_CALL(param_tok_test("tmp", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COLON));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_INTEGER));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_ASSIGN));
  CHECK_CALL(param_tok_test("factorial", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_LPAR));
  CHECK_CALL(param_tok_test("n1", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_RPAR));

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_RETURN));
  CHECK_CALL(param_tok_test("n", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_MOP_MUL));
  CHECK_CALL(param_tok_test("tmp", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_END));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_END));

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_FUNCTION));
  CHECK_CALL(param_tok_test("main", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_LPAR));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_RPAR));

  CHECK_CALL(param_tok_test("write", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_LPAR));
  CHECK_CALL(param_tok_test("\"Zadejte cislo pro vypocet faktorialu: \"", 0, 0, TT_STRING));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_RPAR));

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_LOCAL));
  CHECK_CALL(param_tok_test("a", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COLON));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_INTEGER));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_ASSIGN));
  CHECK_CALL(param_tok_test("readi", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_LPAR));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_RPAR));


  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_IF));
  CHECK_CALL(param_tok_test("a", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COP_NEQ));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_NIL));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_THEN));

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_IF));
  CHECK_CALL(param_tok_test("a", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COP_LT));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_INTEGER));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_THEN));

  CHECK_CALL(param_tok_test("write", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_LPAR));
  CHECK_CALL(param_tok_test("\"Faktorial nejde spocitat!\"", 0, 0, TT_STRING));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COMMA));
  CHECK_CALL(param_tok_test("\"\\n\"", 0, 0, TT_STRING));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_RPAR));

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_ELSE));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_LOCAL));
  CHECK_CALL(param_tok_test("vysl", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COLON));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_INTEGER));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_ASSIGN));
  CHECK_CALL(param_tok_test("factorial", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_LPAR));
  CHECK_CALL(param_tok_test("a", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_RPAR));

  CHECK_CALL(param_tok_test("write", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_LPAR));
  CHECK_CALL(param_tok_test("\"Vysledek je \"", 0, 0, TT_STRING));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COMMA));
  CHECK_CALL(param_tok_test("vysl", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COMMA));
  CHECK_CALL(param_tok_test("\"\\n\"", 0, 0, TT_STRING));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_RPAR));


  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_END));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_ELSE));

  CHECK_CALL(param_tok_test("write", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_LPAR));
  CHECK_CALL(param_tok_test("\"Chyba pri nacitani celeho cisla!\\n\"", 0, 0, TT_STRING));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_RPAR));

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_END));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_END));

  CHECK_CALL(param_tok_test("main", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_LPAR));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_RPAR));

  fclose(stdin);
  PASS();
}

TEST input_file_3_test() {
  freopen("tests/unit/scanner_input_files/test_in_ws_strings.tl", "r", stdin);

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_REQUIRE));
  CHECK_CALL(param_tok_test("\"ifj21\"", 0, 0, TT_STRING));

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_FUNCTION));
  CHECK_CALL(param_tok_test("main", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_LPAR));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_RPAR));

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_LOCAL));
  CHECK_CALL(param_tok_test("s1", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COLON));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_STRING));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_ASSIGN));
  CHECK_CALL(param_tok_test("\"Toto je nejaky text\"", 0, 0, TT_STRING));

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_LOCAL));
  CHECK_CALL(param_tok_test("s2", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COLON));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_STRING));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_ASSIGN));
  CHECK_CALL(param_tok_test("s1", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_SOP_CONCAT));
  CHECK_CALL(param_tok_test("\", ktery jeste trochu obohatime\"", 0, 0, TT_STRING));

  CHECK_CALL(param_tok_test("write", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_LPAR));
  CHECK_CALL(param_tok_test("s1", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COMMA));
  CHECK_CALL(param_tok_test("\"\\010\"", 0, 0, TT_STRING));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COMMA));
  CHECK_CALL(param_tok_test("s2", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_RPAR));

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_LOCAL));
  CHECK_CALL(param_tok_test("s1len", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COLON));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_INTEGER));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_ASSIGN));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_SOP_LENGTH));
  CHECK_CALL(param_tok_test("s1", 0, 0, TT_ID));

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_LOCAL));
  CHECK_CALL(param_tok_test("s1len4", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COLON));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_INTEGER));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_ASSIGN));
  CHECK_CALL(param_tok_test("s1len", 0, 0, TT_ID));

  CHECK_CALL(param_tok_test("s1len", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_ASSIGN));
  CHECK_CALL(param_tok_test("s1len", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_MOP_MINUS));
  CHECK_CALL(param_tok_test(NULL, 4, 0, TT_INTEGER));

  CHECK_CALL(param_tok_test("s1", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_ASSIGN));
  CHECK_CALL(param_tok_test("substr", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_LPAR));
  CHECK_CALL(param_tok_test("s2", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COMMA));
  CHECK_CALL(param_tok_test("s1len", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COMMA));
  CHECK_CALL(param_tok_test("s1len4", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_RPAR));

  CHECK_CALL(param_tok_test("s1len", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_ASSIGN));
  CHECK_CALL(param_tok_test("s1len", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_MOP_PLUS));
  CHECK_CALL(param_tok_test(NULL, 1, 0, TT_INTEGER));


  CHECK_CALL(param_tok_test("write", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_LPAR));
  CHECK_CALL(param_tok_test("\"4 znaky od\"", 0, 0, TT_STRING));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COMMA));
  CHECK_CALL(param_tok_test("s1len", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COMMA));
  CHECK_CALL(param_tok_test("\". znaku v \\\"\"", 0, 0, TT_STRING));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COMMA));
  CHECK_CALL(param_tok_test("s2", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COMMA));
  CHECK_CALL(param_tok_test("\"\\\":\"", 0, 0, TT_STRING));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COMMA));
  CHECK_CALL(param_tok_test("s1", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COMMA));
  CHECK_CALL(param_tok_test("\"\\n\"", 0, 0, TT_STRING));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_RPAR));

  CHECK_CALL(param_tok_test("write", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_LPAR));
  CHECK_CALL(param_tok_test("\"Zadejte serazenou posloupnost vsech malych pismen a-h, \"", 0, 0, TT_STRING));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_RPAR));

  CHECK_CALL(param_tok_test("write", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_LPAR));
  CHECK_CALL(param_tok_test("\"pricemz se pismena nesmeji v posloupnosti opakovat: \"", 0, 0, TT_STRING));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_RPAR));

  CHECK_CALL(param_tok_test("s1", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_ASSIGN));
  CHECK_CALL(param_tok_test("reads", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_LPAR));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_RPAR));

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_IF));
  CHECK_CALL(param_tok_test("s1", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COP_NEQ));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_NIL));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_THEN));

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_WHILE));
  CHECK_CALL(param_tok_test("s1", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COP_NEQ));
  CHECK_CALL(param_tok_test("\"abcdefgh\"", 0, 0, TT_STRING));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_DO));

  CHECK_CALL(param_tok_test("write", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_LPAR));
  CHECK_CALL(param_tok_test("\"\\n\"", 0, 0, TT_STRING));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_COMMA));
  CHECK_CALL(param_tok_test("\"Spatne zadana posloupnost, zkuste znovu:\"", 0, 0, TT_STRING));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_RPAR));

  CHECK_CALL(param_tok_test("s1", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_ASSIGN));
  CHECK_CALL(param_tok_test("reads", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_LPAR));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_RPAR));

  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_END));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_ELSE));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_END));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_K_END));

  CHECK_CALL(param_tok_test("main", 0, 0, TT_ID));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_LPAR));
  CHECK_CALL(param_tok_test(NULL, 0, 0, TT_RPAR));

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

