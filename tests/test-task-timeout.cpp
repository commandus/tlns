/**
 * Check task queue
 */
#include <iostream>
#include <thread>
#include <future>

#include "lorawan/task/message-task-dispatcher.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/proto/gw/basic-udp.h"
#include "gen/regional-parameters-3.h"
#include "lorawan/task/task-udp-control-socket.h"

int main(
	int argc,
	char *argv[]
)
{
    MessageTaskDispatcher dispatcher;
    uint64_t gatewayId = 1;
    int r = 0;
    ProtoGwParser *parser = new GatewayBasicUdpProtocol(&dispatcher);
    dispatcher.addParser(parser);

    dispatcher.onPushData = [] (
        MessageTaskDispatcher* dispatcher,
        MessageQueueItem *item
    ) {
        std::cout
            << "{\"metadata\": [";
        bool f = true;
        for (auto & it : item->metadata) {
            if (f)
                f = false;
            else
                std::cout << ", ";
            std::cout << "{\"gateway_id\": " << gatewayId2str(it.first)
                << ", \"sock_addr\": " << sockaddr2string(&it.second.addr)
                << ", \"metadata\": " << SEMTECH_PROTOCOL_METADATA_RX2string(it.second.rx) << "}";
        }
        std::cout
            << "],\n\"rfm\": "
            << item->radioPacket.toString()
            << "}" << std::endl;
    };

    dispatcher.onPullResp = [] (
        MessageTaskDispatcher* dispatcher,
        GwPullResp &item
    ) {
        std::cout
            << "{\"metadata\": "
            << SEMTECH_PROTOCOL_METADATA_TX2string(item.txMetadata)
            << ", \"gwId\": " << gatewayId2str(item.gwId.u)
            << ", \"txData\": " << item.txData.toString()
            << "}" << std::endl;
    };

    dispatcher.onTxPkAck = [] (
            MessageTaskDispatcher* dispatcher,
            ERR_CODE_TX code
    ) {
        std::cout
                << R"({"txPkAck": ")"
                << ERR_CODE_TX2string(code)
                << "\"}" << std::endl;
    };

    // set selected regional parameters e.g. frequency
    dispatcher.setRegionalParameterChannelPlan(regionalParameterChannelPlanMem.get(1));
    dispatcher.setControlSocket(new TaskUDPControlSocket("127.0.0.1:4242"));
    dispatcher.start();

    auto futureWait5s = std::async(std::launch::async, []() {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    });

    while (futureWait5s.wait_for(std::chrono::milliseconds(100)) != std::future_status::ready) {
        std::cout << "." << std::flush;
        std::string l = "024c7e0000006cc3743eed467b227278706b223a5b7b22746d7374223a313237353533303937322c226368616e223a362c2272666368223a312c2266726571223a3836382e3930303030302c2273746174223a312c226d6f6475223a224c4f5241222c2264617472223a22534631324257313235222c22636f6472223a22342f35222c226c736e72223a2d392e352c2272737369223a2d3131352c2273697a65223a33372c2264617461223a2251444144525147416e5259436b4c72715672703677324a55547958744a4467315669464a354d44666b756e336f762f5653513d3d227d2c7b22746d7374223a313237353533303938302c226368616e223a342c2272666368223a302c2266726571223a3836342e3930303030302c2273746174223a312c226d6f6475223a224c4f5241222c2264617472223a22534631324257313235222c22636f6472223a22342f35222c226c736e72223a31302e382c2272737369223a2d32372c2273697a65223a33372c2264617461223a2251444144525147416e5259436b4c72715672703677324a55547958744a4467315669464a354d44666b756e336f762f5653513d3d227d5d7d";
        auto t = std::chrono::system_clock::now() + std::chrono::seconds(1);
        TaskDescriptor taskDescriptor;    // contain time to send, NetworkIdentity and best gateway address
        uint8_t fport = 2;
        const void *payload = nullptr; // up to 255 bytes, can be NULL
        uint8_t payloadSize = 0;
        const void *fopts = nullptr; // up to 15 bytes, can be NULL
        uint8_t foptsSize = 0;
        DownlinkMessageBuilder msg(taskDescriptor, fport, payload, payloadSize, fopts, foptsSize);
        NetworkIdentity networkIdentity;
        dispatcher.queue.putDownlink(t, msg, networkIdentity, gatewayId, parser);
        dispatcher.queue.printStateDebug(std::cout, std::chrono::system_clock::now());
    }

    dispatcher.stop();
    return r;
}
