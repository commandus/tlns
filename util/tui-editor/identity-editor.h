#ifndef IDENTITY_EDITOR_H
#define IDENTITY_EDITOR_H       1

#include <string>
#include <vector>

#include "lorawan/storage/service/identity-service-c-wrapper.h"

enum MODE_EDITOR {
    MODE_VIEW = 0,
    MODE_EDIT = 1
};

class IdentityRow {
public:
    std::vector <std::string> row;
    bool changed;
    IdentityRow();
    bool toArray(void *buffer, size_t bufferSize, char** linePtr) const;
};

class IdentityTable {
public:
    void* storage;
    std::string fn;
    size_t recordCount() const;
    void getRows(
        std::vector <IdentityRow> &retVal,
        uint32_t offset,
        size_t count
    ) const;

    int openIdentityStorage(
        C_IDENTITY_SERVICE_IMPL typ,
        const char *aFn
    );

    int closeIdentityStorage();

    IdentityTable();
    ~IdentityTable();

    bool saveRow(const IdentityRow &value) const;
    bool rmRow(const IdentityRow &value) const;
};

class IdentityTableEditor;

class CellEditor {
public:
    int row;
    int col;
    int cursor;
    std::string value;
    std::string cellValue() const;
    void cellValue(const std::string &value);
    CellEditor();
    std::string hotCharValue(char ch) const;
    void setToCurrent(const IdentityTableEditor *editor);

    /**
     * Return true if move outside
     * @param step
     * @return true if move outside
     */
    bool moveCursor(int step);
    void placeChar(char value);
    void insertChar(char value);
    void deleteChar(int step);
};

class IdentityTableEditor {
private:
    int oldCurrentRow;
    int oldCurrentCol;
public:
    IdentityTable *table;
    int datasetOffset;
    int rows;
    int cols;
    int leftColumn;
    bool alwaysShowAddressColumn;
    int currentRow;
    int currentColumn;
    int startCurrentColumn;
    int finishCurrentColumn;
    std::vector <IdentityRow> cellValue;
    CellEditor cellEditor;
    MODE_EDITOR mode;
    bool insertMode;

    IdentityTableEditor();
    ~IdentityTableEditor();
    explicit IdentityTableEditor(
        IdentityTable &table
    );

    size_t maxDataRowsVisible() const;
    size_t dataRowsVisible() const;
    void getRows();
    /**
     * Move cursor
     * @param steps
     * @return return actual step value
     */
    int move2row(int steps);
    std::string strHeader() const;
    std::string strRow(int row);
    /**
     * Return highlighted cells positions
     * set start position of focused cell, if cell is out of screen (not visible), set to -1
     * set finish position of focused cell, if cell is out of screen (not visible), set to -1
     * set finish position of focused cell, if cell is out of screen (not visible), set to -1
     * @param addressStart if address column visible in the screen (also in the case address column is always visible), return start position of the address = 0
     * @param addressFinish if address column visible in the screen (also in the case address column is always visible), return finish position of the address = 8
     * @return true if focused cell visible in the screen
     */
    bool getCurrentColumnStartFinish(
        int &addressStart,
        int &addressFinish
    );

    /**
     * Return value to +/- leftColumn to saveRow focus in the screen
     * @return 0 if selected column is visible
     */
    int moveLeftColumnX2makeFocusVisible() const;
    void printHeader();
    void printRow(int rowIndex);
    void printFooter() const;
    std::string toString() const;
    void draw();
    const std::string& oldCellValue() const;
    bool currentCellChanged() const;
    void saveRow();
    bool rmRow(int index);
    bool move2col(int step);
    bool toggleInsertMode();
    void placeChar(char value);
    void deleteChar(int step);
    void deleteLine(int step);
    bool visibleRowsChanged();
    void save();
    bool cellJumped();
    const std::string &currentCellValue();
    void currentCellValue(const std::string &v);

    // methods to override
    virtual void init();
    virtual void done();
    virtual void gotoXY(int x, int y) const = 0;
    virtual void getScreenSize() = 0;
    virtual void textOut(const std::string &s) const = 0;
    virtual void clearScreen() const = 0;
    virtual void attrOn(uint32_t attr) const = 0;
    virtual void attrOff(uint32_t attr) const = 0;
    virtual void flushScreen() const = 0;
    virtual int readKey() const = 0;
    virtual void view();
    virtual void edit();
    virtual void notify(const std::string &msg) = 0;
};

#endif
