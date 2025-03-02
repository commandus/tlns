#include <sstream>
#include <Windows.h>
#include "curses-identity-editor.h"
#include "curses.h"

#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif

void CursesIdentityTableEditor::init()
{
    IdentityTableEditor::init();
    initscr();
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_YELLOW);
    init_pair(2, COLOR_BLACK, COLOR_WHITE);
    keypad(stdscr, TRUE);
}

void CursesIdentityTableEditor::done()
{
    endwin();
    IdentityTableEditor::done();
}

void CursesIdentityTableEditor::gotoXY(int x, int y) const
{
    move(y, x);
}

void CursesIdentityTableEditor::textOut(const std::string &s) const
{
    printw(s.c_str());
}

void CursesIdentityTableEditor::attrOn(uint32_t attr) const
{
    attron(COLOR_PAIR(attr));
}

void CursesIdentityTableEditor::attrOff(uint32_t attr) const
{
    attroff(COLOR_PAIR(attr));
}

void CursesIdentityTableEditor::clearScreen() const
{
    clear();
}

void CursesIdentityTableEditor::flushScreen() const
{
    refresh();
}

void CursesIdentityTableEditor::getScreenSize() {
    resize_term(0, 0);
    getmaxyx(stdscr, rows, cols);
}

int CursesIdentityTableEditor::readKey() const
{
    return getch();
}

CursesIdentityTableEditor::CursesIdentityTableEditor(
    IdentityTable &table
)
    : IdentityTableEditor(table)
{

}

bool CursesIdentityTableEditor::viewProcessKey(
    int key
) {
    bool r = true;
    switch (key) {
        case KEY_BTAB:
        case KEY_LEFT:
            if (mode == MODE_EDIT)
                return false;
            move2col(-1);
            break;
        case 9:
        case 10:
            move2col(1);
            break;
        case KEY_RIGHT:
            if (mode == MODE_EDIT)
                return false;
            move2col(1);
            break;
        case KEY_HOME:
            move2col(-currentColumn);
            break;
        case KEY_END:
            move2col(12 - currentColumn);
            break;
        case KEY_UP:
            move2row(-1);
            break;
        case KEY_DOWN:
            move2row(1);
            break;
        case KEY_PPAGE:
            move2row(-(int) maxDataRowsVisible());
            break;
        case KEY_NPAGE:
            move2row((int) maxDataRowsVisible());
            break;
        case KEY_F(5):
            getRows();
            break;
        case KEY_RESIZE:
            break;
        default:
            r = false;
    }
    return r;
}

bool CursesIdentityTableEditor::editProcessKey(
    int key
)
{
    std::string hk = cellEditor.hotCharValue((char) key);
    if (!hk.empty()) {
        cellEditor.cellValue(hk);
        auto r = currentCellChanged();
        cellValue[currentRow].changed |= r;
        cellValue[currentRow].row[currentColumn] = cellEditor.cellValue();
        return r;
    }
    switch (key) {
        case KEY_LEFT:
            if (cellEditor.moveCursor(-1))
                return (move2col(-1));
            break;
        case KEY_RIGHT:
            if (cellEditor.moveCursor(1))
                return (move2col(1));
            break;
        case KEY_DC:
            deleteChar(1);
            break;
        case 8:
        case KEY_BACKSPACE:
            deleteChar(-1);
            break;
        case KEY_IC:
            toggleInsertMode();
            break;
        case KEY_F(8):
            deleteLine(0);
            break;
        default:
            placeChar((char) key);
            break;
    }
    return false;
}

void CursesIdentityTableEditor::view() {
    IdentityTableEditor::view();
    bool stopRequest = false;
    while (!stopRequest) {
        auto ch = readKey();
        viewProcessKey(ch);
        switch (ch) {
            case 27:
                stopRequest = true;
                break;
        }
        draw();
    }
}

void CursesIdentityTableEditor::edit() {
    IdentityTableEditor::edit();
    bool stopRequest = false;
    while (!stopRequest) {
        auto ch = readKey();
        if (viewProcessKey(ch) || cellJumped()) {
            // save if changed before lost focus
            save();
        }

        if (!editProcessKey(ch)) {
            switch (ch) {
                case 27:
                    stopRequest = true;
                    break;
            }
        }
        draw();
    }
}

void CursesIdentityTableEditor::notify(
    const std::string &msg
)
{
    printFooter();
    std::stringstream ss;
    ss << ' ' << msg;
    attron(COLOR_PAIR(1));
    textOut(ss.str());
    attroff(COLOR_PAIR(1));
    flushScreen();
    Sleep(1000);
}
