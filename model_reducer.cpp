//
// Created by giacomo on 22/08/23.
//

#include "model_reducer.h"
#include "yaucl/graphs/FlexibleFA.h"


#define BINARY_PRESENCE     (0b01)
#define BINARY_ABSENCE    (0b10)
#define BINARY_EXCL     (0b11)

// SR1
#define EXCLUDE_FROM_EXISTANCE(cf) do { auto it2 = Future.emplace(cf, BINARY_ABSENCE);\
if ((!it2.second) && ((it2.first->second & BINARY_ABSENCE) != BINARY_ABSENCE)) {\
return {};  /* Inconsistent model, as both clauses coexist */\
}} while(0)
#define EXCLUDE_FROM_EXISTANCE2(cf) do { auto it2 = Future.emplace(cf, BINARY_ABSENCE);\
if ((!it2.second) && ((it2.first->second & BINARY_ABSENCE) != BINARY_ABSENCE)) {\
return false;  /* Inconsistent model, as both clauses coexist */\
}} while(0)

template <typename T>
static inline void exclude_from_map(std::unordered_map<std::string, T>& map, const std::string& elem) {
    auto it = map.find(elem);
    if (it != map.end()) {
        map.erase(it);
    }
}

static inline bool exclude_from_precedence_map(std::unordered_map<std::string, std::unordered_set<std::string>>& map,
                                               const std::string& elem,
                                               std::unordered_map<std::string, char>& Future) {
    auto it = map.find(elem);
    if (it != map.end()) {
        for (const auto& b : it->second) {
            auto it2 = Future.emplace(b, BINARY_ABSENCE);
            if ((!it2.second) && ((it2.first->second & BINARY_ABSENCE) != BINARY_ABSENCE))
                return true;
        }
        map.erase(it);
    }
    return false;
}

#define EXCLUDE_FROM_ALL_MAPS(cf)  do { exclude_from_map(MNext, cf);\
exclude_from_map(Malt_response, cf);\
exclude_from_map(Mneg_chainsuccession, cf);                          \
exclude_from_map(MNext, cf);                          \
exclude_from_map(MFuture, cf);                          \
if (exclude_from_precedence_map(Mprecedence, cf, Future)) return {}; \
} while(0)

#define EXCLUDE_FROM_ALL_MAPS2(cf)  do { exclude_from_map(MNext, cf);\
exclude_from_map(Malt_response, cf);\
exclude_from_map(Mneg_chainsuccession, cf);                          \
exclude_from_map(MNext, cf);                          \
exclude_from_map(MFuture, cf);                                       \
if (exclude_from_precedence_map(Mprecedence, cf, Future)) return false; \
} while(0)

#define EXCLUDE_FROM_NOTMALT_MAPS(cf)  do { exclude_from_map(MNext, cf); \
exclude_from_map(Mneg_chainsuccession, cf);                          \
exclude_from_map(MNext, cf);                          \
exclude_from_map(MFuture, cf);                          \
if (exclude_from_precedence_map(Mprecedence, cf, Future)) return {}; \
} while(0)


static inline bool test_future_condition(const std::unordered_map<std::string, char>& Future, const std::string& label, char condition) {
    auto it = Future.find(label);
    if (it == Future.end()) return false;
    return ((it->second & condition) == condition);
}

template <typename K, typename V>
bool dfsFirstLoopVisit(roaring::Roaring64Map& visited,
                       FlexibleFA<K, V>& graph,
                       size_t x,
                       std::vector<size_t> &stack_array,
                       std::vector<size_t> &out) {
    visited.add(x);
    stack_array.emplace_back(x);
    for (const std::pair<V, size_t>& edgeId : graph.outgoingEdges(x)) {
        if (visited.contains(edgeId.second)) {
            // Checking whether the current element has been visited in the current path
            auto it = std::find(stack_array.begin(), stack_array.end(),edgeId.second);
            if (it != stack_array.end()) {
                // If it is present in the stack, then I found a loop
                out = {it, stack_array.end()};
                return true; // found
            }
            // Otherwise, block the recursive iteration
        } else {
            if (dfsFirstLoopVisit(visited, graph, edgeId.second, stack_array, out))
                return true;
        }
    }
    stack_array.pop_back();
    return false;
}

