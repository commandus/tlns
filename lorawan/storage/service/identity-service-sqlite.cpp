#include <sstream>
#include <iostream>
#include "lorawan/storage/service/identity-service-sqlite.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/helper/file-helper.h"
#include "lorawan/helper/sqlite-helper.h"

#ifdef ESP_PLATFORM
#include <iostream>
#include "platform-defs.h"
#endif

#include "lorawan/storage/serialization/identity-binary-serialization.h"

#define FIELD_LIST "addr, activation, class, deveui, nwkskey, appskey, version, appeui, appkey, nwkkey, devnonce, joinnonce, name"

SqliteIdentityService::SqliteIdentityService()
    : db(nullptr)
{

}

SqliteIdentityService::~SqliteIdentityService() = default;

static void row2DEVICEID(
    DEVICEID &retVal,
    const std::vector<std::string> &row
) {
    // row 0- address
    retVal.id.activation = string2activation(row[1]);
    retVal.id.deviceclass = string2deviceclass(row[2]);
    string2DEVEUI(retVal.id.devEUI, row[3]);
    string2KEY(retVal.id.nwkSKey, row[4]);
    string2KEY(retVal.id.appSKey, row[5]);
    retVal.id.version = string2LORAWAN_VERSION(row[6]);
    string2DEVEUI(retVal.id.appEUI, row[7]);
    string2KEY(retVal.id.appKey, row[8]);
    string2KEY(retVal.id.nwkKey, row[9]);
    retVal.id.devNonce = string2DEVNONCE(row[10]);
    string2JOINNONCE(retVal.id.joinNonce, row[11]);
    string2DEVICENAME(retVal.id.name, row[12].c_str());
}

/**
 * request device identifier by network address. Return 0 if success, retval = EUI and keys
 * @param retval device identifier
 * @param devaddr network address
 * @return CODE_OK- success
 */
int SqliteIdentityService::get(
    DEVICEID &retVal,
    const DEVADDR &request
)
{
    if (!db)
        return ERR_CODE_DB_DATABASE_NOT_FOUND;
    char *zErrMsg = nullptr;
    std::stringstream statement;
    statement << "SELECT " FIELD_LIST " FROM device WHERE addr = '"
        << DEVADDR2string(request)
        << "'";
    std::vector<std::string> row;
    int r = sqlite3_exec(db, statement.str().c_str(), rowCallback, &row, &zErrMsg);
    if (r != SQLITE_OK) {
        if (zErrMsg) {
            sqlite3_free(zErrMsg);
        }
        return ERR_CODE_DB_SELECT;
    } else {
        if (row.size() < 2)
            return ERR_CODE_BEST_GATEWAY_NOT_FOUND;
    }
    row2DEVICEID(retVal, row);
    return CODE_OK;
}

// List entries
int SqliteIdentityService::list(
    std::vector<NETWORKIDENTITY> &retVal,
    uint32_t offset,
    uint8_t size
) {
    if (!db)
        return ERR_CODE_DB_DATABASE_NOT_FOUND;
    char *zErrMsg = nullptr;
    std::stringstream statement;
    statement << "SELECT " FIELD_LIST " FROM device LIMIT " << (int) size << " OFFSET " << offset;
    std::vector<std::vector<std::string>> table;
    // uncomment to check SQL expression
    // std::cerr << statement.str() << std::endl;
    int r = sqlite3_exec(db, statement.str().c_str(), tableCallback, &table, &zErrMsg);
    if (r != SQLITE_OK) {
        if (zErrMsg) {
            sqlite3_free(zErrMsg);
        }
        return ERR_CODE_DB_SELECT;
    }
    for (auto row : table) {
        if (row.size() < 2)
            continue;
        NETWORKIDENTITY ni;
        row2DEVICEID(ni.value.devid, row);
        ni.value.devaddr = row[0];
        retVal.push_back(ni);
    }
    return CODE_OK;
}

// Entries count
size_t SqliteIdentityService::size()
{
    if (!db)
        return 0;
    char *zErrMsg = nullptr;
    std::string statement = "SELECT count(addr) FROM device";
    std::vector<std::string> row;
    int r = sqlite3_exec(db, statement.c_str(), rowCallback, &row, &zErrMsg);
    if (r != SQLITE_OK) {
        if (zErrMsg) {
            sqlite3_free(zErrMsg);
        }
        return 0;
    } else {
        if (row.empty())
            return 0;
    }
    return strtoull(row[0].c_str(), nullptr, 10);
}

