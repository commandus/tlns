#include <sstream>
#include <iostream>
#include <cstring>
#include "lorawan/storage/service/identity-service-lmdb.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/helper/file-helper.h"
#include "lorawan/storage/serialization/identity-binary-serialization.h"

#ifdef ESP_PLATFORM
#include <iostream>
#include "platform-defs.h"
#endif

LMDBIdentityService::LMDBIdentityService() = default;

LMDBIdentityService::~LMDBIdentityService() = default;

/**
 * request device identifier by network address. Return 0 if success, retval = EUI and keys
 * @param retval device identifier
 * @param devaddr network address
 * @return CODE_OK- success
 */
int LMDBIdentityService::get(
    DEVICEID &retVal,
    const DEVADDR &request
)
{
    // start transaction
    int r = mdb_txn_begin(env.env, nullptr, MDB_RDONLY, &env.txn);
    if (r)
        return ERR_CODE_LMDB_TXN_BEGIN;
    MDB_val dbKey {SIZE_DEVADDR, (void *) &request.u };
    MDB_val dbVal {};
    r = mdb_get(env.txn, env.dbi, &dbKey, &dbVal);
    if (r != MDB_SUCCESS) {
        // it's ok
        mdb_txn_abort(env.txn);
        return r;
    }
    memmove((void*) &retVal.id, dbVal.mv_data, dbVal.mv_size < sizeof(DEVICE_ID) ? dbVal.mv_size : sizeof(DEVICE_ID));
    r = mdb_txn_commit(env.txn);
    return r;
}

// List entries
int LMDBIdentityService::list(
    std::vector<NETWORKIDENTITY> &retVal,
    uint32_t offset,
    uint8_t size
) {
    // start transaction
    int r = mdb_txn_begin(env.env, nullptr, MDB_RDONLY, &env.txn);
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

    MDB_val dbKey {};
    MDB_val dbVal {};

    while ((r = mdb_cursor_get(cursor, &dbKey, &dbVal, MDB_NEXT)) == 0) {
        if (o < offset) {
            // skip first
            o++;
            continue;
        }
        sz++;
        if (sz > size)
            break;

        if (dbKey.mv_size != SIZE_DEVADDR)
            break;  // error, database corrupted
        NETWORKIDENTITY nid;
        memmove((void*) &nid.value.devaddr, dbKey.mv_data, dbKey.mv_size < SIZE_DEVADDR ? dbKey.mv_size : SIZE_DEVADDR);
        memmove((void*) &nid.value.devid, dbVal.mv_data, dbVal.mv_size < sizeof(DEVICE_ID) ? dbVal.mv_size : sizeof(DEVICE_ID));
        retVal.emplace_back(nid.value.devaddr, nid.value.devid);
    } while (mdb_cursor_get(cursor, &dbKey, &dbVal, MDB_NEXT) == MDB_SUCCESS);

    r = mdb_txn_commit(env.txn);
    return r;
}

// Entries count
size_t LMDBIdentityService::size()
{
    // start transaction
    int r = mdb_txn_begin(env.env, nullptr, MDB_RDONLY, &env.txn);
    if (r)
        return ERR_CODE_LMDB_TXN_BEGIN;
    MDB_stat stat;
    mdb_stat(env.txn, env.dbi, &stat);
    mdb_txn_commit(env.txn);
    return stat.ms_entries;
}

