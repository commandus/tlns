#ifndef SQLITE_HELPER_H
#define SQLITE_HELPER_H

int tableCallback(
    void *env,
    int columns,
    char **value,
    char **column
);

int rowCallback(
    void *env,
    int columns,
    char **value,
    char **column
);

#endif //SQLITE_HELPER_H