bool model_reducer::reduce_map_to_be_considered(std::unordered_map<std::string, std::unordered_set<std::string>>& map_to_be_considered) {
    FlexibleFA<std::string, char> crg;
    std::unordered_map<std::string, size_t> node_map;
    for (const auto& [a, targets] : map_to_be_considered) {
        // If the activation shall never happen, the clause is never triggered for checking
        if (test_future_condition(Future, a, BINARY_ABSENCE)) continue;
        DEBUG_ASSERT(targets.size() == 1);
        for (const auto& b : targets) {
            auto itSrc = node_map.find(a);
            if (itSrc == node_map.end()) {
                itSrc = node_map.emplace(a, crg.addNewNodeWithLabel(a)).first;
            }
            auto itDst = node_map.find(b);
            if (itDst == node_map.end()) {
                itDst = node_map.emplace(b, crg.addNewNodeWithLabel(b)).first;
            }
            crg.addNewEdgeFromId(itSrc->second, itDst->second, 0);
        }
    }
    bool foundLoop = false;
    roaring::Roaring64Map visited;
    std::vector<size_t> array, loop;
    size_t maximal_size = crg.vSize()+1;
    // Getting all the distinct cycles
    array.reserve(crg.vSize()+1);
    do {
        foundLoop = false;
        visited.clear();
        for (size_t i : crg.getNodeIds()) {
            if (!visited.contains(i)) {
                if (dfsFirstLoopVisit(visited,
                                      crg,
                                      i,
                                      array, loop)) {
                    for (size_t nodeId : loop) {
                        const auto& label = crg.getNodeLabel(nodeId);
                        EXCLUDE_FROM_EXISTANCE2(label);
                        EXCLUDE_FROM_ALL_MAPS2(label);
                    }
                    for (size_t nodeId : loop) {
                        crg.removeNode(nodeId);
                    }
                    loop.clear();
                    foundLoop = true;
                }
            }
        }
    } while ((crg.vSize() > 0) && foundLoop);
    return true;
}

static inline void result_fill(std::unordered_map<std::string, std::unordered_set<std::string>>& map_to_consider,
                               declare_cases caso,
                               std::vector<DatalessCases>& result) {
    for (const auto& [a, bSet]: map_to_consider) {
        for (const auto& b : bSet)
            result.emplace_back(caso, a, b);
    }
    map_to_consider.clear();
}


bool model_reducer::reduce_cr(std::unordered_map<std::string, std::unordered_set<std::string>>& MN,
                                                    const std::string& label) {
    std::unordered_set<std::string> visited;
    std::queue<std::string> toRemove;
    toRemove.emplace(label);
    while (!toRemove.empty()) {
        auto top = toRemove.front();
        toRemove.pop();
        if (visited.emplace(top).second) {
            EXCLUDE_FROM_EXISTANCE2(top);
            EXCLUDE_FROM_ALL_MAPS2(top);
            for (auto & it2 : MN) {
                if (it2.second.contains(top)) {
                    toRemove.emplace(it2.first);
                }
            }
        }
    }
    return true;
}

