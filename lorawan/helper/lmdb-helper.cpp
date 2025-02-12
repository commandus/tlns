#include <string>

#include "lorawan/lorawan-error.h"
#include "lorawan/helper/lmdb-helper.h"
#include "lorawan/lorawan-msg.h"
#include "file-helper.h"

#include <sstream>
#include <iostream>

dbenv::dbenv()
    : flags(0), mode(0664), log(nullptr)
{

}

dbenv::dbenv(
    const std::string &aPath,
    int aflags,
    int amode
)
    : path(aPath), flags(aflags), mode(amode), log(nullptr)
{

}

void dbenv::setDb(
    const std::string &aPath
)
{
    path = aPath;
}

void dbenv::LOG(
    int level,
    int code,
    const char *msg
)
{
    if (log)
        log(this, level, code, msg);
}

int processMapFull(
    dbenv *env
)
{
    mdb_txn_abort(env->txn);
    struct MDB_envinfo current_info;
    int r;
    r = mdb_env_info(env->env, &current_info);
    if (r) {
        env->LOG(LOG_ERR, ERR_CODE_LMDB_FULL_ENV_INFO, ERR_LMDB_FULL_ENV_INFO);
        return r;
    }
    if (!closeDb(env)) {
        env->LOG(LOG_ERR, ERR_CODE_LMDB_FULL_DB_CLOSE, ERR_LMDB_FULL_DB_CLOSE);
        return ERR_CODE_LMDB_CLOSE;
    }

    size_t new_size = current_info.me_mapsize;
    if (new_size <= 1024 * 1024)
        new_size *= 2;
    else
        new_size += 2 * 1024 * 1024;

    env->LOG(LOG_INFO, ERR_CODE_LMDB_FULL_DB_CLOSE, MSG_LMDB_INCREASE_MAP_SIZE);

    r = mdb_env_create(&env->env);
    if (r) {
        env->LOG(LOG_ERR, ERR_CODE_LMDB_FULL_ENV_CREATE, ERR_LMDB_FULL_ENV_CREATE);
        env->env = nullptr;
        return ERR_CODE_LMDB_OPEN;
    }
    r = mdb_env_set_mapsize(env->env, new_size);
    if (r)
        env->LOG(LOG_ERR, ERR_CODE_LMDB_FULL_SET_SIZE, ERR_LMDB_FULL_SET_SIZE);
    r = mdb_env_open(env->env, env->path.c_str(), env->flags, env->mode);
    mdb_env_close(env->env);
    if (!openDb(env)) {
        env->LOG(LOG_ERR, ERR_CODE_LMDB_FULL_DB_OPEN, ERR_LMDB_FULL_DB_OPEN);
        return r;
    }

    // start transaction
    r = mdb_txn_begin(env->env, nullptr, 0, &env->txn);
    if (r)
        env->LOG(LOG_ERR, ERR_CODE_LMDB_FULL_TXN_BEGIN, ERR_LMDB_FULL_TXN_BEGIN);
    return r;
}

/**
 * @brief Opens LMDB database file
 * @param env created LMDB environment(transaction, cursor)
 * @param config pass path, flags, file open mode
 * @return true- success
 */
bool openDb(
    dbenv *env
)
{
    int rc = mdb_env_create(&env->env);
    if (rc) {
        env->LOG(LOG_ERR, ERR_CODE_LMDB_OPEN, ERR_LMDB_ENV_CREATE);
        env->env = nullptr;
        return false;
    }

    rc = mdb_env_open(env->env, env->path.c_str(), env->flags, env->mode);
    if (rc) {
        if (rc == 3) {
            // try to create a new directory
            bool dirCreated = file::mkDir(env->path);
            if (dirCreated) {
                // try again
                rc = mdb_env_open(env->env, env->path.c_str(), env->flags, env->mode);
                if (rc == 0)
                    return true;
            }
        }
        env->LOG(LOG_ERR, ERR_CODE_LMDB_OPEN, ERR_LMDB_ENV_OPEN);
        env->env = nullptr;
        return false;
    }

    rc = mdb_txn_begin(env->env, nullptr, 0, &env->txn);
    if (rc) {
        env->LOG(LOG_ERR, ERR_CODE_LMDB_OPEN, ERR_LMDB_TXN_BEGIN);
        env->env = nullptr;
        return false;
    }

    rc = mdb_dbi_open(env->txn, nullptr, 0, &env->dbi);
    if (rc) {
        env->LOG(LOG_ERR, ERR_CODE_LMDB_OPEN, ERR_LMDB_OPEN);
        env->env = nullptr;
        return false;
    }

    rc = mdb_txn_commit(env->txn);

    return rc == 0;
}

/**
 * @brief Close LMDB database file
 * @param config pass path, flags, file open mode
 * @return true- success
 */
bool closeDb(
    dbenv *env
)
{
    mdb_dbi_close(env->env, env->dbi);
    mdb_env_close(env->env);
    return true;
}
