//
// Created by giacomo on 26/08/23.
//

#ifndef REDUCER_DECLARE_TO_POWERDECL_H
#define REDUCER_DECLARE_TO_POWERDECL_H

#include <yaucl/structures/any_to_uint_bimap.h>
#include <ostream>
#include <declare_cases.h>

void stream_powerdecl(std::ostream& os,
                       const std::vector<DatalessCases>& model,
                       yaucl::structures::any_to_uint_bimap<std::string>& act_map);
#endif //REDUCER_DECLARE_TO_POWERDECL_H
