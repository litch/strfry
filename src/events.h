#pragma once

#include <secp256k1_schnorrsig.h>

#include "golpe.h"

#include "constants.h"




inline bool isReplaceableEvent(uint64_t kind) {
    return (
        kind == 0 ||
        kind == 3 ||
        kind == 41 ||
        (kind >= 10'000 && kind < 20'000)
    );
}

inline bool isEphemeralEvent(uint64_t kind) {
    return (
        (kind >= 20'000 && kind < 30'000)
    );
}




std::string nostrJsonToFlat(const tao::json::value &v);
std::string nostrHash(const tao::json::value &origJson);

bool verifySig(secp256k1_context* ctx, std::string_view sig, std::string_view hash, std::string_view pubkey);
void verifyNostrEvent(secp256k1_context *secpCtx, const NostrIndex::Event *flat, const tao::json::value &origJson);
void verifyNostrEventJsonSize(std::string_view jsonStr);
void verifyEventTimestamp(const NostrIndex::Event *flat);

void parseAndVerifyEvent(const tao::json::value &origJson, secp256k1_context *secpCtx, bool verifyMsg, bool verifyTime, std::string &flatStr, std::string &jsonStr);


// Does not do verification!
inline const NostrIndex::Event *flatStrToFlatEvent(std::string_view flatStr) {
    return flatbuffers::GetRoot<NostrIndex::Event>(flatStr.data());
}


std::optional<defaultDb::environment::View_Event> lookupEventById(lmdb::txn &txn, std::string_view id);
uint64_t getMostRecentEventId(lmdb::txn &txn);
std::string_view getEventJson(lmdb::txn &txn, uint64_t quadId);

inline quadrable::Key flatEventToQuadrableKey(const NostrIndex::Event *flat) {
    return quadrable::Key::fromIntegerAndHash(flat->created_at(), sv(flat->id()).substr(0, 23));
}






enum class EventWriteStatus {
    Pending,
    Written,
    Duplicate,
    Replaced,
    Deleted,
};


struct EventToWrite {
    std::string flatStr;
    std::string jsonStr;
    uint64_t receivedAt;
    void *userData = nullptr;
    quadrable::Key quadKey;
    uint64_t nodeId = 0;
    EventWriteStatus status = EventWriteStatus::Pending;

    EventToWrite() {}

    EventToWrite(std::string flatStr, std::string jsonStr, uint64_t receivedAt, void *userData = nullptr) : flatStr(flatStr), jsonStr(jsonStr), receivedAt(receivedAt), userData(userData) {
        const NostrIndex::Event *flat = flatbuffers::GetRoot<NostrIndex::Event>(flatStr.data());
        quadKey = flatEventToQuadrableKey(flat);
    }
};


void writeEvents(lmdb::txn &txn, quadrable::Quadrable &qdb, std::vector<EventToWrite> &evs);