/**
* request network identity(with address) by network address. Return 0 if success, retval = EUI and keys
* @param retval network identity(with address)
* @param eui device EUI
* @return CODE_OK- success
*/
int SqliteIdentityService::getNetworkIdentity(
    NETWORKIDENTITY &retval,
    const DEVEUI &eui
)
{
    if (!db)
        return ERR_CODE_DB_DATABASE_NOT_FOUND;
    char *zErrMsg = nullptr;
    std::stringstream statement;
    statement << "SELECT " FIELD_LIST " FROM device WHERE deveui = " << DEVEUI2string(eui) << " LIMIT 1";

    // uncomment to check SQL expression
    // std::cerr << statement.str() << std::endl;

    std::vector<std::vector<std::string>> table;
    int r = sqlite3_exec(db, statement.str().c_str(), tableCallback, &table, &zErrMsg);
    if (r != SQLITE_OK) {
        if (zErrMsg) {
            sqlite3_free(zErrMsg);
        }
        return ERR_CODE_DB_SELECT;
    }
    if (table.empty())
        return ERR_CODE_DEVICE_EUI_NOT_FOUND;
    row2DEVICEID(retval.value.devid, table[0]);
    retval.value.devaddr = table[0][0];
    return CODE_OK;
}

/**
 * UPSERT SQLite >= 3.24.0
 * @param devAddr device address
 * @param id device identifier
 * @return 0- success
 */
int SqliteIdentityService::put(
    const DEVADDR &devAddr,
    const DEVICEID &id
)
{
    if (!db)
        return ERR_CODE_DB_DATABASE_NOT_FOUND;
    char *zErrMsg = nullptr;
    std::stringstream statement;

    statement << "INSERT INTO device(" FIELD_LIST ") VALUES ('"
        << DEVADDR2string(devAddr) << "', "
        << "'" << activation2string(id.id.activation) << "', "
        << "'" << deviceclass2string(id.id.deviceclass) << "', "
        << "'" << DEVEUI2string(id.id.devEUI) << "', "
        << "'" << KEY2string(id.id.nwkSKey) << "', "
        << "'" << KEY2string(id.id.appSKey) << "', "
        << "'" << LORAWAN_VERSION2string(id.id.version) << "', "
        << "'" << DEVEUI2string(id.id.appEUI) << "', "
        << "'" << KEY2string(id.id.appKey) << "', "
        << "'" << KEY2string(id.id.nwkKey) << "', "
        << "'" << DEVNONCE2string(id.id.devNonce) << "', "
        << "'" << JOINNONCE2string(id.id.joinNonce) << "', "
        << "'" << DEVICENAME2string(id.id.name)
        << "') ON CONFLICT(addr) DO UPDATE SET "
        "activation=excluded.activation, class=excluded.class, deveui=excluded.deveui, "
        "nwkskey=excluded.nwkskey, appskey=excluded.appskey, version=excluded.version, "
        "appeui=excluded.appeui, appkey=excluded.appkey, nwkkey=excluded.nwkkey, "
        "devnonce=excluded.devnonce, joinnonce=excluded.joinnonce, name=excluded.name";
    int r = sqlite3_exec(db, statement.str().c_str(), nullptr, nullptr, &zErrMsg);
    if (r != SQLITE_OK) {
        if (zErrMsg) {
            sqlite3_free(zErrMsg);
        }
        return ERR_CODE_DB_INSERT;
    }
    return CODE_OK;
}

int SqliteIdentityService::rm(
    const DEVADDR &addr
)
{
    if (!db)
        return ERR_CODE_DB_DATABASE_NOT_FOUND;
    char *zErrMsg = nullptr;
    std::stringstream statement;
    statement << "DELETE FROM device WHERE addr = '" << DEVADDR2string(addr) << "'";
    int r = sqlite3_exec(db, statement.str().c_str(), nullptr, nullptr, &zErrMsg);
    // uncomment to check SQL expression
    // std::cerr << statement.str() << std::endl;
    if (r != SQLITE_OK) {
        if (zErrMsg) {
            sqlite3_free(zErrMsg);
        }
        return ERR_CODE_DB_EXEC;
    }
    return CODE_OK;
}

/**
 * "CREATE DATABASE IF NOT EXISTS \"device\" USE \"db_name\"",
 */
static std::string SCHEMA_STATEMENT[] {
    R"(CREATE TABLE "device" ("addr" TEXT NOT NULL PRIMARY KEY, "activation" TEXT, "class" TEXT, "deveui" TEXT, "nwkskey" TEXT, "appskey" TEXT, "version" TEXT, "appeui" TEXT, "appkey" TEXT, "nwkkey" TEXT, "devnonce" TEXT, "joinnonce" TEXT, "name" TEXT))",
    R"(CREATE INDEX "device_key_deveui" ON "device" ("deveui"))"
};