/**
* request network identity(with address) by network address. Return 0 if success, retval = EUI and keys
* @param retval network identity(with address)
* @param eui device EUI
* @return CODE_OK- success
*/
int LMDBIdentityService::getNetworkIdentity(
    NETWORKIDENTITY &retVal,
    const DEVEUI &eui
)
{
    // start transaction
    int r = mdb_txn_begin(env.env, nullptr, MDB_RDONLY, &env.txn);
    if (r)
        return ERR_CODE_LMDB_TXN_BEGIN;

    MDB_cursor *cursor;
    r = mdb_cursor_open(env.txn, env.dbi, &cursor);
    if (r != MDB_SUCCESS) {
        mdb_txn_commit(env.txn);
        return r;
    }

    MDB_val dbKey {};
    MDB_val dbVal {};

    while ((r = mdb_cursor_get(cursor, &dbKey, &dbVal, MDB_NEXT)) == MDB_SUCCESS) {
        if (dbKey.mv_size != SIZE_DEVADDR || dbVal.mv_size != sizeof(DEVICE_ID))
            break;  // error, database corrupted

        if (((DEVICE_ID *)dbVal.mv_data)->devEUI == eui) {
            memmove((void*) &retVal.value.devaddr.u, dbKey.mv_data, dbKey.mv_size < SIZE_DEVADDR ? dbKey.mv_size : SIZE_DEVADDR);
            memmove((void*) &retVal.value.devid, dbVal.mv_data, dbVal.mv_size < sizeof(DEVICE_ID) ? dbVal.mv_size : sizeof(DEVICE_ID));
            break;
        }
    };

    r = mdb_txn_commit(env.txn);
    return r;
}

/**
 * UPSERT SQLite >= 3.24.0
 * @param request gateway identifier or address
 * @return 0- success
 */
