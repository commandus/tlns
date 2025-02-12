#include <cstring>

#include "gateway-service-lmdb.h"
#include "lorawan/helper/ip-address.h"
#include "lorawan/lorawan-types.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-string.h"

#ifdef ESP_PLATFORM
#include <iostream>
#include "platform-defs.h"
#endif

LMDBGatewayService::LMDBGatewayService()
= default;

LMDBGatewayService::~LMDBGatewayService() = default;

void LMDBGatewayService::clear()
{
}

/**
 * request device identifier by network address. Return 0 if success, retval = EUI and keys
 * @param retval device identifier
 * @param devaddr network address
 * @return LORA_OK- success
 */
int LMDBGatewayService::get(
    GatewayIdentity &retVal,
    const GatewayIdentity &request
)
{
    if (request.gatewayId) {
        // find out by gateway identifier
        memset(&retVal.sockaddr, 0, sizeof(retVal.sockaddr));
        return ERR_CODE_GATEWAY_NOT_FOUND;
    } else {
        // reverse find out by address
        return ERR_CODE_GATEWAY_NOT_FOUND;
    }
}

// List entries
int  LMDBGatewayService::list(
    std::vector<GatewayIdentity> &retVal,
    uint32_t offset,
    uint8_t size
)
{
    // start transaction
    int r = mdb_txn_begin(env.env, nullptr, 0, &env.txn);
    if (r)
        return ERR_CODE_LMDB_TXN_BEGIN;

    size_t o = 0;
    size_t sz = 0;

    MDB_cursor *cursor;
    r = mdb_cursor_open(env.txn, env.dbi, &cursor);
    if (r != MDB_SUCCESS) {
        mdb_txn_commit(env.txn);
        return r;
    }

    GatewayIdentity id;
    MDB_val dbKey {sizeof(uint64_t), (void *) &id.gatewayId };
    MDB_val dbVal {sizeof(struct sockaddr), (void *) &id.sockaddr };

    while ((r = mdb_cursor_get(cursor, &dbKey, &dbVal, MDB_NEXT)) == 0) {
        if (o < offset) {
            // skip first
            o++;
            continue;
        }
        sz++;
        if (sz > size)
            break;

        if (dbKey.mv_size != sizeof(uint64_t) || dbVal.mv_size != sizeof(struct sockaddr))
            break;  // error, database corrupted
        retVal.emplace_back(id);
    } while (mdb_cursor_get(cursor, &dbKey, &dbVal, MDB_NEXT) == MDB_SUCCESS);

    r = mdb_txn_commit(env.txn);
    return r;
}

// Entries count
size_t LMDBGatewayService::size()
{
    // start transaction
    int r = mdb_txn_begin(env.env, nullptr, 0, &env.txn);
    if (r)
        return ERR_CODE_LMDB_TXN_BEGIN;
    MDB_stat stat;
    mdb_stat(env.txn, env.dbi, &stat);
    mdb_txn_commit(env.txn);
    return stat.ms_entries;
}

int LMDBGatewayService::put(
    const GatewayIdentity &request
)
{
    // start transaction
    int r = mdb_txn_begin(env.env, nullptr, 0, &env.txn);
    if (r)
        return ERR_CODE_LMDB_TXN_BEGIN;
    MDB_val dbKey { sizeof(uint64_t), (void*) &request.gatewayId };
    MDB_val dbData { sizeof(struct sockaddr), (void *) &request.sockaddr };
    r = mdb_put(env.txn, env.dbi, &dbKey, &dbData, 0);
    if (r) {
        if (r == MDB_MAP_FULL) {
            r = processMapFull(&env);
            if (r == 0)
                r = mdb_put(env.txn, env.dbi, &dbKey, &dbData, 0);
        }
        if (r) {
            mdb_txn_abort(env.txn);
            return ERR_CODE_LMDB_PUT;
        }
    }
    r = mdb_txn_commit(env.txn);
    if (r) {
        if (r == MDB_MAP_FULL) {
            r = processMapFull(&env);
            if (r == 0) {
                r = mdb_txn_commit(env.txn);
            }
        }
        if (r)
            return ERR_CODE_LMDB_TXN_COMMIT;
    }
    return r;
}

int LMDBGatewayService::rm(
    const GatewayIdentity &request
)
{
    // start transaction
    int r = mdb_txn_begin(env.env, nullptr, 0, &env.txn);
    if (r)
        return ERR_CODE_LMDB_TXN_BEGIN;
    MDB_cursor *cursor;
    r = mdb_cursor_open(env.txn, env.dbi, &cursor);
    if (r != MDB_SUCCESS) {
        mdb_txn_commit(env.txn);
        return r;
    }

    if (request.gatewayId) {
        // find out by gateway identifier
        MDB_val dbKey { sizeof(uint64_t), (void *) &request.gatewayId };
        struct sockaddr addr {};
        MDB_val dbVal {sizeof(struct sockaddr), (void*) &addr };
        r = mdb_cursor_get(cursor, &dbKey, &dbVal, MDB_SET_RANGE);
        if (r != MDB_SUCCESS) {
            mdb_txn_commit(env.txn);
            return r;
        }
        do {
            if (dbKey.mv_size != sizeof(uint64_t))
                break;  // error, database corrupted
            if (*(uint32_t*)dbKey.mv_data != request.gatewayId)
                break;  // out of range
            mdb_cursor_del(cursor, 0);
        } while (mdb_cursor_get(cursor, &dbKey, &dbVal, MDB_NEXT) == MDB_SUCCESS);
    } else {
        // reverse find out by address
        uint64_t gwId = 0;
        MDB_val dbKey { sizeof(uint64_t), (void *) &gwId };
        struct sockaddr addr {};
        MDB_val dbVal {sizeof(struct sockaddr), (void*) &addr };
        while ((r = mdb_cursor_get(cursor, &dbKey, &dbVal, MDB_NEXT)) == 0) {
            if (dbKey.mv_size != sizeof(uint64_t))
                break;  // error, database corrupted
            if (sameSocketAddress(&request.sockaddr, &addr)) {
                return CODE_OK;
            }
        } while (mdb_cursor_get(cursor, &dbKey, &dbVal, MDB_NEXT) == MDB_SUCCESS);
    }
    r = mdb_txn_commit(env.txn);
    if (r)
        return ERR_CODE_LMDB_TXN_COMMIT;
    return r;
}

int LMDBGatewayService::init(
    const std::string &databaseName,
    void *data
)
{
    env.setDb(databaseName);
    if (!openDb(&env))
        return ERR_CODE_LMDB_OPEN;
    return CODE_OK;
}

void LMDBGatewayService::flush()
{
}

void LMDBGatewayService::done()
{
    closeDb(&env);
}

void LMDBGatewayService::setOption(
    int option,
    void *value
)

{
    // nothing to do
}

EXPORT_SHARED_C_FUNC GatewayService* makeGatewayService5()
{
    return new LMDBGatewayService;
}