static int createDatabaseFile(
    const std::string &fileName
)
{
    sqlite3 *db;
    int r = sqlite3_open_v2(fileName.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    if (r)
        return r;
    char *zErrMsg = nullptr;
    for (auto &s : SCHEMA_STATEMENT) {
        r = sqlite3_exec(db, s.c_str(), nullptr, nullptr, &zErrMsg);
        if (r != SQLITE_OK) {
            if (zErrMsg) {
                sqlite3_free(zErrMsg);
            }
            break;
        }
    }
    sqlite3_close(db);
    return r;
}

int SqliteIdentityService::init(
    const std::string &databaseName,
    void *database
)
{
    dbName = databaseName;
    if (database) {
        // use external db
        db = (sqlite3 *) database;
        return CODE_OK;
    }
    if (!file::fileExists(dbName)) {
        int r = createDatabaseFile(dbName);
        if (r)
            return r;
    }
    int r = sqlite3_open(dbName.c_str(), &db);
    if (r) {
        db = nullptr;
        return ERR_CODE_DB_DATABASE_OPEN;
    }
    // validate objects
    r = sqlite3_exec(db, "SELECT " FIELD_LIST " FROM device WHERE addr = ''", nullptr, nullptr, nullptr);
    if (r != SQLITE_OK) {
        r = createDatabaseFile(dbName);
        if (r)
            return r;
    }
    return CODE_OK;
}

void SqliteIdentityService::flush()
{
    // re-open database file
    // external db closed
    if (db)
        sqlite3_close(db);
    sqlite3_open(dbName.c_str(), &db);
}

void SqliteIdentityService::done()
{
    // int r =
    sqlite3_close(db);
    db = nullptr;
}

/**
 * Return next network address if available
 * @return 0- success, ERR_CODE_ADDR_SPACE_FULL- no address available
 */
int SqliteIdentityService::next(
    NETWORKIDENTITY &retval
)
{
    return ERR_CODE_ADDR_SPACE_FULL;
}

void SqliteIdentityService::setOption(
    int option,
    void *value
)

{
    // nothing to do
}

// ------------------- asynchronous imitation -------------------
int SqliteIdentityService::cGet(const DEVADDR &request)
{
    IdentityGetResponse r;
    r.response.value.devaddr = request;
    get(r.response.value.devid, request);
    if (responseClient)
        responseClient->onIdentityGet(nullptr, &r);
    return CODE_OK;
}

int SqliteIdentityService::cGetNetworkIdentity(const DEVEUI &eui)
{
    IdentityGetResponse r;
    getNetworkIdentity(r.response, eui);
    if (responseClient)
        responseClient->onIdentityGet(nullptr, &r);
    return CODE_OK;
}

int SqliteIdentityService::cPut(const DEVADDR &devAddr, const DEVICEID &id)
{
    IdentityOperationResponse r;
    r.response = put(devAddr, id);
    if (responseClient)
        responseClient->onIdentityOperation(nullptr, &r);
    return CODE_OK;
}

int SqliteIdentityService::cRm(const DEVADDR &devAddr)
{
    IdentityOperationResponse r;
    r.response = rm(devAddr);
    if (responseClient)
        responseClient->onIdentityOperation(nullptr, &r);
    return CODE_OK;
}

int SqliteIdentityService::cList(
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

int SqliteIdentityService::cSize()
{
    IdentityOperationResponse r;
    r.size = (uint8_t) size();
    if (responseClient)
        responseClient->onIdentityOperation(nullptr, &r);
    return CODE_OK;
}

int SqliteIdentityService::cNext()
{
    IdentityGetResponse r;
    next(r.response);
    if (responseClient)
        responseClient->onIdentityGet(nullptr, &r);
    return CODE_OK;
}

int SqliteIdentityService::filter(
    std::vector<NETWORKIDENTITY> &retVal,
    const std::vector<NETWORK_IDENTITY_FILTER> &filters,
    uint32_t offset,
    uint8_t size
)
{
    if (!db)
        return ERR_CODE_DB_DATABASE_NOT_FOUND;
    char *zErrMsg = nullptr;
    std::stringstream statement;
    statement << "SELECT " FIELD_LIST " FROM device ";
    if (!filters.empty())
        statement << "WHERE " << NETWORK_IDENTITY_FILTERS2string(filters);
    statement << " LIMIT " << (int) size << " OFFSET " << offset;

    // uncomment to check SQL expression
    // std::cerr << statement.str() << std::endl;

    std::vector<std::vector<std::string>> table;
    int r = sqlite3_exec(db, statement.str().c_str(), tableCallback, &table, &zErrMsg);
    if (r != SQLITE_OK) {
        if (zErrMsg) {
            sqlite3_free(zErrMsg);
        }
        return ERR_CODE_DB_SELECT;
    }
    for (auto row : table) {
        if (row.size() < 2)
            continue;
        NETWORKIDENTITY ni;
        row2DEVICEID(ni.value.devid, row);
        ni.value.devaddr = row[0];
        retVal.push_back(ni);
    }
    return CODE_OK;
}

int SqliteIdentityService::cFilter(
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

EXPORT_SHARED_C_FUNC IdentityService* makeIdentityService3()
{
    return new SqliteIdentityService;
}
