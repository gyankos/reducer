//
// Created by giacomo on 22/08/23.
//

#define DEBUG
#include "model_reducer.h"
#include "yaucl/graphs/FlexibleFA.h"

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

bool model_reducer::reduce_map_to_be_considered(map_inout<std::string, std::string>& map_to_be_considered, bool alsoPassedMap) {
    FlexibleFA<std::string, char> crg;
    std::unordered_map<std::string, size_t> node_map;
    for (const auto& [a, targets] : map_to_be_considered) {
        // If the activation shall never happen, the clause is never triggered for checking
        if (test_future_condition(Future, a, BINARY_ABSENCE)) continue;
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
                        if (!exclude_from_existance(label)) return false;
                        if (alsoPassedMap)
                            exclude_from_map(map_to_be_considered, label);
                        if (!exclude_from_all_maps(label)) return false;
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

static inline void result_fill(map_inout<std::string, std::string>& map_to_consider,
                               declare_cases caso,
                               std::vector<DatalessCases>& result) {
    for (const auto& [a, bSet]: map_to_consider) {
        for (const auto& b : bSet)
            result.emplace_back(caso, a, b);
    }
    map_to_consider.clear();
}


bool model_reducer::reduce_cr(map_inout<std::string, std::string>& MN,
                                                    const std::string& absentLabel) {
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> toRemove;
    toRemove.emplace(absentLabel);
    while (!toRemove.empty()) {
        auto it3 = toRemove.begin();
        auto top = *it3; // Absent Label
        toRemove.erase(it3);
        if (visited.emplace(top).second) {
            if (!exclude_from_existance(top)) return false;
            if (!exclude_from_all_maps(top)) return false;
            auto it = MN.find_in(top);
            if (it != MN.end_in())
                // Shifting the check to all the previous activations requiring the presence of a label in
                // the future that shall be, on the other hand, absent.
                toRemove.insert(it->second.begin(), it->second.end());
        }
    }
    return true;
}

bool model_reducer::reduce_forward_re(const std::string& presentLabel) {
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> toExtend;
    toExtend.emplace(presentLabel);
    while (!toExtend.empty()) {
        auto it3 = toExtend.begin();
        auto top = *it3; // Absent Label
        toExtend.erase(it3);
        if (visited.emplace(top).second) {
            // If the novel existance leads to an inconsistent model, terminate
            if (!allow_existance(top))
                return false;
            // Removing all the choices containing this activated existential, as this trivally satisfies
            // the choice
            {
                auto it = Mchoice.find_out(top);
                if (it != Mchoice.end_out()) {
                    auto cp = it->second;
                    for (const auto& str : cp) {
                        Mchoice.erase(str, top);
                        Mchoice.erase(top, str);
                    }
                }
            }
            // If there are clauses having targets such existence, then please remove those,
            // as those are completely pointless (as the existence already exists alone)
            {
                auto it = Mresp_existence.find_in(top);
                if (it != Mresp_existence.end_in()) {
                    auto cp = it->second;
                    for (const auto& str : cp)
                        Mresp_existence.erase(str, top);
                }
            }
            // If there are clauses having such existence as activation, then remove all such clauses
            // and reduce those into other existentials in the next run
            auto it = Mresp_existence.find_out(top);
            if (it != Mresp_existence.end_out()) {
                // Shifting the check to all the previous activations requiring the presence of a label in
                // the future that shall be, on the other hand, absent.
                toExtend.insert(it->second.begin(), it->second.end());
                // Now, removing all the clauses starting from top, after saving all of the consequences
                Mresp_existence.eraseFirst(top);
            }
        }
    }
    return true;
}

bool model_reducer::reduce_r(const std::string& absentLabel) {
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> toRemove;
    toRemove.emplace(absentLabel);
    std::pair<is_negated, std::string> is_true{false, ""};
    while (!toRemove.empty()) {
        auto it3 = toRemove.begin();
        is_true.second = *it3; // Absent Label
        toRemove.erase(it3);
        if (visited.emplace(is_true.second).second) {
            // Setting this as an Absence clause. Returns false if this leads to an inconsistency
            if (!exclude_from_existance(is_true.second)) return false;
            // Removing all the clauses having the absentLabel as an activation condition, as they might
            // never occur
            if (!exclude_from_all_maps(is_true.second)) return false;
            auto it = MFuture.find_in(is_true);
            if (it != MFuture.end_in())
                // Shifting the check to all the previous activations requiring the presence of a label in
                // the future that shall be, on the other hand, absent.
                toRemove.insert(it->second.begin(), it->second.end());
        }
    }
    return true;
}

std::vector<DatalessCases> model_reducer::run(const std::vector<DatalessCases>& model) {
    clear();
    std::vector<DatalessCases> result;

    /// ① Knowledge-Base expansion phase
    /// Preliminary inconsistency detection with absence/existence, expanding the compound clauses
    /// and filling up the vectors/maps
    for (const auto& clause : model) {
        switch (clause.casus) {
            case Existence: {
                if (!allow_existance(clause.left))
                    return result; // Inconsistent empty model
            } break;

            case Absence: {
                if (!exclude_from_existance(clause.left))
                    return result;
            } break;

            case Choice:
                if (clause.left < clause.right)
                    choice.emplace_back(clause.left, clause.right);
                else if (clause.left != clause.right)
                    choice.emplace_back(clause.right, clause.left);
                else {
                    if (!allow_existance(clause.left))
                        return result; // Inconsistent empty model
                }
                break;

            case ExclChoice:
                if (clause.left < clause.right){
                    choice.emplace_back(clause.left, clause.right);
                    not_coexistence.emplace_back(clause.left, clause.right);
                } else if (clause.left != clause.right) {
                    choice.emplace_back(clause.right, clause.left);
                    not_coexistence.emplace_back(clause.right, clause.left);
                } else {
                    // Exclusive choice for the same activity label is remarking that this should
                    // both exist and not exists, which is inconsistent
                    return result;
                }
                break;

            case RespExistence:
                if (clause.left != clause.right) {
                    resp_existence.emplace_back(clause.left, clause.right);
                }
                // Else, if both are the same, this is a clause which is always true. Ignoring the
                // Always true clause at the moment, and distinguishing this case from an inconsistent
                // Model as an inconsistent model is immediately returned without any further ado,
                // while reaching the end of the algorithm with nothing to do means that there was no
                // inconsistency, and therefore a bogus always true clause can be added instead
                break;

            case CoExistence:
                if (clause.left != clause.right) {
                    resp_existence.emplace_back(clause.left, clause.right);
                    resp_existence.emplace_back(clause.right, clause.left);
                }
                break;

            case Response:
                response.emplace_back(clause.left, clause.right);
                break;

            case Precedence:
                Mprecedence.add(clause.left, clause.right);
                break;

            case ChainResponse:
                chain_response.emplace_back(clause.left, clause.right);
                response.emplace_back(clause.left, clause.right);
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
                Mprecedence.add(clause.left, clause.right);
                break;

            case AltPrecedence:
                Mprecedence.add(clause.left, clause.right);
                Malt_precedence.add(clause.right, clause.left); // This only regards G(b → X(¬bW a))
                break;

            case AltSuccession:
                response.emplace_back(clause.left, clause.right);
                Mprecedence.add(clause.left, clause.right);
                Malt_precedence.add(clause.right, clause.left); // This only regards G(b → X(¬bW a))
                Malt_response.add(clause.left, clause.right);
                break;

            case AltResponse:
                response.emplace_back(clause.left, clause.right);
                Malt_response.add(clause.left, clause.right);
                break;

            case NegSuccession:
                neg_succession.emplace_back(clause.left, clause.right);
                break;

            case NegChainSuccession:
                Mneg_chainsuccession.add(clause.left, clause.right);
                break;

            case NotCoexistence:
                if (clause.left < clause.right)
                    not_coexistence.emplace_back(clause.left, clause.right);
                else if (clause.left != clause.right)
                    not_coexistence.emplace_back(clause.right, clause.left);
                else {
                    if (!exclude_from_existance(clause.left))
                        return result;
                }
                break;

            case TRUTH:
                break;
        }
    }

    /// ② Ensuring that each clause appears at most only once in the model
    remove_duplicates(not_coexistence);
    remove_duplicates(response);
    remove_duplicates(neg_succession);
    remove_duplicates(chain_response);
    remove_duplicates(chain_precedence);
    remove_duplicates(resp_existence);
    remove_duplicates(choice);
    remove_duplicates(not_coexistence);

    /// ③ Detecting ChainResponse(A,B): G(A -> X(b))
    for (const auto& clause : chain_response) {
        // a. Not considering the clause if we now that the activation condition shall never occur
        if (test_future_condition(Future, clause.first, BINARY_ABSENCE))
            continue;

        // b. If the target condition is remarked as absence, then this implies the absence of the
        // activation, too! (Similar to ④c)
        if (test_future_condition(Future, clause.second, BINARY_ABSENCE)) {
            if (reduce_cr(MNext, clause.first))
                continue;
            else
                return result;
        }

        // c. If this should occur when G(a -> X(a)) and G(a → X (¬aU a)), then this is clearly
        // dictating the impossibility of a to occur
        if (clause.first == clause.second) {
            if (Malt_response.erase(clause.first, clause.second)) {
                if (reduce_cr(MNext, clause.first))
                    continue;
                else
                    return result;
            }
        }

        // d. If there is also a ChainSuccession with the same activation and target condition,
        // this clearly postulates the absence of the activation.
        {
            if (Mneg_chainsuccession.erase(clause.first, clause.second)) {
                if (reduce_cr(MNext, clause.first))
                    continue;
                else
                    return result;
            }
        }

        // e. Attempting to add the clause
        auto& mnextA = MNext.add(clause.first, clause.second);

        // f. if the next should contain two elements, due to G(a & b -> False) then the activation
        // should be also negated
        if (mnextA.size() > 1) {
            if (reduce_cr(MNext, clause.first))
                continue;
            else
                return result;
        }
    }
    chain_response.clear();

    /// ④ Detecting Response(A,B): G(a -> F(b))
    for (const auto& clause : response) {
        // a. Not considering the clause if we now that the activation condition shall never occur
        if (test_future_condition(Future, clause.first, BINARY_ABSENCE))
            continue;

        // b. G(a->X(b)) is stronger than G(a -> F(b)), so we can avoid the latter
        // This holds for G(a & b -> False) for which G(a -> F(b)) = G(a -> X(F(b)))
        if (MNext.contains(clause.first, clause.second))
            continue;

        // d. If the target condition is remarked as absence, then this implies the absence of the
        // activation, too! This requires checking all the clauses for which G(c -> F(a))
        if (test_future_condition(Future, clause.second, BINARY_ABSENCE)) {
            if (reduce_r(clause.first))
                continue; // Avoiding adding the clause as per d.
            else
                return result;
        }

        // e. Adding the clause
        MFuture.add(clause.first, {false, clause.second});
    }

    /// ⑤ Discarding all the clauses being falsified from ChainResponse, that might have not been
    /// handled
    for (const auto& [k, char_]: Future) {
        if (char_ & BINARY_ABSENCE) {
            if (!reduce_cr(MNext, k))
                return result;
        }
    }
    response.clear();

    /// ⑥ Detecting NegSuccession: G(a -> G(!b))
    std::pair<is_negated, std::string> is_positive{false, ""};
    for (const auto& clause : neg_succession) {
        // a. Not considering the clause if we now that the activation condition shall never occur.
        //    Similarly, if we already know that the second shall never occur, then we can ignore the
        //    presence of an activation
        if (test_future_condition(Future, clause.first, BINARY_ABSENCE) ||
                test_future_condition(Future, clause.second, BINARY_ABSENCE))
            continue;

        // b. if G(a -> F(b)) is present, this boils down to G(a -> False) = G(!a)
        //    This now also contemplates G(a -> X(b)) as G(a & b -> False) entails that
        //    it never happens that an event is satisfied by more than one atom for activity label
        is_positive.second = clause.second;
        if (MFuture.contains(clause.first, is_positive)) {
            if (!exclude_from_existance(clause.first))
                return result;
            if (!exclude_from_all_maps(clause.first))
                return result;
            continue;
        }

        // d. Adding the NegSuccession only if no further simplification was provided
        auto mfutureA = MFuture.find_out(clause.first);
        MFuture.add(mfutureA, clause.first, {true, clause.second});
    }
    neg_succession.clear();

    /// ⑤bis Discarding all the clauses being falsified from ChainResponse, that might have not been
    /// handled
    for (const auto& [k, char_]: Future) {
        if (char_ & BINARY_ABSENCE) {
            if (!reduce_r(k))
                return result;
            if (!reduce_cr(MNext, k))
                return result;
        }
    }

    /// ⑦ Declare AltResponse(A,B) G(a -> X(!a U b)).
    /// As this entails G(a -> F(b)), this case is handled in ④
    // The map was previously required to fastly scanning the map. For this other set up, we need to
    // constantly update the map while iterating, which might be detrimental on the long run. Therefore,
    // we prefer to go back to the list of elements
    for (const auto& [a, bSet] : Malt_response)
        for (const auto& b : bSet)
            alt_response.emplace_back(a, b);
    Malt_response.clear();
    std::pair<is_negated, std::string> is_neg{true, ""};
    for (const auto& clause : alt_response) {
        // a. Not considering the clause if we now that the activation condition shall never occur
        if (test_future_condition(Future, clause.first, BINARY_ABSENCE))
            continue;

        // b. ChainResponse is stronger than AltResponse, so keeping the former
        if (MNext.contains(clause.first, clause.second)) {
            continue;
        }

        // c. If G(a -> G(!b)) or G(!b), then this entails the absence of a, with a cascade effect
        // towards the other clauses
        is_neg.second = clause.second;
        if (MFuture.contains(clause.first, is_neg) ||
            (test_future_condition(Future, clause.second, BINARY_ABSENCE))) {
            if (reduce_cr(Malt_response, clause.first))
                continue;
            else
                return result;
        }

        // Observe! Keeping the G(a -> F(b)), as this might trigger other clause rewritings with absences.
        // Therefore, removing the clause G(a -> F(b)) is handled only at the end of the algorithm
    }
    alt_response.clear();

    /// ⑤ter Discarding all the clauses being falsified from ChainResponse, that might have not been
    /// handled
    for (const auto& [k, char_]: Future) {
        if (char_ & BINARY_ABSENCE) {
            if (!reduce_r(k))
                return result;
            if (!reduce_cr(MNext, k))
                return result;
            if (!reduce_cr(Malt_response, k))
                return result;
        }
    }

    /// ⑧ Declare AltPrecedence(A,B): this deals only with the G(b → X(¬bW a)) side of things,
    /// as the precedence part is dealt within the precedence itself. This clause does not lead
    /// to additional rewritings, and is kept last
    for (auto it = Malt_precedence.begin_out(); it != Malt_precedence.end_out(); it++) {
        // a. Not considering the clause if we now that the activation condition shall never occur
        if (test_future_condition(Future, it->first, BINARY_ABSENCE))
            continue;

        for (auto it2 = it->second.begin(); it2 != it->second.end(); ) {
            // b. If G(b -> X(a)) exists, then the latter is stronger than the current, which is then removed
            if (MNext.contains(it->first, *it2)) {
                it2 = it->second.erase(it2);
                continue;
            } else {
                it2++;
            }
        }
    }
    // Observe: not calling a similar procedure to ⑤ and its variants, as the former clause handling
    // does not lead to additional elements to be discarded


    /// ⑨ Detecting possible loops leading in LTLf to absence of all the activation codnitions
    /// This is applied to Precedence, Response, and ChainResponse. In all of these cases,
    /// I am detecting always whether this reduction also leads to a model inconsistency, for which
    /// I immediately return the empty model instead of furhter processing
    // a. Precedence
    if (!reduce_map_to_be_considered(Mprecedence))
        return result;

    // b. Dealing with Response requires to remove the information for NegSuccession first,
    // while removing the not NegSuccession information from the map
    {
        map_inout<std::string, std::string> intermediate;
        for (auto it2 = MFuture.begin_out(); it2 != MFuture.end_out(); ) {
            const auto& k = it2->first;
            auto& vals = it2->second;
            for (auto it = vals.begin(); it != vals.end(); ) {
                if (it->first)
                    it++;
                else {
                    intermediate.add(k, it->second);
                    it = vals.erase(it);
                }
            }
            if (vals.empty())
                it2 = MFuture.erase_out(it2);
            else
                it2++;
        }
        // Reducing only the Responses from the map
        if (!reduce_map_to_be_considered(intermediate, true))
            return result;
        // Adding back only the information that was not removed from the map
        for (const auto& [k, vals] : intermediate) {
            for (const auto& b : vals) {
                is_positive.second = b;
                MFuture.add(k, is_positive);
            }
        }
    }

    // c. ChainResponse
    if (!reduce_map_to_be_considered(MNext))
        return result;

    /// ⑤quater
    for (const auto& [k, char_]: Future) {
        if (char_ & BINARY_ABSENCE) {
            if (!reduce_r(k))
                return result;
            if (!reduce_cr(MNext, k))
                return result;
            if (!reduce_cr(Malt_response, k))
                return result;
        }
    }

    /// ⑩ Dealing with RespExistence very last, after all the absences have been produced (so to avoid
    /// unnecessary recomputations)
    for (const auto& clause : resp_existence) {
        // a. If the activation shall never happen, the clause is never triggered for checking
        if (test_future_condition(Future, clause.first, BINARY_ABSENCE))
            continue;

        if (test_future_condition(Future, clause.first, BINARY_PRESENCE)) {
            if (test_future_condition(Future, clause.second, BINARY_ABSENCE)) {
                // b. If the clause is activated and the response shall never happen, then I have an
                // inconsistency...
                return result; // Inconsistent model, as we prescribe that b shall never happen
            } else {
                // ... Otherwise, enforce this element to appear and ignore the clause, and immediately
                // return if this leads to an inconsistent model, which should have been detected in the
                // previous branch if any, but this might trigger other inconsistencies that were not
                // detectable before
                if (!reduce_forward_re(clause.second))
                    return result;
                // Discarding adding the clause here
                continue;
            }
        }

        // c. If the second element is already present, then it makes little sense of adding a RespExistence,
        //    as we require that the target should be there already indepedently from the activation
        if (test_future_condition(Future, clause.second, BINARY_PRESENCE)) {
            continue;
        }

        // d. Otherwise, if it is not prescribed to be activated, then I shall declare the
        // clause to be []!a. Also, propagating this absence towards the other clauses being collected
        if (test_future_condition(Future, clause.second, BINARY_ABSENCE)) {
            if (reduce_cr(Mresp_existence, clause.second))
                continue;
            else
                return result;
        }
        Mresp_existence.add(clause.first, clause.second);
    }
    resp_existence.clear();
    for (const auto& [k, char_]: Future) {
        if (char_ & BINARY_ABSENCE) {
            if (!reduce_r(k))
                return result;
            if (!reduce_cr(MNext, k))
                return result;
            if (!reduce_cr(Malt_response, k))
                return result;
        }
    }

    /// ⑪ Detecting choices. This will only generate existences, and not absences
    for (const auto& clause : choice) {
        // a. if we know for sure that both events must eventually occur, then choice is weaker,
        // and can be discarded in favour of two existences
        if (test_future_condition(Future, clause.first, BINARY_PRESENCE) ||
            test_future_condition(Future, clause.second, BINARY_PRESENCE))
            continue;

        // b. If both elements are said to be absent, then I have an inconsistent model...
        bool leftAbsent = test_future_condition(Future, clause.first, BINARY_ABSENCE);
        bool rightAbsent = test_future_condition(Future, clause.second, BINARY_ABSENCE);
        if (leftAbsent && rightAbsent) {
            return result; // Inconsistent model, as both clauses coexist
        } else
            // ... otherwise, if just one of the two is absent, then this boils down to F(a)
            // While doing so, further reducing the choices
            if (leftAbsent) {
                if (!reduce_forward_re(clause.second)) {
                    return result; // Inconsistent empty model
                } else
                    continue; // not adding the clause!
        } else if (rightAbsent) {
                if (!reduce_forward_re(clause.first)) {
                    return result; // Inconsistent empty model
                } else
                    continue; // not adding the clause!
        }

        // c. In all of the remaining cases, then I am introducing the clause.
        Mchoice.add(clause.first, clause.second);
        Mchoice.add(clause.second, clause.first);
    }
    choice.clear();

    /// ⑫  Detecting inconsistent models through not_coexistence
    for (const auto& clause : not_coexistence) {
        // a. If both events are present, then the model is inconsistent
        if ((test_future_condition(Future, clause.first, BINARY_PRESENCE)) &&
            (test_future_condition(Future, clause.second, BINARY_PRESENCE))) {
            result.clear();
            return result; // Inconsistent model, as both clauses coexist.
        }

        // I can start directly populate the model, at this stage
        if (Mchoice.erase(clause.first, clause.second) ||
            Mchoice.erase(clause.second, clause.first)) {
            result.emplace_back(ExclChoice,
                                std::min(clause.first, clause.second),
                                std::max(clause.second, clause.first));
        } else
            result.emplace_back(NotCoexistence,
                                std::min(clause.first, clause.second),
                                std::max(clause.second, clause.first));
    }
    not_coexistence.clear();

    DEBUG_ASSERT(not_coexistence.empty());
    DEBUG_ASSERT(chain_response.empty());
    DEBUG_ASSERT(response.empty());
    DEBUG_ASSERT(resp_existence.empty());
    DEBUG_ASSERT(choice.empty());
    DEBUG_ASSERT(choice.empty());
    DEBUG_ASSERT(resp_existence.empty());

    /// ⑬ Now, adding all the results to the model
    // a. ChainPrecedence(B,A)
    for (const auto& clause : chain_precedence) {
        // In particular, if there is also a ChainResponse(A,B), then return a ChainSuccession instead!
        if (MNext.erase(clause.second, clause.first)) {
            result.emplace_back(ChainSuccession, clause.second, clause.first);
        } else
            result.emplace_back(ChainPrecedence, clause.first, clause.second);
    }
    chain_precedence.clear();
    DEBUG_ASSERT(chain_precedence.empty());

    // c. Dealing with the G(b → X(¬bW a)) part of AltPrecedence(A,B). Still, considering adding an
    // AltSuccession if a AltResponse is also present
    for (const auto& [b, aSet]: Malt_precedence) {
        // AltPrecedence holds if both the Precedence was reduced to an absence...
        if (test_future_condition(Future, b, BINARY_ABSENCE)) {
            continue;
        } else {
            //...Or if still the precedence holds. Still, we expect the precedence to be there o.O
            for (const auto& a : aSet) {
                if (test_future_condition(Future, a, BINARY_ABSENCE)) {
                    if(!test_future_condition(Future, b, BINARY_ABSENCE))
                        throw std::runtime_error("LOGICAL ERROR (3)!");
                    continue;
                } else {
                    auto it = Mprecedence.find_out(a);
                    if (it != Mprecedence.end_out()) {
                        if (it->second.contains(b)) {
                            if (Malt_response.erase(a,b))
                                result.emplace_back(AltSuccession, a, b);
                            else
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
    }
    Malt_precedence.clear();

    // d. Dealing with the remaining AltResponse(A,B)
    for (const auto& [a, bSet]: Malt_response) {
        for (const auto& b : bSet) {
            result.emplace_back(AltResponse, a, b);
        }
    }
    Malt_response.clear();

    // e. Response(A,B)
    for (const auto& [a, bSet]: MFuture) {
        if (test_future_condition(Future, a, BINARY_ABSENCE)) continue;
        for (const auto& [negated, b] : bSet) {
            if (negated) {
                result.emplace_back(NegSuccession, a, b);
            } else {
                // Returning G(A -> F(b)) iff. G(A -> X(b)) is not in the model
                if (!MNext.contains(a, b)) {
                    // If there is also a Precedence, then return a Succession
                    if (Mprecedence.erase(a,b)) {
                        result.emplace_back(Succession, a, b);
                    } else
                        // Otherwise, return a simple Response
                        result.emplace_back(Response, a, b);
                }
            }
        }
    }
    MFuture.clear();

    // f. Precedence
    result_fill(Mprecedence, Precedence, result);
    DEBUG_ASSERT(Mprecedence.empty());

    // g. ChainResponse(A,B), adding the remaining ones, which are not ChainSuccessions
    result_fill(MNext, ChainResponse, result);
    DEBUG_ASSERT(MNext.empty());

    // h. Existence(A) and Absence(A)
    for (const auto& [act, char_] : Future) {
        if ((char_ & BINARY_PRESENCE))
            result.emplace_back(Existence, act, "");
        else if ((char_ & BINARY_ABSENCE))
            result.emplace_back(Absence, act, "");
    }
    Future.clear();

    // i. NegChainSuccession
    result_fill(Mneg_chainsuccession, NegChainSuccession, result);
    DEBUG_ASSERT(Mneg_chainsuccession.empty());

    // j. RespExistence
    for (const auto& [a, bSet]: Mresp_existence) {
        for (const auto& b : bSet) {
            if (Mresp_existence.contains(b,a)) {
                if (a<b)
                    result.emplace_back(CoExistence, a, b);
            } else {
                result.emplace_back(RespExistence, a, b);
            }
        }
    }
    Mresp_existence.clear();
    DEBUG_ASSERT(Mresp_existence.empty());

    // k. Choice
    for (const auto& [a, apSet] : Mchoice) {
        for (const auto& ap : apSet) {
            if (a<ap)
                result.emplace_back(Choice, a, ap);
            else
                DEBUG_ASSERT(Mchoice.contains(ap,a)); // Otherwise, it should be added later on
        }
    }
    Mchoice.clear();

    /// ⑭ At this last stage, I can never have an inconsistent model. Therefore, if no clause was given,
    /// This is very likely to be an always-true model, which shall not be even considered for specification
    /// checking
    if (result.empty())
        result.emplace_back(TRUTH, "", "");
    return result;
}