#include <assert.h>
#include "compiler.h"

int E();
void STMT();
void IF();
void BLOCK();

int tempIdx = 0, labelIdx = 0;

#define nextTemp() (tempIdx++)
#define nextLabel() (labelIdx++)
#define emit printf

int isNext(char *set) {
  char eset[SMAX], etoken[SMAX];
  sprintf(eset, " %s ", set);
  sprintf(etoken, " %s ", tokens[tokenIdx]);
  return (tokenIdx < tokenTop && strstr(eset, etoken) != NULL);
}

int isEnd() {
  return tokenIdx >= tokenTop;
}

char *next() {
  // printf("token[%d]=%s\n", tokenIdx, tokens[tokenIdx]);
  return tokens[tokenIdx++];
}

char *skip(char *set) {
  if (isNext(set)) {
    return next();
  } else {
    printf("skip(%s) got %s fail!\n", set, next());
    assert(0);
  }
}

// F = (E) | Number | Id
int F() {
  int f;
  if (isNext("(")) { // '(' E ')'
    next(); // (
    f = E();
    next(); // )
  } else { // Number | Id
    f = nextTemp();
    char *item = next();
    if (isdigit(*item)) {
      emit("t%d = %s\n", f, item);
    } else {
      if (isNext("++")) {
        next();
        emit("%s = %s + 1\n", item, item);
      }
      emit("t%d = %s\n", f, item);
    }
  }
  return f;
}

// E = F (op E)*
int E() {
  int i1 = F();
  while (isNext("+ - * / & | ! < > = <= >= == !=")) {
    char *op = next();
    int i2 = E();
    int i = nextTemp();
    emit("t%d = t%d %s t%d\n", i, i1, op, i2);
    i1 = i;
  }
  return i1;
}

// FOR =  for (ASSIGN EXP; EXP) STMT

void FOR() {
  int forBegin = nextLabel();    // 生成迴圈開始的標籤
  int forEnd = nextLabel();      // 生成迴圈結束的標籤

  // 生成 (Lx) 標籤，表示迴圈開始
  emit("(L%d)\n", forBegin);

  skip("for");  // 讀取 "for"
  skip("(");     // 讀取 "("

  ASSIGN();      // 解析賦值語句 (初始化部分)

  skip(";");     // 讀取 ";"
  int condition = E();  // 解析條件表達式 (迴圈條件)

  emit("if not T%d goto L%d\n", condition, forEnd); // 如果條件不成立，跳到迴圈結束部分

  skip(";");     // 讀取 ";"
  E();           // 解析更新表達式 (迴圈每次執行結束時的更新)

  skip(")");     // 讀取 ")"

  STMT();        // 解析迴圈體 (當條件成立時執行的語句)

  emit("goto L%d\n", forBegin);  // 迴圈結束後跳到迴圈開始部分

  emit("(L%d)\n", forEnd);   // 生成 (Lx) 標籤，表示迴圈結束
}

// ASSIGN = id '=' E;
void ASSIGN() {
  char *id = next();
  skip("=");
  int e = E();
  skip(";");
  emit("%s = t%d\n", id, e);
}

// WHILE = while (E) STMT
void WHILE() {
  int whileBegin = nextLabel();
  int whileEnd = nextLabel();
  emit("(L%d)\n", whileBegin);
  skip("while");
  skip("(");
  int e = E();
  emit("if not T%d goto L%d\n", e, whileEnd);
  skip(")");
  STMT();
  emit("goto L%d\n", whileBegin);
  emit("(L%d)\n", whileEnd);
}

// if (EXP) STMT (else STMT)?
void IF() {
  skip("if");
  skip("(");
  E();
  skip(")");
  STMT();
  if (isNext("else")) {
    skip("else");
    STMT();
  }
}

// DOWHILE = do STMT while (E);
void DOWHILE() {
  int doBegin = nextLabel();   // 生成 do 迴圈開始標籤
  int doEnd = nextLabel();     // 生成 do 迴圈結束標籤
  emit("(L%d)\n", doBegin);    // 生成 do 開始標籤的代碼
  STMT();                      // 執行迴圈體
  skip("while");               // 確保是 "while"
  skip("(");                   // 跳過左括號 "("
  int e = E();                 // 解析條件表達式 E，並返回結果
  emit("if not T%d goto L%d\n", e, doEnd);  // 如果條件為假，跳轉到 do 結束標籤
  skip(")");                   // 跳過右括號 ")"
  emit("goto L%d\n", doBegin); // 無條件跳轉到 do 開始標籤，繼續執行
  emit("(L%d)\n", doEnd);      // 生成 do 結束標籤的代碼
}

// STMT = WHILE | BLOCK | IF | DOWHILE | ASSIGN
void STMT() {
  if (isNext("while"))
    WHILE();
  else if (isNext("do"))
    DOWHILE();
  else if (isNext("if"))
    IF();
  else if (isNext("{"))
    BLOCK();
  else
    ASSIGN();
}

// STMTS = STMT*
void STMTS() {
  while (!isEnd() && !isNext("}")) {
    STMT();
  }
}

// BLOCK = { STMTS }
void BLOCK() {
  skip("{");
  STMTS();
  skip("}");
}

// PROG = STMTS
void PROG() {
  STMTS();
}

void parse() {
  printf("============ parse =============\n");
  tokenIdx = 0;
  PROG();
}