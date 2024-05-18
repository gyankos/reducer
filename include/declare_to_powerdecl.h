/*
 * declare_to_powerdecl.h
 * This file is part of DECLAREd
 *
 * Copyright (C) 2023 - Giacomo Bergami
 *
 * DECLAREd is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * DECLAREd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with DECLAREd. If not, see <http://www.gnu.org/licenses/>.
 */
//
// Created by Anonymous Ⅳ on 26/08/23.
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
