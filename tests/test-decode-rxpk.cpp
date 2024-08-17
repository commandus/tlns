#include <string>
#include <iostream>
#include "lorawan/lorawan-types.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-conv.h"
#include "lorawan/helper/aes-helper.h"
#include "lorawan/proto/gw/basic-udp.h"

/*
02bbe50000006cc3743eed467b227278706b223a5b7b22746d7374223a343032333131313534302c226368616e223a332c2272666368223a302c2266726571223a3836342e3730303030302c2273746174223a312c226d6f6475223a224c4f5241222c2264617472223a22534631324257313235222c22636f6472223a22342f35222c226c736e72223a2d31382e352c2272737369223a2d3132312c2273697a65223a33372c2264617461223a22514441445251474151774143334749312b374553394d697030356a436c6f536f464e367a634b65437877394d7357457634513d3d227d5d7d
{"rxpk":[{"tmst":4023111540,"chan":3,"rfch":0,"freq":864.700000,"stat":1,"modu":"LORA","datr":"SF12BW125","codr":"4/5","lsnr":-18.5,"rssi":-121,"size":37,"data":"QDADRQGAQwAC3GI1+7ES9Mip05jCloSoFN6zcKeCxw9MsWEv4Q=="}]}
rxpk addr: 01450330, gw: 00006cc3743eed46, rssi: -121dBm, lsnr: -18.5dB
{"activation":"ABP", "nwkSKey":"313747123434383535003a0066378888", "appSKey":"35003a003434383531374712656b7f47"}


403003450180430002dc6235fbb112f4c8a9d398c29684a814deb370a782c70f4cb1612fe1
dc6235fbb112f4c8a9d398c29684a814deb370a782c70f4c
addr 01450330
fcnt: 67, direction: 0, devAddr: 01450330, appSKey: 5555000010dfffffff7f000085265655,
01002180549c6118000000004a0000000000000000000000
*/


/*
cd ~/src/lorawan-storage
./cmake-build-debug/lorawan-identity-service -f ../lorawan-network-server/identity.json -v

./lorawan-identity-print 403003450180430002dc6235fbb112f4c8a9d398c29684a814deb370a782c70f4cb1612fe1 -vv -s 127.0.0.2:4244 
unconfirmed-data-up, 01450330, 0, no-classB, no ACK, no-addrACKrequest, adr, 67, , 2, fcnt: 67, direction: 0, devAddr: 01450330, appSKey: 5555000010dfffffff7f000085265655, 556142b800005555, d045b47b1983c60d8cb03e1a500ed7190e1ec442bbe7bf9d

 */

static std::string decodePayload(
    const std::string &payload,
    const KEY128 &appSKey
)
{
    auto *rfm = (RFM_HEADER *) payload.c_str();
    auto sz = payload.size();
    char *pl = hasPayload((void *) payload.c_str(), sz);
    if (!pl)
        return "";
    std::string pld(pl, sz - (pl - (char *) rfm) - SIZE_MIC);
    std::cout << "addr: " << DEVADDR2string(rfm->fhdr.devaddr) << std::endl;    
    std::cout << "direction: " << (int) (rfm->macheader.f.mtype & 1) << std::endl;    
    std::cout << "FCnt: " << (int) (rfm->fhdr.fcnt) << std::endl;
    std::cout << "AppSKey: " << KEY2string(appSKey) << std::endl;
    decryptPayloadString(pld, rfm->fhdr.fcnt, rfm->macheader.f.mtype & 1, rfm->fhdr.devaddr, appSKey);
    return pld;
}

static std::string extractPayloadFromBasicUDP(
    const std::string &packetForwarderPacket,
    const KEY128 &appSKey
) {
    MessageTaskDispatcher d;
    GatewayBasicUdpProtocol p(&d);
    ParseResult pr;
    TASK_TIME receivedTime = std::chrono::system_clock::now();
    p.parse(pr, packetForwarderPacket.c_str(), packetForwarderPacket.size(), receivedTime);
    pr.gwPushData.rxData.decode(*pr.gwPushData.rxData.getAddr(), appSKey);
    return pr.gwPushData.rxData.payloadString();
}

int main(int argc, char **argv) {
    KEY128 appSKey("35003a003434383531374712656b7f47");
    std::string s = decodePayload(
        hex2string("403003450180430002dc6235fbb112f4c8a9d398c29684a814deb370a782c70f4cb1612fe1"), appSKey);
    std::cout << hexString(s) << std::endl;
    if (s != hex2string("01002180549c6118000000004a0000000000000000000000"))
        return 1;

    std::string basicUDPHex("02bbe50000006cc3743eed467b227278706b223a5b7b22746d7374223a343032333131313534302c226368616e223a332c2272666368223a302c2266726571223a3836342e3730303030302c2273746174223a312c226d6f6475223a224c4f5241222c2264617472223a22534631324257313235222c22636f6472223a22342f35222c226c736e72223a2d31382e352c2272737369223a2d3132312c2273697a65223a33372c2264617461223a22514441445251474151774143334749312b374553394d697030356a436c6f536f464e367a634b65437877394d7357457634513d3d227d5d7d");
    s = extractPayloadFromBasicUDP(hex2string(basicUDPHex), appSKey);
    std::cout << hexString(s) << std::endl;
    s = decodePayload(s, appSKey);
    std::cout << hexString(s) << std::endl;
    if (s != hex2string("01002180549c6118000000004a0000000000000000000000"))
        return 2;
}
