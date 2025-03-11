#include <sstream>
#include <iomanip>
#include "identity-editor.h"

std::string &trim(std::string &s);

static const std::string EMPTY_STR;

#define COLUMN_COUNT 13

static const int columnWidth[COLUMN_COUNT] {
    8, 4, 1, 16, 32, 32, 5, 16, 32, 32, 4, 6,8
};

IdentityRow::IdentityRow()
    : changed(false)
{

}

bool IdentityRow::toArray(
    void *buf,
    size_t bufferSize,
    char** p
) const
{
    // memset(buf, 0, bufferSize);
    char *pp = (char *) buf;
    size_t cnt = 0;
    for (int i = 0; i < COLUMN_COUNT; i++) {
        std::string v = row[i];
        v = trim(v);
        auto sz = v.size();
        p[i] = pp;
        if (cnt + sz + 1 >= bufferSize)
            return false;
        memmove(pp, v.c_str(), sz);
        pp += sz;
        *pp = '\0';
        pp++;
        cnt += sz + 1;
    }
    return true;
}

CellEditor::CellEditor()
    : row(0), col(0), cursor(0)
{

}

std::string CellEditor::cellValue() const
{
    return value;
}

void CellEditor::cellValue(
    const std::string &val
)
{
    value = val;
    cursor = (int) val.length();
}

