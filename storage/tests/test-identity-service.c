#include <string.h>
#include <stdio.h>

#include "lorawan/storage/service/identity-service-c-wrapper.h"

static C_DEVADDR devAddr = 0x12345678;

static C_DEVICEID devId = {
    0, 1,   // activation, device class
    0x12345678, // devEui
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
    1,  // version
    0x12345678, // appEui
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
    0xaabb,      // last device nonce
    { 5, 6, 7 },    ///< last Join nonce
    { 'n', 'a', 'm', 'e', 0, 0, 0, 0 }
};

static void testSqlite()
{
    // sqlite
    void *o = makeIdentityServiceC(CISI_SQLITE);

    c_init(o, "test.sqlite.db", NULL);
    c_put(o, &devAddr, &devId);
    memset(&devId, 0, sizeof(devId));
    c_get(o, &devId, &devAddr);

    size_t c = c_size(o);
    C_NETWORKIDENTITY nis[2];
    c = c_list(o, nis, 0, 2);
    // c_rm(o, &devAddr);
    c = c_list(o, nis, 0, 2);
    C_NETWORK_IDENTITY_FILTER ff = {C_NILPO_AND, C_NIP_ACTIVATION, C_NICO_EQ, 1, 1};
    C_NETWORK_IDENTITY_FILTER filters[3] = {
        { C_NILPO_AND, C_NIP_ACTIVATION, C_NICO_EQ, 1, 0 },
        { C_NILPO_AND, C_NIP_DEVEUI, C_NICO_EQ, sizeof(C_DEVEUI), { 0x78, 0x56, 0x34, 0x12 }},
        { C_NILPO_AND, C_NIP_DEVICE_CLASS, C_NICO_EQ, 1, 1 }
    };
    c = c_filter(o, nis, filters, 3, 0, 2);

    const char *filterExpression = "activation = 'ABP' and deveui = '12345678' and class = 'B'";
    c = c_filterExpression(o, nis, filterExpression, strlen(filterExpression), 0, 2);

    c_done(o);
    destroyIdentityServiceC(o);
}

static void testJson()
{
    // JSON
    void *o = makeIdentityServiceC(CISI_JSON);
    c_init(o, "test.json", NULL);
    size_t sz = c_size(o);
    c_put(o, &devAddr, &devId);
    memset(&devId, 0, sizeof(devId));
    c_get(o, &devId, &devAddr);

    C_NETWORKIDENTITY nis[2];
    int c = c_list(o, nis, 0, 2);
    // c_rm(o, &devAddr);
    c = c_list(o, nis, 0, 2);
    C_NETWORK_IDENTITY_FILTER ff = {C_NILPO_AND, C_NIP_ACTIVATION, C_NICO_EQ, 1, 1};
    C_NETWORK_IDENTITY_FILTER filters[3] = {
        { C_NILPO_AND, C_NIP_ACTIVATION, C_NICO_EQ, 1, 0 },
        { C_NILPO_AND, C_NIP_DEVEUI, C_NICO_EQ, sizeof(C_DEVEUI), { 0x78, 0x56, 0x34, 0x12 }},
        { C_NILPO_AND, C_NIP_DEVICE_CLASS, C_NICO_EQ, 1, 1 }
    };
    c = c_filter(o, nis, filters, 3, 0, 2);


    const char *filterExpression = "activation = 'ABP' and deveui = '12345678' and class = 'B'";
    c = c_filterExpression(o, nis, filterExpression, strlen(filterExpression), 0, 2);

    c_flush(o);
    c_done(o);
    destroyIdentityServiceC(o);
}

static void testLmdb()
{
    // LMDB
    void *o = makeIdentityServiceC(CISI_LMDB);

    int r = c_init(o, "test.identity.lmdb.db", NULL);
    if (r)
        return;
    c_put(o, &devAddr, &devId);
    memset(&devId, 0, sizeof(devId));

    c_get(o, &devId, &devAddr);

    C_NETWORKIDENTITY nis[2];
    int c = c_list(o, nis, 0, 2);
    // c_rm(o, &devAddr);
    c = c_list(o, nis, 0, 2);
    C_NETWORK_IDENTITY_FILTER ff = {C_NILPO_AND, C_NIP_ACTIVATION, C_NICO_EQ, 1, 1};
    C_NETWORK_IDENTITY_FILTER filters[3] = {
            { C_NILPO_AND, C_NIP_ACTIVATION, C_NICO_EQ, 1, 0 },
            { C_NILPO_AND, C_NIP_DEVEUI, C_NICO_EQ, sizeof(C_DEVEUI), { 0x78, 0x56, 0x34, 0x12 }},
            { C_NILPO_AND, C_NIP_DEVICE_CLASS, C_NICO_EQ, 1, 1 }
    };
    c = c_filter(o, nis, filters, 3, 0, 2);

    const char *filterExpression = "activation = 'ABP' and deveui = '12345678' and class = 'B'";
    c = c_filterExpression(o, nis, filterExpression, strlen(filterExpression), 0, 2);

    c_done(o);
    destroyIdentityServiceC(o);
}

static void testString()
{
    char buffer[256];
    char *p[13];

    memset(buffer, 0, sizeof(buffer));
    C_DEVADDR a = 0x01020304;
    C_DEVICEID did;
    size_t position = c_devaddr2text(buffer, sizeof(buffer), &a);
    if (position >= sizeof(buffer))
        return;
    buffer[position] = '\0';
    position++;
    p[0] = buffer;

    c_deviceid2text(buffer + position, sizeof(buffer) - position, p, &devId);
    memset(&did, 0, sizeof(did));
    text2c_deviceid(&did, (const char **) p);
    for (int i = 0; i < 13; i++) {
        printf("%s ", p[i]);
    }
    printf("\n");


    memset(buffer, 0, sizeof(buffer));
    C_NETWORKIDENTITY v;
    v.devaddr = 0x01020304;
    memmove((char *) &v.devid, (char *) &devId, sizeof(devId));
    memset(&buffer, 0, sizeof(buffer));
    memset(&p, 0, sizeof(p));
    c_networkidentity2text(buffer, sizeof(buffer), p, &v);
    memset(&v, 0, sizeof(v));
    text2c_networkidentity(&v, (const char **) p);
    memset(&buffer, 0, sizeof(buffer));
    memset(&p, 0, sizeof(p));
    c_networkidentity2text(buffer, sizeof(buffer), p, &v);
    for (int i = 0; i < 13; i++) {
        printf("%s ", p[i]);
    }
    printf("\n");

}

int main() {
    testString();
    // testSqlite();
    // testJson();
    // testLmdb();
    return 0;
}
