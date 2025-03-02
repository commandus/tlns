#include <cassert>
#include <sstream>
#include <iostream>
#include "curses-identity-editor.h"

static void testDown()
{
    IdentityTable table;
    // table.openIdentityStorage(CISI_SQLITE, "test.sqlite.db");
    table.openIdentityStorage(CISI_JSON, "test.json");
    CursesIdentityTableEditor editor(table);

    editor.rows = 10;
    editor.cols = 80;
    editor.init();

    std::cout << editor.toString() << std::endl;
    assert(editor.toString() == "10x80 1:8/63 @1,1 L:1 INS");

    editor.move2row(1);
    std::cout << editor.toString() << std::endl;
    assert(editor.toString() == "10x80 1:8/63 @2,1 L:1 INS");

    editor.move2row(6);
    std::cout << editor.toString() << std::endl;
    assert(editor.toString() == "10x80 1:8/63 @8,1 L:1 INS");

    editor.move2row(-9);
    std::cout << editor.toString() << std::endl;
    assert(editor.toString() == "10x80 1:8/63 @1,1 L:1 INS");

    editor.move2col(2);
    editor.cellEditor.cellValue("C");
    editor.saveRow();
    std::cout << editor.toString() << std::endl;
    // assert(editor.toString() == "10x80 7/63 @0,2 L:1 INS");
    editor.move2row(60);
    std::cout << editor.toString() << std::endl;
    assert(editor.toString() == "10x80 57:63/63 @61,3 L:1 INS");

    editor.move2col(1);
    std::cout << editor.toString() << std::endl;
    assert(editor.toString() == "10x80 57:63/63 @61,4 L:1 INS");

    editor.move2col(1);
    std::cout << editor.toString() << std::endl;
    assert(editor.toString() == "10x80 57:63/63 @61,5 L:1 INS");

    editor.move2col(1);
    std::cout << editor.toString() << std::endl;
    assert(editor.toString() == "10x80 57:63/63 @61,6 L:4 INS");

    editor.move2col(1);
    std::cout << editor.toString() << std::endl;
    assert(editor.toString() == "10x80 57:63/63 @61,7 L:5 INS");

    editor.move2col(1);
    std::cout << editor.toString() << std::endl;
    assert(editor.toString() == "10x80 57:63/63 @61,8 L:5 INS");
}

static void testTail()
{
    IdentityTable table;
    // table.openIdentityStorage(CISI_SQLITE, "test.sqlite.db");
    table.openIdentityStorage(CISI_JSON, "test.json");
    CursesIdentityTableEditor editor(table);

    editor.rows = 10;
    editor.cols = 80;
    editor.init();

    editor.move2row(60);
    std::cout << editor.toString() << std::endl;
    assert(editor.toString() == "10x80 57:63/63 @61,1 L:1 INS");

    editor.move2col(7);
    std::cout << editor.toString() << std::endl;
    assert(editor.toString() == "10x80 57:63/63 @61,8 L:5 INS");
}

static void testSave()
{
    IdentityTable table;
    // table.openIdentityStorage(CISI_SQLITE, "test.sqlite.db");
    table.openIdentityStorage(CISI_JSON, "test.json");
    CursesIdentityTableEditor editor(table);

    editor.rows = 10;
    editor.cols = 80;
    editor.init();

    editor.move2row(60);
    editor.move2col(2);
    editor.currentCellValue("C");
    // editor.move2col(1);
    editor.move2row(-30);
}

int main() {
    // testDown();
    // testTail();
    testSave();
    return 0;
}
