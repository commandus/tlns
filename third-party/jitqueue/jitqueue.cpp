/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2019 Semtech

Description:
    LoRa concentrator : Just In Time TX scheduling queue

License: Revised BSD License, see LICENSE.Semtech.txt file include in the project
*/

/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */

#include <mutex>
#include <algorithm>
#include <iostream>
#include <cassert>
#include "jitqueue.h"

#define DEBUG_JIT   0

/* -------------------------------------------------------------------------- */
/* --- PRIVATE MACROS ------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS & TYPES -------------------------------------------- */
#define TX_START_DELAY          1500    /* microseconds */
#define TX_MARGIN_DELAY         1000    /* Packet overlap margin in microseconds */
#define TX_JIT_DELAY            40000   /* Pre-delay to program packet for TX in microseconds */
#define TX_MAX_ADVANCE_DELAY    ((JIT_NUM_BEACON_IN_QUEUE + 1) * 128 * 1E6) /* Maximum advance delay accepted for a TX packet, compared to current time */

#define BEACON_GUARD            3000000 /* Interval where no ping slot can be placed,
                                            to ensure beacon can be sent */
#define BEACON_RESERVED         2120000 /* Time on air of the beacon, with some margin */

/* -------------------------------------------------------------------------- */
/* --- PRIVATE VARIABLES (GLOBAL) ------------------------------------------- */
static std::mutex mx_jit_queue;

/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS DEFINITION ----------------------------------------- */

/* -------------------------------------------------------------------------- */
/* --- PUBLIC FUNCTIONS DEFINITION ----------------------------------------- */

C_CALL bool jit_queue_is_full(struct jit_queue_s *queue) {
    bool result;
    mx_jit_queue.lock();
    result = (queue->num_pkt == JIT_QUEUE_MAX)?true:false;

    mx_jit_queue.unlock();

    return result;
}

C_CALL bool jit_queue_is_empty(struct jit_queue_s *queue) {
    bool result;
    mx_jit_queue.lock();
    result = (queue->num_pkt == 0)?true:false;
    mx_jit_queue.unlock();
    return result;
}

C_CALL void jit_queue_init(struct jit_queue_s *queue) {
    int i;

    mx_jit_queue.lock();

    memset(queue, 0, sizeof(*queue));
    for (i=0; i<JIT_QUEUE_MAX; i++) {
        queue->nodes[i].pre_delay = 0;
        queue->nodes[i].post_delay = 0;
    }

    mx_jit_queue.unlock();
}

C_CALL void jit_sort_queue(struct jit_queue_s *queue) {
    if (queue->num_pkt == 0)
        return;
    std::cout <<"sorting queue in ascending order packet timestamp - queue size: " << (int) queue->num_pkt << std::endl;
    std::sort(queue->nodes, queue->nodes + queue->num_pkt, [=] (const struct jit_node_s &a, const struct jit_node_s &b) {
        return a.pkt.count_us < b.pkt.count_us;
    });
    std::cout <<"sorting queue done" << std::endl;
}

C_CALL bool jit_collision_test(uint32_t p1_count_us, uint32_t p1_pre_delay, uint32_t p1_post_delay, uint32_t p2_count_us, uint32_t p2_pre_delay, uint32_t p2_post_delay) {
    if (((p1_count_us - p2_count_us) <= (p1_pre_delay + p2_post_delay + TX_MARGIN_DELAY)) ||
        ((p2_count_us - p1_count_us) <= (p2_pre_delay + p1_post_delay + TX_MARGIN_DELAY))) {
        return true;
    } else {
        return false;
    }
}

int printRFM2json(
    std::ostream &ss,
    const uint8_t *payload,
    uint16_t size
);