std::string CellEditor::hotCharValue(
    char ch
) const
{
    switch (col) {
        case 1:
            switch (ch) {
                case 'A':
                case 'a':
                    return "ABP";
                case 'O':
                case 'o':
                    return "OTAA";
                default:
                    break;
            }
            break;
        case 2:
            switch (ch) {
                case 'A':
                case 'a':
                    return "A";
                case 'B':
                case 'b':
                    return "B";
                case 'C':
                case 'c':
                    return "C";
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return EMPTY_STR;
}

void CellEditor::setToCurrent(
    const IdentityTableEditor *editor
) {
    cellValue(editor->oldCellValue());
    row = editor->currentRow;
    col = editor->currentColumn;
}

bool CellEditor::moveCursor(int step) {
    cursor += step;
    if (cursor < 0) {
        cursor = 0;
        return true;
    }
    if (cursor >= value.size()) {
        cursor = (int) (value.size() - 1);
        return true;
    }
    return false;
}

void CellEditor::placeChar(char ch) {
    if (value.size() > cursor) {
        value[cursor] = ch;
        moveCursor(1);
    }
}

void CellEditor::insertChar(char ch) {
    auto w = columnWidth[col];
    if (cursor >= w)
        return;
    if (cursor < value.size())
        value.insert(cursor, 1, ch);
    else
        value += ch;
    moveCursor(1);
}

void CellEditor::deleteChar(
    int step
) {
    auto len = value.size();
    if (step > 0) {
        if (cursor < len) {
            value.erase(cursor, step);
            value.insert(len - step, step, ' ');
        }
        return;
    }
    if (step < 0) {
        int m = cursor + step;
        if (m < 0)
            return;
        if (m < len + 1) {
            value.erase(m, -step);
            value.insert(value.size(), -step, ' ');
            moveCursor(step);
        }
    }
}


size_t IdentityTable::recordCount() const {
    return c_size(storage);
}

void IdentityTable::getRows(
    std::vector <IdentityRow> &retVal,
    uint32_t offset,
    size_t count
) const {
    auto c = count;
    C_NETWORKIDENTITY ls[256];

    while (c > 0) {
        uint8_t chunkSize = c % 256;
        c -= chunkSize;
        auto cnt = c_list(storage, ls, offset, chunkSize);
        char buffer[512];
        char *lines[COLUMN_COUNT];
        for (int i = 0; i < cnt; i++) {
            c_networkidentity2text(buffer, sizeof(buffer), lines, &ls[i]);
            IdentityRow row;
            for (auto & line : lines) {
                row.row.emplace_back(line);
            }
            retVal.push_back(row);
        }
    }
}

int IdentityTable::openIdentityStorage(
    C_IDENTITY_SERVICE_IMPL typ,
    const char *aFn
) {
    storage = makeIdentityServiceC(typ);
    if (storage) {
        c_init(storage, aFn, nullptr);
        fn = aFn;
    }
    return 0;
}

int IdentityTable::closeIdentityStorage() {
    if (storage) {
        c_done(storage);
        storage = nullptr;
    }
    fn = "";
    return 0;
}

IdentityTable::IdentityTable()
    : storage(nullptr)
{

}

IdentityTable::~IdentityTable()
{
    closeIdentityStorage();
}

bool IdentityTable::saveRow(
    const IdentityRow &value
) const
{
    char buf[512];
    char *p[COLUMN_COUNT];
    value.toArray(buf, sizeof(buf), p);

    C_NETWORKIDENTITY nid;
    text2c_networkidentity(&nid, (const char **) p);
    return c_put(storage, &nid.devaddr, &nid.devid) == 0;
}

bool IdentityTable::rmRow(
    const IdentityRow &value
) const
{
    if (value.row.empty())
        return false;
    C_DEVADDR ca;
    text2c_devaddr(&ca, value.row[0].c_str());
    return c_rm(storage, &ca) == 0;
}

static const char* columnHeader[COLUMN_COUNT] {
    "Address", "Act", "C", "DevEUI", "NwkSKey",
    "AppSKey", "Ver", "AppEUI", "AppKey",
    "NwkKey", "Nonc", "JNonce", "Name"
};

#define DLMT ' '
#define DLMT_SIZE 1

IdentityTableEditor::IdentityTableEditor()
    : oldCurrentRow(0), oldCurrentCol(0), table(nullptr), datasetOffset(), rows(80), cols(25), leftColumn(1), alwaysShowAddressColumn(true),
      currentRow(0), currentColumn(0), startCurrentColumn(0), finishCurrentColumn(0), mode(MODE_VIEW), insertMode(true)

{
    init();
}

IdentityTableEditor::~IdentityTableEditor()
{
    done();
}

IdentityTableEditor::IdentityTableEditor(
    IdentityTable &table
)
    : oldCurrentRow(0), oldCurrentCol(0), table(&table), datasetOffset(0), rows(80), cols(25), leftColumn(1), alwaysShowAddressColumn(true),
      currentRow(0), currentColumn(0), startCurrentColumn(0), finishCurrentColumn(0), mode(MODE_VIEW), insertMode(true)
{
}

size_t IdentityTableEditor::maxDataRowsVisible() const {
    return rows - 2;    // - header - status line
}

size_t IdentityTableEditor::dataRowsVisible() const {
    return cellValue.size();
}

void IdentityTableEditor::getRows() {
    cellValue.clear();
    if (table)
        table->getRows(cellValue, datasetOffset, maxDataRowsVisible());
}

int IdentityTableEditor::move2row(int steps) {
    if (steps == 0 || !table)
        return 0;
    auto recordCount = table->recordCount();
    auto visibleRows = maxDataRowsVisible();

    auto previousPosition = currentRow + datasetOffset;
    auto newPosition = previousPosition + steps;
    if (newPosition < 0)
        newPosition = 0;
    else
        if (newPosition >= recordCount)
            newPosition = (int) (recordCount > 0 ? recordCount - 1 : 0);
    int newDatasetOffset = (int) ((newPosition / visibleRows) * visibleRows);
    currentRow = newPosition - newDatasetOffset;

    // update edit
    cellEditor.setToCurrent(this);

    if (newDatasetOffset != datasetOffset) {
        // save changes if any
        save();
        datasetOffset = newDatasetOffset;
        cellValue.clear();
        table->getRows(cellValue, datasetOffset, visibleRows);
    }
    return newPosition - previousPosition;
}

bool IdentityTableEditor::move2col(int step) {
    auto oldCol = currentColumn;
    if (step < 0) {
        currentColumn += step;
        if (currentColumn < 0)
            currentColumn = 0;
        int v = moveLeftColumnX2makeFocusVisible();
        if (v)
            leftColumn += v;
        if (currentColumn == 0) {
            if (move2row(-1) == -1)
                currentColumn = COLUMN_COUNT - 1;
        }
    }
    if (step > 0) {
        if (currentColumn < COLUMN_COUNT - step) {
            currentColumn += step;
            int v = moveLeftColumnX2makeFocusVisible();
            if (v)
                leftColumn += v;
        } else {
            currentColumn = 0;
            int v = moveLeftColumnX2makeFocusVisible();
            if (v)
                leftColumn += v;
            move2row(1);
        }
    }
    // update edit
    cellEditor.setToCurrent(this);
    return currentColumn != oldCol;
}

bool IdentityTableEditor::toggleInsertMode()
{
    insertMode = !insertMode;
    return insertMode;
}

void IdentityTableEditor::deleteChar(
    int step
) {
    cellEditor.deleteChar(step);
    cellValue[currentRow].changed |= currentCellChanged();
    cellValue[currentRow].row[currentColumn] = cellEditor.cellValue();
}

void IdentityTableEditor::deleteLine(
    int step
) {
    if (rmRow(currentRow + step)) {
        getRows();
    }
}

bool IdentityTableEditor::visibleRowsChanged()
{
    for (auto &c : cellValue) {
        if (c.changed)
            return true;
    }
    return false;
}

void IdentityTableEditor::save()
{
    for (auto &c : cellValue) {
        if (c.changed) {
            if (table)
                table->saveRow(c);
            c.changed = false;
        }
    }
}

bool IdentityTableEditor::rmRow(int index)
{
    if (index < 0 || index >= cellValue.size())
        return false;
    bool r = table->rmRow(cellValue[index]);
    if (r)
        cellValue.erase(cellValue.begin() + index);
    return r;
}

std::string IdentityTableEditor::strHeader() const {
    std::stringstream ss;
    int totalWidth = 0;
    int i = leftColumn;
    if (alwaysShowAddressColumn) {
        ss << std::left << std::setw(columnWidth[0]) << std::setprecision(columnWidth[0]) << columnHeader[0] << DLMT;
        totalWidth += columnWidth[0] + DLMT_SIZE;
        if (i == 0)
            i = 1;
    }
    for (; i < COLUMN_COUNT; i++) {
        ss << std::left << std::setw(columnWidth[i]) << std::setprecision(columnWidth[i]) << columnHeader[i] << DLMT;
        totalWidth += columnWidth[i] + DLMT_SIZE;
        if (totalWidth >= cols)
            break;
    }
    std::string r = ss.str();
    return r.substr(0, cols);
}

std::string IdentityTableEditor::strRow(int row) {
    std::stringstream ss;
    int totalWidth = 0;
    int i = leftColumn;
    if (alwaysShowAddressColumn) {
        ss << std::right << std::setw(columnWidth[0]) << std::setprecision(columnWidth[0]) << cellValue[row].row[0].c_str() << DLMT;
        totalWidth += columnWidth[0] + DLMT_SIZE;
        if (i == 0)
            i = 1;
    }
    for (; i < COLUMN_COUNT; i++) {
        ss << std::right << std::setw(columnWidth[i]) << std::setprecision(columnWidth[i]) << cellValue[row].row[i].c_str() << DLMT;
        totalWidth += columnWidth[i] + DLMT_SIZE;
        if (totalWidth >= cols)
            break;
    }
    std::string r = ss.str();
    return r.substr(0, cols);
}

/**
 * Return highlighted cells positions
 * @param start return start position of focused cell, if cell is out of screen (not visible), return -1
 * @param finish return finish position of focused cell, if cell is out of screen (not visible), return -1
 * @param addressStart if address column visible in the screen (also in the case address column is always visible), return start position of the address = 0
 * @param addressFinish if address column visible in the screen (also in the case address column is always visible), return finish position of the address = 8
 * @return true if focused cell visible in the screen
 */
bool IdentityTableEditor::getCurrentColumnStartFinish(
    int &addressStart,
    int &addressFinish
) {
    startCurrentColumn = -1;
    finishCurrentColumn = -1;
    addressStart = -1;
    addressFinish = -1;

    int totalWidth = 0;
    int i = leftColumn;
    if (alwaysShowAddressColumn) {
        totalWidth += columnWidth[0] + DLMT_SIZE;
        if (currentColumn == 0) {
            // highlight as selected
            startCurrentColumn = 0;
            finishCurrentColumn = totalWidth - DLMT_SIZE;
            if (finishCurrentColumn > cols)
                finishCurrentColumn = cols + 1;
            return true;
        } else {
            // highlight as row ptr
            addressStart = 0;
            addressFinish = totalWidth - DLMT_SIZE;
            if (addressFinish > cols)
                addressFinish = cols + 1;
        }
        if (i == 0)
            i = 1;
    }
    for (; i < COLUMN_COUNT; i++) {
        int w = columnWidth[i] + DLMT_SIZE;
        if (currentColumn == i) {
            startCurrentColumn = totalWidth;
            finishCurrentColumn = totalWidth + w - DLMT_SIZE;
            if (finishCurrentColumn > cols)
                finishCurrentColumn = cols + 1;
            return true;
        } else {
            if (currentColumn == 0) {
                // highlight as row ptr
                addressStart = 0;
                addressFinish = totalWidth - DLMT_SIZE;
                if (addressFinish > cols)
                    addressFinish = cols + 1;
            }
        }
        totalWidth += w;
        if (totalWidth >= cols)
            break;
    }
    return false;
}

/**
 * Return value to +/- leftColumn to saveRow focus in the screen
 * @return 0 if selected column is visible
 */
int IdentityTableEditor::moveLeftColumnX2makeFocusVisible() const {
    // 1. focus to the left
    if (leftColumn >= currentColumn) {
        if (alwaysShowAddressColumn && currentColumn == 0)
            return 0;
        return -(leftColumn - currentColumn);
    }
    // 2. focus to the right
    int totalWidth = 0;
    int leftCol = leftColumn;
    if (alwaysShowAddressColumn) {
        totalWidth += columnWidth[0] + DLMT_SIZE;
        if (currentColumn == 0)
            return 0;
        if (leftCol == 0)
            leftCol = 1;
    }
    // calc distance from the left to start and finish positions
    for (int c = leftCol; c <= currentColumn; c++) {
        totalWidth += columnWidth[c] + DLMT_SIZE;
    }
    // increment leftColumn until cell is fit
    int c = leftCol;
    for (; c <= currentColumn; c++) {
        if (totalWidth < cols )
            break;
        totalWidth -= columnWidth[c] + DLMT_SIZE;
    }
    return c - leftColumn;
}

void IdentityTableEditor::printHeader() {
    std::string header = strHeader();
    int startAddressColumn, finishAddressColumn;
    if (getCurrentColumnStartFinish(startAddressColumn, finishAddressColumn)) {
        std::string s = header.substr(0, startCurrentColumn);
        std::string m = header.substr(startCurrentColumn, finishCurrentColumn - startCurrentColumn);
        std::string f;
        if (finishCurrentColumn < header.size())
            f = header.substr(finishCurrentColumn);

        gotoXY(0, 0);
        if (!s.empty())
            textOut(s);
        if (!m.empty()) {
            attrOn(1);
            textOut(m);
            attrOff(1);
        }
        if (!f.empty())
            textOut(f);
    } else
        textOut(header);
}

void IdentityTableEditor::printFooter() const {
    gotoXY(0, rows - 1);
    std::stringstream ss;
    ss << currentRow + datasetOffset + 1 << '/' << table->recordCount() << ' ' << (insertMode ? "INS" : "RPL");
    ss << " F5 Reload";
    if (mode == MODE_EDIT)
        ss << " F4 Ins F8 Del";
    textOut(ss.str());
}

void IdentityTableEditor::printRow(int rowIndex) {
    std::string row = strRow(rowIndex);
    if (rowIndex != currentRow) {
        gotoXY(0, rowIndex + 1);
        textOut(row);
        return;
    }
    int startAddressColumn, finishAddressColumn;
    bool selected = getCurrentColumnStartFinish(startAddressColumn, finishAddressColumn);

    std::string a;

    int startCol = startCurrentColumn;
    int finishCol = finishCurrentColumn;

    if (finishAddressColumn > startAddressColumn) {
        auto sz = finishAddressColumn - startAddressColumn;
        a = row.substr(0, sz);
        startCol -= sz;
        finishCol -= sz;
        row = row.substr(sz);
    }

    if (selected) {
        std::string s = row.substr(0, startCol);
        size_t selectedColWidth = columnWidth[currentColumn];
        std::string m = cellEditor.cellValue().substr(0, selectedColWidth); // row.substr(startCol, selectedRowWidth);
        if (m.size() < selectedColWidth)
            m.insert(0, selectedColWidth - m.size(), ' ');

        std::string f;
        if (finishCol < row.size())
            f = row.substr(finishCol);
        gotoXY(0, rowIndex + 1);
        if (!a.empty()) {
            attrOn(1);
            textOut(a);
            attrOff(1);
        }
        if (!s.empty())
            textOut(s);
        if (!m.empty()) {
            attrOn(2);
            textOut(m);
            attrOff(2);
        }
        if (!f.empty())
            textOut(f);
    } else {
        gotoXY(0, rowIndex + 1);
        if (!a.empty()) {
            attrOn(1);
            textOut(a);
            attrOff(1);
        }
        textOut(row);
    }
}

void IdentityTableEditor::placeChar(
    char ch
) {
    if (insertMode)
        cellEditor.insertChar(ch);
    else
        cellEditor.placeChar(ch);
    cellValue[currentRow].changed |= currentCellChanged();
    cellValue[currentRow].row[currentColumn] = cellEditor.cellValue();
}

std::string IdentityTableEditor::toString() const
{
    std::stringstream ss;
    ss
        << rows << 'x' << cols
        << ' ' << datasetOffset + 1
        << ':' << datasetOffset + dataRowsVisible() << '/' << table->recordCount()
        << " @" << currentRow + datasetOffset + 1 << ',' << currentColumn + 1  << " L:" << leftColumn
        << ' ' << (insertMode ? "INS" : "RPL");
    return ss.str();
}

void IdentityTableEditor::init()
{
    getRows();
}

void IdentityTableEditor::done()
{

}

const std::string& IdentityTableEditor::oldCellValue() const
{
    if (currentRow >= cellValue.size())
        return EMPTY_STR;
    if (currentColumn >= cellValue[currentRow].row.size())
        return EMPTY_STR;
    return cellValue[currentRow].row[currentColumn];
}

bool IdentityTableEditor::currentCellChanged() const
{
    return oldCellValue() != cellEditor.cellValue();
}

void IdentityTableEditor::saveRow()
{
    if (cellEditor.row < cellValue.size()) {
        if (cellEditor.col < cellValue[cellEditor.row].row.size()) {
            if (cellValue[cellEditor.row].changed) {
                cellValue[cellEditor.row].row[cellEditor.col] = cellEditor.cellValue();
                table->saveRow(cellValue[cellEditor.row]);
                cellValue[cellEditor.row].changed = true;
                notify("saveRow");
            }
        }
    }
}

void IdentityTableEditor::draw()
{
    getScreenSize();
    clearScreen();
    size_t rowsWithData = cellValue.size();
    printHeader();
    for (int r = 0; r < maxDataRowsVisible(); r++) {
        if (r < rowsWithData)
            printRow(r);
    }
    printFooter();
    // gotoXY(finishCurrentColumn, currentRow + 1);
    gotoXY(startCurrentColumn + cellEditor.cursor, currentRow + 1);
    flushScreen();
}

void IdentityTableEditor::view() {
    mode = MODE_VIEW;
    // update edit
    cellEditor.setToCurrent(this);
    draw();
}

void IdentityTableEditor::edit() {
    mode = MODE_EDIT;
    // update edit
    cellEditor.setToCurrent(this);
    draw();
}

bool IdentityTableEditor::cellJumped() {
    auto r = oldCurrentRow != currentColumn || oldCurrentCol != currentColumn;
    oldCurrentRow = currentColumn;
    oldCurrentCol = currentColumn;
    return r;
}

const std::string &IdentityTableEditor::currentCellValue()
{
    return cellEditor.cellValue();
}

void IdentityTableEditor::currentCellValue(
    const std::string &v
)
{
    cellEditor.cellValue(v);
    cellValue[currentRow].changed |= currentCellChanged();
    cellValue[currentRow].row[currentColumn] = v;
}
