//
// Created by giacomo on 26/08/23.
//

#ifndef REDUCER_DECLARE_TO_LYDIALTLF_H
#define REDUCER_DECLARE_TO_LYDIALTLF_H

#include <yaucl/structures/any_to_uint_bimap.h>
#include <ostream>
#include <declare_cases.h>

void stream_lydia_ltlf(std::ostream& os,
                       const std::vector<DatalessCases>& model,
                       yaucl::structures::any_to_uint_bimap<std::string>& act_map,
                       bool withAxiom = true);

void stream_aaltaf_ltlf(std::ostream& os,
                       const std::vector<DatalessCases>& model,
                       yaucl::structures::any_to_uint_bimap<std::string>& act_map,
                       bool withAxiom = true);

#endif //REDUCER_DECLARE_TO_LYDIALTLF_H
