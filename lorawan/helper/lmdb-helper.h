#ifndef LMDB_HELPER_H
#define LMDB_HELPER_H

#include "lmdb.h"

/**
 * @brief LMDB environment(transaction, cursor)
 */
class dbenv {
public:
    MDB_env *env;
    MDB_dbi dbi;
    MDB_txn *txn;
    MDB_cursor *cursor;
    // open db options
    std::string path;
    int flags;
    int mode;
    void (*log)(dbenv* env, int level, int code, const char *msg);
    dbenv();
    dbenv(const std::string &aPath, int flags, int mode);
    void setDb(const std::string &path);
    void LOG(int level, int code, const char *msg);
};

int processMapFull(
    dbenv *env
);

/**
 * @brief Opens LMDB database file
 * @param env created LMDB environment(transaction, cursor)
 * @param config pass path, flags, file open mode
 * @return true- success
 */
bool openDb(
    dbenv *env
);

/**
 * @brief Close LMDB database file
 * @param config pass path, flags, file open mode
 * @return true- success
 */
bool closeDb(
    dbenv *env
);

#endif // LMDB_HELPER_H