C_CALL enum jit_error_e jit_enqueue(struct jit_queue_s *queue, uint32_t time_us, struct lgw_pkt_tx_s *packet, enum jit_pkt_type_e pkt_type) {
    int i = 0;
    uint32_t packet_post_delay = 0;
    uint32_t packet_pre_delay = 0;
    uint32_t target_pre_delay = 0;
    enum jit_error_e err_collision;
    uint32_t asap_count_us;

    std::cout <<"Current concentrator time is " << time_us << " pkt_type: " << (int) pkt_type << std::endl;


    if (packet == NULL) {
        std::cerr <<"ERROR: invalid parameter" << std::endl;
        return JIT_ERROR_INVALID;
    }

    if (jit_queue_is_full(queue)) {
        std::cerr <<"ERROR: cannot enqueue packet, JIT queue is full" << std::endl;
        return JIT_ERROR_FULL;
    }

    /* Compute packet pre/post delays depending on packet's type */
    switch (pkt_type) {
        case JIT_PKT_TYPE_DOWNLINK_CLASS_A:
        case JIT_PKT_TYPE_DOWNLINK_CLASS_B:
        case JIT_PKT_TYPE_DOWNLINK_CLASS_C:
            packet_pre_delay = TX_START_DELAY + TX_JIT_DELAY;
            packet_post_delay = lgw_time_on_air(packet) * 1000UL; /* in us */
            break;
        case JIT_PKT_TYPE_BEACON:
            /* As defined in LoRaWAN spec */
            packet_pre_delay = TX_START_DELAY + BEACON_GUARD + TX_JIT_DELAY;
            packet_post_delay = BEACON_RESERVED;
            break;
        default:
            break;
    }

    mx_jit_queue.lock();

    /* An immediate downlink becomes a timestamped downlink "ASAP" */
    /* Set the packet count_us to the first available slot */
    if (pkt_type == JIT_PKT_TYPE_DOWNLINK_CLASS_C) {
        /* change tx_mode to timestamped */
        packet->tx_mode = TIMESTAMPED;

        /* Search for the ASAP timestamp to be given to the packet */
        asap_count_us = time_us + 2 * TX_JIT_DELAY; /* margin */
        if (queue->num_pkt == 0) {
            /* If the jit queue is empty, we can insert this packet */
            std::cerr <<"DEBUG: insert IMMEDIATE downlink, first in JiT queue, count_us: " << asap_count_us << std::endl;
        } else {
            /* Else we can try to insert it:
                - ASAP meaning NOW + MARGIN
                - at the last index of the queue
                - between 2 downlinks in the queue
            */

            /* First, try if the ASAP time collides with an already enqueued downlink */
            for (i=0; i<queue->num_pkt; i++) {
                if (jit_collision_test(asap_count_us, packet_pre_delay, packet_post_delay, queue->nodes[i].pkt.count_us, queue->nodes[i].pre_delay, queue->nodes[i].post_delay) == true) {
                    std::cerr <<"DEBUG: cannot insert IMMEDIATE downlink at count_us= " << asap_count_us
                                << " collides with " << queue->nodes[i].pkt.count_us
                                << "(index = " << i << std::endl;
                    break;
                }
            }
            if (i == queue->num_pkt) {
                /* No collision with ASAP time, we can insert it */
                std::cerr << "DEBUG: insert IMMEDIATE downlink ASAP at " << asap_count_us << " (no collision)" << std::endl;
            } else {
                /* Search for the best slot then */
                for (i=0; i<queue->num_pkt; i++) {
                    asap_count_us = queue->nodes[i].pkt.count_us + queue->nodes[i].post_delay + packet_pre_delay + TX_JIT_DELAY + TX_MARGIN_DELAY;
                    if (i == (queue->num_pkt - 1)) {
                        /* Last packet index, we can insert after this one */
                        std::cerr << "DEBUG: insert IMMEDIATE downlink, last in JiT queue" << asap_count_us << std::endl;
                    } else {
                        /* Check if packet can be inserted between this index and the next one */
                        std::cerr << "DEBUG: try to insert IMMEDIATE downlink (count_us=" << asap_count_us
                                     << ") between index " << i
                                     << " and index " << i + 1
                                     << "?" << std::endl;
                        if (jit_collision_test(asap_count_us, packet_pre_delay, packet_post_delay, queue->nodes[i+1].pkt.count_us, queue->nodes[i+1].pre_delay, queue->nodes[i+1].post_delay) == true) {
                            std::cerr << "DEBUG: failed to insert IMMEDIATE downlink (count_us="
                                         << asap_count_us << "), continue..." << std::endl;
                            continue;
                        } else {
                            std::cout << "DEBUG: insert IMMEDIATE downlink (count_us="
                                      << asap_count_us << "), continue..." << std::endl;
                            break;
                        }
                    }
                }
            }
        }
        /* Set packet with ASAP timestamp */
        packet->count_us = asap_count_us;
    }

    /* Check criteria_1: is it already too late to send this packet ?
     *  The packet should arrive at least at (tmst - TX_START_DELAY) to be programmed into concentrator
     *  Note: - Also add some margin, to be checked how much is needed, if needed
     *        - Valid for both Downlinks and Beacon packets
     *
     *  Warning: unsigned arithmetic (handle roll-over)
     *      t_packet < t_current + TX_START_DELAY + MARGIN
     */
    if ((packet->count_us - time_us) <= (TX_START_DELAY + TX_MARGIN_DELAY + TX_JIT_DELAY)) {
        std::cerr << "ERROR: Packet REJECTED, already too late to send it (current=" << time_us
            << ", packet=" << packet->count_us
            <<", type=" << (int) pkt_type
            << asap_count_us << "), continue..." << std::endl;
        mx_jit_queue.unlock();
        return JIT_ERROR_TOO_LATE;
    }

    /* Check criteria_2: Does packet timestamp seem plausible compared to current time
     *  We do not expect the server to program a downlink too early compared to current time
     *  Class A: downlink has to be sent in a 1s or 2s time window after RX
     *  Class B: downlink has to occur in a 128s time window
     *  Class C: no check needed, departure time has been calculated previously
     *  So let's define a safe delay above which we can say that the packet is out of bound: TX_MAX_ADVANCE_DELAY
     *  Note: - Valid for Downlinks only, not for Beacon packets
     *
     *  Warning: unsigned arithmetic (handle roll-over)
                t_packet > t_current + TX_MAX_ADVANCE_DELAY
     */
    if ((pkt_type == JIT_PKT_TYPE_DOWNLINK_CLASS_A) || (pkt_type == JIT_PKT_TYPE_DOWNLINK_CLASS_B)) {
        if ((packet->count_us - time_us) > TX_MAX_ADVANCE_DELAY) {
            std::cerr << "\"ERROR: Packet REJECTED, timestamp seems wrong, too much in advance (current=" << time_us
                      << ", packet=)" << packet->count_us
                      <<", type=" << (int) pkt_type
                      << asap_count_us << "), continue..." << std::endl;
            mx_jit_queue.unlock();
            return JIT_ERROR_TOO_EARLY;
        }
    }

    /* Check criteria_3: does this new packet overlap with a packet already enqueued ?
     *  Note: - need to take into account packet's pre_delay and post_delay of each packet
     *        - Valid for both Downlinks and beacon packets
     *        - Beacon guard can be ignored if we try to queue a Class A downlink
     */
    for (i=0; i<queue->num_pkt; i++) {
        /* We ignore Beacon Guard for Class A/C downlinks */
        if (((pkt_type == JIT_PKT_TYPE_DOWNLINK_CLASS_A) || (pkt_type == JIT_PKT_TYPE_DOWNLINK_CLASS_C)) && (queue->nodes[i].pkt_type == JIT_PKT_TYPE_BEACON)) {
            target_pre_delay = TX_START_DELAY;
        } else {
            target_pre_delay = queue->nodes[i].pre_delay;
        }

        /* Check if there is a collision
         *  Warning: unsigned arithmetic (handle roll-over)
         *      t_packet_new - pre_delay_packet_new < t_packet_prev + post_delay_packet_prev (OVERLAP on post delay)
         *      t_packet_new + post_delay_packet_new > t_packet_prev - pre_delay_packet_prev (OVERLAP on pre delay)
         */
        if (jit_collision_test(packet->count_us, packet_pre_delay, packet_post_delay, queue->nodes[i].pkt.count_us, target_pre_delay, queue->nodes[i].post_delay) == true) {
            switch (queue->nodes[i].pkt_type) {
                case JIT_PKT_TYPE_DOWNLINK_CLASS_A:
                case JIT_PKT_TYPE_DOWNLINK_CLASS_B:
                case JIT_PKT_TYPE_DOWNLINK_CLASS_C:
                    std::cerr << "ERROR: Packet (type=" << (int) pkt_type << ") REJECTED, collision with packet already programmed at " << queue->nodes[i].pkt.count_us
                        << ", (" << packet->count_us << std::endl;

                    err_collision = JIT_ERROR_COLLISION_PACKET;
                    break;
                case JIT_PKT_TYPE_BEACON:
                    if (pkt_type != JIT_PKT_TYPE_BEACON) {
                        /* do not overload logs for beacon/beacon collision, as it is expected to happen with beacon pre-scheduling algorith used */
                        std::cerr << "ERROR: Packet (type=" << (int) pkt_type << ") REJECTED, collision with beacon already programmed at "
                            << queue->nodes[i].pkt.count_us << ", (" << packet->count_us << std::endl;
                    }
                    err_collision = JIT_ERROR_COLLISION_BEACON;
                    break;
                default:
                    std::cerr << "ERROR: Unknown packet type, should not occur, BUG?" << std::endl;
                    assert(0);
                    break;
            }
            mx_jit_queue.unlock();
            return err_collision;
        }
    }

    /* Finally enqueue it */
    /* Insert packet at the end of the queue */
    memcpy(&(queue->nodes[queue->num_pkt].pkt), packet, sizeof(struct lgw_pkt_tx_s));
    queue->nodes[queue->num_pkt].pre_delay = packet_pre_delay;
    queue->nodes[queue->num_pkt].post_delay = packet_post_delay;
    queue->nodes[queue->num_pkt].pkt_type = pkt_type;
    if (pkt_type == JIT_PKT_TYPE_BEACON) {
        queue->num_beacon++;
    }
    queue->num_pkt++;
    /* Sort the queue in ascending order of packet timestamp */
    jit_sort_queue(queue);

    /* Done */
    mx_jit_queue.unlock();

    jit_print_queue(queue, false, DEBUG_JIT);

    std::cout << "enqueued packet with count_us=" << packet->count_us
            << " (size " << packet->size << " bytes, toa = " << packet_post_delay
            << " type= " << (int) pkt_type << std::endl;
    for (int i = 0; i < 27 + packet->size; i++) {
        printf("%02x", ((uint8_t *)packet )[i]);
    }
    std::cout << std::endl;
    printRFM2json(std::cout, (uint8_t *) packet, 27 + packet->size);
    std::cout << std::endl;

    return JIT_ERROR_OK;
}