int LMDBIdentityService::put(
    const DEVADDR &devAddr,
    const DEVICEID &id
)
{
    // start transaction
    int r = mdb_txn_begin(env.env, nullptr, 0, &env.txn);
    if (r)
        return ERR_CODE_LMDB_TXN_BEGIN;
    MDB_val dbKey {SIZE_DEVADDR, (void*) &devAddr.u };
    MDB_val dbData {sizeof(DEVICE_ID), (void *) &id.id };
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

int LMDBIdentityService::rm(
    const DEVADDR &addr
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
    MDB_val dbKey {SIZE_DEVADDR, (void *) &addr.u};
    DEVICE_ID did;
    r = mdb_cursor_get(cursor, &dbKey, nullptr, MDB_SET_RANGE);
    if (r != MDB_SUCCESS) {
        mdb_txn_commit(env.txn);
        return r;
    }

    do {
        if (dbKey.mv_size != SIZE_DEVADDR)
            break;  // error, database corrupted
        if (*(uint32_t*)dbKey.mv_data != addr.u)
            break;  // out of range
        mdb_cursor_del(cursor, 0);
    } while (mdb_cursor_get(cursor, &dbKey, nullptr, MDB_NEXT) == MDB_SUCCESS);

    r = mdb_txn_commit(env.txn);
    if (r)
        return ERR_CODE_LMDB_TXN_COMMIT;
    return r;
}

int LMDBIdentityService::init(
    const std::string &databaseName,
    void *database
)
{
    env.setDb(databaseName);
    if (!openDb(&env))
        return ERR_CODE_LMDB_OPEN;
    return CODE_OK;
}

void LMDBIdentityService::flush()
{
}

void LMDBIdentityService::done() {
    closeDb(&env);
}

/**
 * Return next network address if available
 * @return 0- success, ERR_CODE_ADDR_SPACE_FULL- no address available
 */
int LMDBIdentityService::next(
    NETWORKIDENTITY &retval
)
{
    return ERR_CODE_ADDR_SPACE_FULL;
}

void LMDBIdentityService::setOption(
    int option,
    void *value
)

{
    // nothing to do
}

EXPORT_SHARED_C_FUNC IdentityService* makeLMDBIdentityService()
{
    return new LMDBIdentityService;
}

// ------------------- asynchronous imitation -------------------
int LMDBIdentityService::cGet(const DEVADDR &request)
{
    IdentityGetResponse r;
    r.response.value.devaddr = request;
    get(r.response.value.devid, request);
    if (responseClient)
        responseClient->onIdentityGet(nullptr, &r);
    return CODE_OK;
}

int LMDBIdentityService::cGetNetworkIdentity(const DEVEUI &eui)
{
    IdentityGetResponse r;
    getNetworkIdentity(r.response, eui);
    if (responseClient)
        responseClient->onIdentityGet(nullptr, &r);
    return CODE_OK;
}

int LMDBIdentityService::cPut(const DEVADDR &devAddr, const DEVICEID &id)
{
    IdentityOperationResponse r;
    r.response = put(devAddr, id);
    if (responseClient)
        responseClient->onIdentityOperation(nullptr, &r);
    return CODE_OK;
}

int LMDBIdentityService::cRm(const DEVADDR &devAddr)
{
    IdentityOperationResponse r;
    r.response = rm(devAddr);
    if (responseClient)
        responseClient->onIdentityOperation(nullptr, &r);
    return CODE_OK;
}

int LMDBIdentityService::cList(
    uint32_t offset,
    uint8_t size
)
{
    IdentityListResponse r;
    r.response = list(r.identities, offset, size);
    r.size = (uint8_t) r.identities.size();
    if (responseClient)
        responseClient->onIdentityList(nullptr, &r);
   return CODE_OK;
}

int LMDBIdentityService::filter(
    std::vector<NETWORKIDENTITY> &retVal,
    const std::vector<NETWORK_IDENTITY_FILTER> &filters,
    uint32_t offset,
    uint8_t size
)
{
    // start transaction
    int r = mdb_txn_begin(env.env, nullptr, MDB_RDONLY, &env.txn);
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

    MDB_val dbKey {};
    MDB_val dbVal {};

    while ((r = mdb_cursor_get(cursor, &dbKey, &dbVal, MDB_NEXT)) == 0) {
        if (o < offset) {
            // skip first
            o++;
            continue;
        }
        sz++;
        if (sz > size)
            break;

        if (dbKey.mv_size != SIZE_DEVADDR || dbVal.mv_size != sizeof(DEVICE_ID))
            break;  // error, database corrupted

        if (!isIdentityFilteredV2(*(DEVADDR*) dbKey.mv_data, *(DEVICE_ID*) dbVal.mv_data, filters))
            continue;
        if (o < offset) {
            // skip first
            o++;
            continue;
        }
        sz++;
        if (sz > size)
            break;
        NETWORKIDENTITY nid;
        memmove((void*) &nid.value.devaddr.u, dbKey.mv_data, dbKey.mv_size < SIZE_DEVADDR ? dbKey.mv_size : SIZE_DEVADDR);
        memmove((void*) &nid.value.devid, dbVal.mv_data, dbVal.mv_size < sizeof(DEVICE_ID) ? dbVal.mv_size : sizeof(DEVICE_ID));
        retVal.emplace_back(nid.value.devaddr, nid.value.devid);
    } while (mdb_cursor_get(cursor, &dbKey, &dbVal, MDB_NEXT) == MDB_SUCCESS);

    r = mdb_txn_commit(env.txn);
    return r;
}

int LMDBIdentityService::cFilter(
    const std::vector<NETWORK_IDENTITY_FILTER> &filters,
    uint32_t offset,
    uint8_t size
)
{
    IdentityListResponse r;
    r.response = filter(r.identities, filters, offset, size);
    r.size = (uint8_t) r.identities.size();
    if (responseClient)
        responseClient->onIdentityList(nullptr, &r);
    return CODE_OK;
}

int LMDBIdentityService::cSize()
{
    IdentityOperationResponse r;
    r.size = (uint8_t) size();
    if (responseClient)
        responseClient->onIdentityOperation(nullptr, &r);
    return CODE_OK;
}

int LMDBIdentityService::cNext()
{
    IdentityGetResponse r;
    next(r.response);
    if (responseClient)
        responseClient->onIdentityGet(nullptr, &r);
    return CODE_OK;
}

EXPORT_SHARED_C_FUNC IdentityService* makeIdentityService5()
{
    return new LMDBIdentityService;
}