std::vector<DatalessCases> model_reducer::run(const std::vector<DatalessCases>& model) {
    clear();

    // Preliminary inconsistency detection with absence/existence, expanding the compound clauses
    // and filling up the vectors/maps
    for (const auto& clause : model) {
        switch (clause.casus) {
            case Existence: {
                auto it = Future.emplace(clause.left, BINARY_PRESENCE);
                if ((!it.second) && ((it.first->second & BINARY_PRESENCE) != BINARY_PRESENCE)) {
                    return {};  // Inconsistent model, as both clauses coexist
                }
            }
                break;
            case Absence: {
                EXCLUDE_FROM_EXISTANCE(clause.left);
            }
                break;
            case Choice:
                if (clause.left < clause.right)
                    choice.emplace_back(clause.left, clause.right);
                else
                    choice.emplace_back(clause.right, clause.left);
                break;
            case ExclChoice:
                if (clause.left < clause.right){
                    choice.emplace_back(clause.left, clause.right);
                    not_coexistence.emplace_back(clause.left, clause.right);
                } else {
                    choice.emplace_back(clause.right, clause.left);
                    not_coexistence.emplace_back(clause.right, clause.left);
                }
                break;
            case RespExistence:
                resp_existence.emplace_back(clause.left, clause.right);
                break;
            case CoExistence:
                resp_existence.emplace_back(clause.left, clause.right);
                resp_existence.emplace_back(clause.right, clause.left);
                break;
            case Response:
                response.emplace_back(clause.left, clause.right);
                break;
            case Precedence:
                Mprecedence[clause.left].emplace(clause.right);
                break;
            case ChainResponse:
                chain_response.emplace_back(clause.left, clause.right);
                break;
            case ChainPrecedence:
                chain_precedence.emplace_back(clause.left, clause.right);
                break;
            case ChainSuccession:
                chain_response.emplace_back(clause.left, clause.right);
                chain_precedence.emplace_back(clause.right, clause.left);
                break;
            case Succession:
                response.emplace_back(clause.left, clause.right);
                Mprecedence[clause.left].emplace(clause.right);
                break;
            case AltPrecedence:
                Mprecedence[clause.left].emplace(clause.right);
                Malt_precedence[clause.right].emplace(clause.left);
                break;
            case AltSuccession:
                Mprecedence[clause.left].emplace(clause.right);
                Malt_precedence[clause.right].emplace(clause.left);
                Malt_response[clause.left].emplace(clause.right);
                break;
            case AltResponse:
                Malt_response[clause.left].emplace(clause.right);
                break;
            case NegSuccession:
                neg_succession.emplace_back(clause.left, clause.right);
                break;
            case NegChainSuccession:
                Mneg_chainsuccession[clause.left].emplace(clause.right);
                break;
            case NotCoexistence:
                if (clause.left < clause.right)
                    not_coexistence.emplace_back(clause.left, clause.right);
                else
                    not_coexistence.emplace_back(clause.right, clause.left);
                break;
        }
    }
    remove_duplicates(not_coexistence);
    remove_duplicates(response);
    remove_duplicates(neg_succession);
    remove_duplicates(chain_response);
    remove_duplicates(chain_precedence);
    remove_duplicates(resp_existence);
    remove_duplicates(choice);
    remove_duplicates(not_coexistence);

    // Detecting G(A -> X(b))
    for (const auto& clause : chain_response) {
        if (test_future_condition(Future, clause.first, BINARY_ABSENCE))
            continue;
        if (test_future_condition(Future, clause.second, BINARY_ABSENCE)) {
            if (reduce_cr(MNext, clause.first))
                continue;
            else
                return {};
        }
        if (clause.first == clause.second) {
            auto it = Malt_response.find(clause.first);
            if (it != Malt_response.end()) {
                if (reduce_cr(MNext, clause.first))
                    continue;
                else
                    return {};
            }
        }
        {
            auto it = Mneg_chainsuccession.find(clause.first);
            if (it != Mneg_chainsuccession.end()) {
                if (it->second.contains(clause.second)) {
                    if (reduce_cr(MNext, clause.first))
                        continue;
                    else
                        return {};
                }
            }
        }
        auto& mnextA = MNext[clause.first];
        mnextA.emplace(clause.second);
        if (mnextA.size() > 1) {
            if (reduce_cr(MNext, clause.first))
                continue;
            else
                return {};
        }
    }
    chain_response.clear();

    // Detecting G(a -> F(b))
    for (const auto& clause : response) {
        if (test_future_condition(Future, clause.first, BINARY_ABSENCE)) continue;
        if (test_future_condition(Future, clause.second, BINARY_ABSENCE)) {
            EXCLUDE_FROM_EXISTANCE(clause.first);
            EXCLUDE_FROM_ALL_MAPS(clause.first);
            continue;
        }
        auto& mfutureA = MFuture[clause.first];
        mfutureA.emplace(false, clause.second);
    }
    response.clear();

    // Detecting G(a -> G(!b))
    std::pair<is_negated, std::string> is_positive{false, ""};
    for (const auto& clause : neg_succession) {
        if (test_future_condition(Future, clause.first, BINARY_ABSENCE))
            continue;
        auto& mfutureA = MFuture[clause.first];
        is_positive.second = clause.second;
        if (mfutureA.contains(is_positive)) {
            EXCLUDE_FROM_EXISTANCE(clause.first);
            EXCLUDE_FROM_ALL_MAPS(clause.first);
            continue;
        }
        {
            auto it = MNext.find(clause.first);
            if (it != MNext.end()) {
                if (it->second.contains(clause.second)) {
                    EXCLUDE_FROM_EXISTANCE(clause.first);
                    EXCLUDE_FROM_ALL_MAPS(clause.first);
                    continue;
                }
            }
        }
        mfutureA.emplace(true, clause.second);
    }
    neg_succession.clear();

    // Declare AltResponse G(a -> X(!a U b))
    std::pair<is_negated, std::string> is_neg{true, ""};
    for (auto it3 = Malt_response.begin(); it3 != Malt_response.end(); ) {
        if (test_future_condition(Future, it3->first, BINARY_ABSENCE)) continue;
        bool badMalt = false;
        for (auto it4 = it3->second.begin(); it4 != it3->second.end(); ) {
            auto it = MNext.find(it3->first);
            if (it != MNext.end()) {
                if (it->second.contains(*it4)) {
                    it4 = it3->second.erase(it4);
                    continue;
                }
            }

            bool hasMFutureANotB = false;
            {
                auto it2 = MFuture.find(it3->first);
                if (it2 != MFuture.end()) {
                    is_neg.second = *it4;
                    hasMFutureANotB = it2->second.contains(is_neg);
                }
            }
            if (hasMFutureANotB || (test_future_condition(Future, *it4, BINARY_ABSENCE))) {
                EXCLUDE_FROM_EXISTANCE(it3->first);
                EXCLUDE_FROM_NOTMALT_MAPS(it3->first);
                badMalt = true;
                it3 = Malt_response.erase(it3);
                break; // Skipping all the it4s
            }

            auto it2 = MFuture.find(it3->first);
            if (it2 != MFuture.end()) {
                is_positive.second = *it4;
                it2->second.erase(is_positive);
            }
            it4++;
        }

        if (!badMalt) it3++;
    }
    // Not erasing: already in map!

    // If AltPrecedence has some rewritings that can be ignored, we just preserve the precedence part
    for (auto it = Malt_precedence.begin(); it != Malt_precedence.end(); it++) {
        if (test_future_condition(Future, it->first, BINARY_ABSENCE)) continue;
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ) {
            {
                auto it3 = MNext.find(it->first);
                if (it3 != MNext.end()) {
                    if (it3->second.contains(*it2)) {
                        it2 = it->second.erase(it2);
                        continue;
                    }
                }
            }
            {
                auto it3 = MFuture.find(it->first);
                if (it3 != MFuture.end()) {
                    is_neg.second = *it2;
                    if (it3->second.contains(is_neg)) {
                        it2 = it->second.erase(it2);
                        continue;
                    }
                }
            }
        }
    }
    // Not erasing: already in map!

    // TODO: Precedence, ChainResponse, Response --> Absences
    if (!reduce_map_to_be_considered(Mprecedence)) return {};
    {
        std::unordered_map<std::string, std::unordered_set<std::string>> intermediate;
        for (auto it2 = MFuture.begin(); it2 != MFuture.end(); ) {
            const auto& k = it2->first;
            auto& vals = it2->second;
            for (auto it = vals.begin(); it != vals.end(); ) {
                if (it->first)
                    it++;
                else {
                    intermediate[k].emplace(it->second);
                    it = vals.erase(it);
                }
            }
            if (vals.empty())
                it2 = MFuture.erase(it2);
            else
                it2++;
        }
        if (!reduce_map_to_be_considered(intermediate)) return {};
        for (const auto& [k, vals] : intermediate) {
            for (const auto& b : vals)
                MFuture[k].emplace(false, b);
        }
    }
    if (!reduce_map_to_be_considered(MNext)) return {};
    // TODO: Precedence, ChainResponse, Response --> Absences

    std::vector<DatalessCases> result;
    for (const auto& clause : resp_existence) {
        // If the activation shall never happen, the clause is never triggered for checking
        if (test_future_condition(Future, clause.first, BINARY_ABSENCE)) continue;

        if (test_future_condition(Future, clause.first, BINARY_PRESENCE)) {
            // If the clause is activated and the response shall never happen, then I have an inconsistency
            if (test_future_condition(Future, clause.second, BINARY_ABSENCE)) {
                return {}; // Inconsistent model, as we prescribe that b shall never happen
            } else {
                // Otherwise, enforce this element to appear
                Future.emplace(clause.second, BINARY_PRESENCE);
                continue;
            }
        }

        // Otherwise, if it is not prescribed to be activated, then I shall declare the
        // clause to be []!a
        if (test_future_condition(Future, clause.second, BINARY_ABSENCE)) {
            EXCLUDE_FROM_EXISTANCE(clause.first);
            EXCLUDE_FROM_ALL_MAPS(clause.first);
            continue;
        }
        result.emplace_back(RespExistence, clause.first, clause.second);
//        MResp_exsistence[clause.first].emplace(clause.second);
    }
    resp_existence.clear();

    // Detecting inconsistent models through not_coexistence
    for (const auto& clause : not_coexistence) {
        if ((test_future_condition(Future, clause.first, BINARY_PRESENCE)) &&
            (test_future_condition(Future, clause.second, BINARY_PRESENCE))) {
            return {}; // Inconsistent model, as both clauses coexist
        }
    }
    not_coexistence.clear();
    DEBUG_ASSERT(not_coexistence.empty());
    DEBUG_ASSERT(chain_response.empty());
    DEBUG_ASSERT(response.empty());
    for (const auto& clause : choice) {
        if (test_future_condition(Future, clause.first, BINARY_PRESENCE) ||
            test_future_condition(Future, clause.second, BINARY_PRESENCE))
            continue;
        if ((test_future_condition(Future, clause.first, BINARY_ABSENCE)) &&
            (test_future_condition(Future, clause.second, BINARY_ABSENCE))) {
            return {}; // Inconsistent model, as both clauses coexist
        }
        result.emplace_back(Choice, clause.first, clause.second);
    }
    choice.clear();
    DEBUG_ASSERT(choice.empty());
    // neg_succession --> MFuture
    DEBUG_ASSERT(resp_existence.clear());
    for (const auto& clause : chain_precedence) {
        result.emplace_back(ChainPrecedence, clause.first, clause.second);
    }
    chain_precedence.clear();
    result_fill(MNext, ChainResponse, result);
    DEBUG_ASSERT(MNext.empty());
    DEBUG_ASSERT(chain_precedence.empty());
    for (const auto& [a, bSet]: MFuture) {
        for (const auto& [negated, b] : bSet) {
            if (negated) {
                result.emplace_back(NegSuccession, a, b);
            } else {
                result.emplace_back(Response, a, b);
            }
        }
    }
    MFuture.clear();
    result_fill(Malt_response, AltResponse, result);
    DEBUG_ASSERT(Malt_response.empty());
    for (const auto& [b, aSet]: Malt_precedence) {
        if (test_future_condition(Future, b, BINARY_ABSENCE)) {
            for (const auto& a : aSet) {
                result.emplace_back(AltPrecedence, a, b);
            }
        } else {
            for (const auto& a : aSet) {
                auto it = Mprecedence.find(a);
                if (it != Mprecedence.end()) {
                    if (it->second.contains(b)) {
                        result.emplace_back(AltPrecedence, a, b);
                    } else {
                        throw std::runtime_error("LOGICAL ERROR (2)!");
                    }
                } else {
                    throw std::runtime_error("LOGICAL ERROR (1)!");
                }
            }
        }
    }
    Malt_precedence.clear();
    result_fill(Mneg_chainsuccession, NegChainSuccession, result);
    DEBUG_ASSERT(Mneg_chainsuccession.empty());
    for (const auto& [act, char_] : Future) {
        if ((char_ & BINARY_PRESENCE))
            result.emplace_back(Existence, act, "");
        else if ((char_ & BINARY_ABSENCE))
            result.emplace_back(Absence, act, "");
    }
    Future.clear();
    return result;
}