C_CALL enum jit_error_e jit_dequeue(struct jit_queue_s *queue, int index, struct lgw_pkt_tx_s *packet, enum jit_pkt_type_e *pkt_type) {
    if (packet == NULL) {
        std::cerr << "ERROR: invalid parameter" << std::endl;
        return JIT_ERROR_INVALID;
    }

    if ((index < 0) || (index >= JIT_QUEUE_MAX)) {
        std::cerr << "ERROR: invalid parameter" << std::endl;
        return JIT_ERROR_INVALID;
    }

    if (jit_queue_is_empty(queue)) {
        std::cerr << "ERROR: cannot dequeue packet, JIT queue is empty" << std::endl;
        return JIT_ERROR_EMPTY;
    }

    mx_jit_queue.lock();

    /* Dequeue requested packet */
    memcpy(packet, &(queue->nodes[index].pkt), sizeof(struct lgw_pkt_tx_s));
    queue->num_pkt--;
    *pkt_type = queue->nodes[index].pkt_type;
    if (*pkt_type == JIT_PKT_TYPE_BEACON) {
        queue->num_beacon--;
        std::cout << "--- Beacon dequeued ---" << std::endl;
    }

    /* Replace dequeued packet with last packet of the queue */
    memcpy(&(queue->nodes[index]), &(queue->nodes[queue->num_pkt]), sizeof(struct jit_node_s));
    memset(&(queue->nodes[queue->num_pkt]), 0, sizeof(struct jit_node_s));

    /* Sort queue in ascending order of packet timestamp */
    jit_sort_queue(queue);

    /* Done */
    mx_jit_queue.unlock();

    jit_print_queue(queue, false, DEBUG_JIT);

    std::cout << "dequeued packet with count_us=" << packet->count_us << " from index " << index << std::endl;
    for (int i = 0; i < 27 + packet->size; i++) {
        printf("%02x", ((uint8_t *)packet )[i]);
    }
    std::cout << std::endl;
    printRFM2json(std::cout, (uint8_t *) packet, 27 + packet->size);
    std::cout << std::endl;

    return JIT_ERROR_OK;
}

