#include <db.h>
#include "curses.h"

#include "curses-identity-editor.h"

int main() {
    IdentityTable table;
    // table.openIdentityStorage(CISI_SQLITE, "test.sqlite.db");
    table.openIdentityStorage(CISI_JSON, "test.json");
    // size_t sz = table.recordCount();
    CursesIdentityTableEditor editor(table);
    editor.init();
    editor.edit();
    // editor.view();
    return 0;
}
