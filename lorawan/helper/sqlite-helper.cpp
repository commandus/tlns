#include <string>
#include <vector>
#include "sqlite-helper.h"

int tableCallback(
    void *env,
    int columns,
    char **value,
    char **column
)
{
    if (!env)
        return 0;
    std::vector<std::vector<std::string>> *table = (std::vector<std::vector<std::string>> *) env;

    std::vector<std::string> line;
    for (int i = 0; i < columns; i++) {
        line.push_back(value[i] ? value[i] : "");
    }
    table->push_back(line);
    return 0;
}

int rowCallback(
    void *env,
    int columns,
    char **value,
    char **column
)
{
    if (!env)
        return 0;
    std::vector<std::string> *row = (std::vector<std::string> *) env;
    if (!row->empty())
        return 0;   // just first row
    for (int i = 0; i < columns; i++) {
        row->push_back(value[i] ? value[i] : "");
    }
    return 0;
}