C_CALL enum jit_error_e jit_peek(struct jit_queue_s *queue, uint32_t time_us, int *pkt_idx) {
    /* Return index of node containing a packet inline with given time */
    int i = 0;
    int idx_highest_priority = -1;
    if (pkt_idx == NULL) {
        std::cerr << "ERROR: invalid parameter" << std::endl;
        return JIT_ERROR_INVALID;
    }

    if (jit_queue_is_empty(queue)) {
        return JIT_ERROR_EMPTY;
    }

    mx_jit_queue.lock();

    /* Search for highest priority packet to be sent */
    for (i=0; i<queue->num_pkt; i++) {
        /* First check if that packet is outdated:
         *  If a packet seems too much in advance, and was not rejected at enqueue time,
         *  it means that we missed it for peeking, we need to drop it
         *
         *  Warning: unsigned arithmetic
         *      t_packet > t_current + TX_MAX_ADVANCE_DELAY
         */
        if ((queue->nodes[i].pkt.count_us - time_us) >= TX_MAX_ADVANCE_DELAY) {
            /* We drop the packet to avoid lock-up */
            queue->num_pkt--;
            if (queue->nodes[i].pkt_type == JIT_PKT_TYPE_BEACON) {
                queue->num_beacon--;
                std::cerr << "WARNING: --- Beacon dropped (current_time=" << time_us
                    << ", packet_time=" << queue->nodes[i].pkt.count_us
                    << ") ---" << std::endl;
            } else {
                std::cerr << "WARNING: --- Packet dropped (current_time=" << time_us
                    << ", packet_time=" << queue->nodes[i].pkt.count_us << std::endl;
            }

            /* Replace dropped packet with last packet of the queue */
            memcpy(&(queue->nodes[i]), &(queue->nodes[queue->num_pkt]), sizeof(struct jit_node_s));
            memset(&(queue->nodes[queue->num_pkt]), 0, sizeof(struct jit_node_s));

            /* Sort queue in ascending order of packet timestamp */
            jit_sort_queue(queue);

            /* restart loop  after purge to find packet to be sent */
            i = 0;
            continue;
        }

        /* Then look for highest priority packet to be sent:
         *  Warning: unsigned arithmetic (handle roll-over)
         *      t_packet < t_highest
         */
        if ((idx_highest_priority == -1) || (((queue->nodes[i].pkt.count_us - time_us) < (queue->nodes[idx_highest_priority].pkt.count_us - time_us)))) {
            idx_highest_priority = i;
        }
    }

    /* Peek criteria 1: look for a packet to be sent in next TX_JIT_DELAY ms timeframe
     *  Warning: unsigned arithmetic (handle roll-over)
     *      t_packet < t_current + TX_JIT_DELAY
     */
    if ((queue->nodes[idx_highest_priority].pkt.count_us - time_us) < TX_JIT_DELAY) {
        *pkt_idx = idx_highest_priority;
        std::cout << "peek packet with count_us=" << queue->nodes[idx_highest_priority].pkt.count_us << " at index " << idx_highest_priority << std::endl;
    } else {
        *pkt_idx = -1;
    }

    mx_jit_queue.unlock();

    return JIT_ERROR_OK;
}

C_CALL void jit_print_queue(struct jit_queue_s *queue, bool show_all, int debug_level) {
    int i = 0;
    int loop_end;

    if (jit_queue_is_empty(queue)) {
        std::cout << "INFO: [jit] queue is empty" << std::endl;

    } else {
        mx_jit_queue.lock();
        std::cout << "INFO: [jit] queue contains " << (int) queue->num_pkt << " packets" << std::endl;
        std::cout << "INFO: [jit] queue contains " << (int) queue->num_beacon << " beacons" << std::endl;
        loop_end = (show_all == true) ? JIT_QUEUE_MAX : queue->num_pkt;
        for (i=0; i<loop_end; i++) {
            std::cout << " - node[" << i << "]: count_us = " << queue->nodes[i].pkt.count_us
                << " - type=" << (int) queue->nodes[i].pkt_type  << std::endl;
        }
        mx_jit_queue.unlock();
    }
}
