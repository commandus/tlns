#ifndef PDCURSES_IDENTITY_EDITOR_H
#define PDCURSES_IDENTITY_EDITOR_H       1

#include <string>
#include <vector>

#include "identity-editor.h"

class CursesIdentityTableEditor : public IdentityTableEditor {
protected:
    /**
     * Navigate spreadsheet. Return true if cell position changed
     * @param key char
     * @return true if cell position changed
     */
    bool viewProcessKey(int key);
    /**
     * Edit current cell of the spreadsheet, navigate spreadsheet. Return true if cell position changed
     * @param key char
     * @return true if cell position changed
     */
    bool editProcessKey(int key);
public:

    CursesIdentityTableEditor(IdentityTable &table);

    void init() override;
    void done() override;
    void gotoXY(int x, int y) const override;
    void getScreenSize() override;
    void textOut(const std::string &s) const override;
    void attrOn(uint32_t attr) const override;
    void attrOff(uint32_t attr) const override;
    void clearScreen() const override;
    void flushScreen() const override;
    int readKey() const override;
    void view() override;
    void edit() override;
    void notify(const std::string &msg) override;
};

#endif
