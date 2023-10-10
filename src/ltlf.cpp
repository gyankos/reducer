/*
 * ltlf.cpp
 * This file is part of DECLAREd
 *
 * Copyright (C) 2023 - Anonymous Ⅳ
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
// Created by Anonymous Ⅳ on 20/08/23.
//

#include <ltlf.h>

const ltlf ltlf::ltlf_true{ltlf::TRUE};
const ltlf ltlf::ltlf_false{ltlf::FALSE};
const ltlf ltlf::ltlf_end = ltlf::globally_(ltlf_false);

std::ostream& ltlf::to_aaltaf(std::ostream& os) const {
    switch (t) {
        case ltlf::TRUE:
            return os << "1";
        case ltlf::FALSE:
            return os << "0";
        case ltlf::END:
            return os << "!(X(1))";
        case ltlf::UNTIL:
             os << ("(");
             children.at(0).to_aaltaf(os) << ") U (";
             return children.at(1).to_aaltaf(os) << ")";
        case ltlf::GLOBALLY:
            os << "G(";
            return children.at(0).to_aaltaf(os) << ")";
        case ltlf::FUTURE:
            os << "F(";
            return children.at(0).to_aaltaf(os) << ")";
        case ltlf::ATOM:
            return os << atom;
        case ltlf::NOT:
            os << "!" << "(";
            return children.at(0).to_aaltaf(os) << ")";
        case ltlf::AND:
            os << ("(");
            (children.at(0).to_aaltaf(os)) << ") && (";
            return children.at(1).to_aaltaf(os) << ")";
        case ltlf::OR:
            os << ("(");
            (children.at(0).to_aaltaf(os)) << ") || (";
            return children.at(1).to_aaltaf(os) << ")";
        case ltlf::NEXT:
            os << "X(";
            return children.at(0).to_aaltaf(os) << ")";
        case ltlf::WEAKNEXT:
            os << "N(";
            return children.at(0).to_aaltaf(os) << ")";
        case ltlf::RELEASE:
            os << ("(");
            (children.at(0).to_aaltaf(os)) << ") R (";
            return children.at(1).to_aaltaf(os) << ")";
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const ltlf &ltlf) {
    switch (ltlf.t) {
        case ltlf::TRUE:
            return os << "tt";
        case ltlf::FALSE:
            return os << "ff";
        case ltlf::END:
            return os << "!(X(true))";
        case ltlf::UNTIL:
            return os << ("(") << (ltlf.children.at(0)) << ") U (" << ltlf.children.at(1) << ")";
        case ltlf::GLOBALLY:
            return os << "G(" << ltlf.children.at(0) << ")";
        case ltlf::FUTURE:
            return os << "F(" << ltlf.children.at(0) << ")";
        case ltlf::ATOM:
            return os << ltlf.atom;
        case ltlf::NOT:
            return os << "!" << "(" << ltlf.children.at(0) << ")";
        case ltlf::AND:
            return os << ("(") << (ltlf.children.at(0)) << ") & (" << ltlf.children.at(1) << ")";
        case ltlf::OR:
            return os << ("(") << (ltlf.children.at(0)) << ") | (" << ltlf.children.at(1) << ")";
        case ltlf::NEXT:
            return os << "X(" << ltlf.children.at(0) << ")";
        case ltlf::WEAKNEXT:
            return os << "WX(" << ltlf.children.at(0) << ")";
        case ltlf::RELEASE:
            return os << ("(") << (ltlf.children.at(0)) << ") R (" << ltlf.children.at(1) << ")";
    }
    return os;
}

ltlf ltlf::to_nnf() const {
    ltlf toreturn;
    switch (t) {
        case UNTIL: {
            toreturn.t = UNTIL;
            toreturn.children.emplace_back(children.at(0).to_nnf());
            toreturn.children.emplace_back(children.at(1).to_nnf());
            return toreturn;
        }

        case GLOBALLY: {
            toreturn.t = RELEASE;
            toreturn.children.emplace_back(ltlf_false);
            toreturn.children.emplace_back(children.at(0).to_nnf());
            return toreturn;
        }

        case FUTURE: {
            toreturn.t = UNTIL;
            toreturn.children.emplace_back(ltlf_true);
            toreturn.children.emplace_back(children.at(0).to_nnf());
            return toreturn;
        }

        case NOT: {
            if (children.at(0).isAtomic())
                return *this;
            else {
                return children.at(0).negate().to_nnf();
            }
        }

        case AND: {
            toreturn.t = AND;
            toreturn.children.emplace_back(children.at(0).to_nnf());
            toreturn.children.emplace_back(children.at(1).to_nnf());
            return toreturn;
        }

        case OR:{
            toreturn.t = OR;
            toreturn.children.emplace_back(children.at(0).to_nnf());
            toreturn.children.emplace_back(children.at(1).to_nnf());
            return toreturn;
        }

        case NEXT:{
            toreturn.t = NEXT;
            toreturn.children.emplace_back(children.at(0).to_nnf());
            return toreturn;
        }


        case WEAKNEXT:{
            toreturn.t = WEAKNEXT;
            toreturn.children.emplace_back(children.at(0).to_nnf());
            return toreturn;
        }

        case END: {
            return ltlf_end.to_nnf();
        }

        default:
            return *this;
    }
}

ltlf ltlf::negate() const {
    ltlf toreturn;
    switch (t) {
        case UNTIL:{
            toreturn.t = RELEASE;
            toreturn.children.emplace_back(children.at(0).negate());
            toreturn.children.emplace_back(children.at(1).negate());
            return toreturn;
        }

        case FUTURE:
        case GLOBALLY:
            return to_nnf().negate();


        case ATOM: {
            toreturn.t = NOT;
            toreturn.children.emplace_back(*this);
            return toreturn;
        }

        case NOT:
            return children.at(0);

        case END:
            return ltlf_end.to_nnf().negate();

        case AND:{
            toreturn.t = OR;
            toreturn.children.emplace_back(children.at(0).negate());
            toreturn.children.emplace_back(children.at(1).negate());
            return toreturn;
        }

        case OR:{
            toreturn.t = AND;
            toreturn.children.emplace_back(children.at(0).negate());
            toreturn.children.emplace_back(children.at(1).negate());
            return toreturn;
        }

        case WEAKNEXT: {
            toreturn.t = NEXT;
            toreturn.children.emplace_back(children.at(0).negate());
            return toreturn;
        }

        case NEXT:{
            toreturn.t = WEAKNEXT;
            toreturn.children.emplace_back(children.at(0).negate());
            return toreturn;
        }

        case TRUE:
            return ltlf_false;

        case FALSE:
            return ltlf_true;

        case RELEASE:{
            toreturn.t = UNTIL;
            toreturn.children.emplace_back(children.at(0).negate());
            toreturn.children.emplace_back(children.at(1).negate());
            return toreturn;
        }
    }
}

bool ltlf::operator==(const ltlf &rhs) const {
    return t == rhs.t &&
           atom == rhs.atom &&
           children == rhs.children;
}

bool ltlf::operator!=(const ltlf &rhs) const {
    return !(rhs == *this);
}

bool ltlf::operator<(const ltlf &rhs) const {
    if (t < rhs.t)
        return true;
    if (rhs.t < t)
        return false;
    if (atom < rhs.atom)
        return true;
    if (rhs.atom < atom)
        return false;
    return children < rhs.children;
}

bool ltlf::operator>(const ltlf &rhs) const {
    return rhs < *this;
}

bool ltlf::operator<=(const ltlf &rhs) const {
    return !(rhs < *this);
}

bool ltlf::operator>=(const ltlf &rhs) const {
    return !(*this < rhs);
}

ltlf ltlf::not_(const ltlf &left) {
    ltlf result{NOT};
    result.children.emplace_back(left);
    return result;
}

ltlf ltlf::x(const ltlf &left) {
    ltlf result{NEXT};
    result.children.emplace_back(left);
    return result;
}

ltlf ltlf::wx(const ltlf &left) {
    ltlf result{WEAKNEXT};
    result.children.emplace_back(left);
    return result;
}

ltlf ltlf::future_(const ltlf &left) {
    ltlf result{FUTURE};
    result.children.emplace_back(left);
    return result;
}

ltlf ltlf::globally_(const ltlf &left) {
    ltlf result{GLOBALLY};
    result.children.emplace_back(left);
    return result;
}

ltlf ltlf::implies(const ltlf &left, const ltlf &right) {
    return or_(not_(left), and_(left, right));
}

ltlf ltlf::or_(const ltlf &left, const ltlf &right) {
    ltlf result{OR};
    result.children.emplace_back(left);
    result.children.emplace_back(right);
    return result;
}

ltlf ltlf::until_(const ltlf &left, const ltlf &right) {
    ltlf result{UNTIL};
    result.children.emplace_back(left);
    result.children.emplace_back(right);
    return result;
}

ltlf ltlf::release_(const ltlf &left, const ltlf &right) {
    ltlf result{RELEASE};
    result.children.emplace_back(left);
    result.children.emplace_back(right);
    return result;
}

ltlf ltlf::and_(const ltlf &left, const ltlf &right) {
    ltlf result{AND};
    result.children.emplace_back(left);
    result.children.emplace_back(right);
    return result;
}

ltlf ltlf::platomic(const ltlf &argument) {
    ltlf result{ATOM};
    std::stringstream ss;
    ss << argument;
    result.atom = ss.str();
    return result;
}

bool ltlf::truth(const std::unordered_map<ltlf::atom_t , bool> &map) const {
    switch (t) {
        case ATOM: {
            auto it = map.find(atom);
            return (it != map.end() && it->second);
        }
        case TRUE:
            return true;
        case FALSE:
            return false;
        case NOT:
            return !children.at(0).truth(map);
        case OR:
            return children.at(0).truth(map) || children.at(1).truth(map);
        case AND:
            return children.at(0).truth(map) && children.at(1).truth(map);

        case END:
        case UNTIL:
        case GLOBALLY:
        case FUTURE:
        case NEXT:
        case RELEASE:
        case WEAKNEXT:
            throw std::runtime_error("ERRROR: This formula is not a proposition!");
    }
}

bool ltlf::truth(const std::unordered_set<atom_t> &map) const {
    switch (t) {
        case ATOM: {
            return map.contains(atom);
        }
        case TRUE:
            return true;
        case FALSE:
            return false;
        case NOT:
            return !children.at(0).truth(map);
        case OR:
            return children.at(0).truth(map) || children.at(1).truth(map);
        case AND:
            return children.at(0).truth(map) && children.at(1).truth(map);

        case END:
        case UNTIL:
        case GLOBALLY:
        case FUTURE:
        case NEXT:
        case RELEASE:
        case WEAKNEXT:
            throw std::runtime_error("ERRROR: This formula is not a proposition!");
    }
}

bool ltlf::truth(const atom_t &map) const {
    switch (t) {
        case ATOM: {
            return atom == map;
        }
        case TRUE:
            return true;
        case FALSE:
            return false;
        case NOT:
            return !children.at(0).truth(map);
        case OR:
            return children.at(0).truth(map) || children.at(1).truth(map);
        case AND:
            return children.at(0).truth(map) && children.at(1).truth(map);

        case END:
        case UNTIL:
        case GLOBALLY:
        case FUTURE:
        case NEXT:
        case RELEASE:
        case WEAKNEXT:
            throw std::runtime_error("ERRROR: This formula is not a proposition!");
    }
}

ltlf ltlf::_delta(const std::unordered_map<ltlf::atom_t, bool> &map, bool epsilon) const {
    switch (t) {
        case TRUE:
            return epsilon ? ltlf_false : ltlf_true;
        case FALSE:
            return ltlf_false;
        case ATOM:
            if (epsilon)
                return ltlf_false;
            else {
                auto it = map.find(atom);
                return (it != map.end() && it->second) ? ltlf_true : ltlf_false;
            }
        case UNTIL: {
            if (epsilon)
                return ltlf_false;
            auto u= or_(children.at(1)._delta(map, epsilon),
                       and_(children.at(0)._delta(map,epsilon), x({*this, true})._delta(map,epsilon)));
//            std::cout << u << std::endl;
            return u;
        }

        case END:
        case GLOBALLY:
        case FUTURE:
            return to_nnf()._delta(map, epsilon);

        case NOT: {
            const auto& c = children.at(0);
            if (c.isAtomic() || (c.t == END))
                if (epsilon)
                    return ltlf_false;
                else
                    return c._delta(map, epsilon).t == FALSE ? ltlf_true : ltlf_false;
            else
                throw std::runtime_error("ERROR: the formula must be in NNF!");
        }

        case AND:
            return and_(children.at(0)._delta(map,epsilon),
                        children.at(1)._delta(map,epsilon));

        case OR:
            return or_(children.at(0)._delta(map,epsilon),
                       children.at(1)._delta(map,epsilon));

        case NEXT:
            if (epsilon)
                return ltlf_false;
            else
                return and_({children.at(0), true}, {not_(ltlf_end).to_nnf(),true});


        case RELEASE:{
            if (epsilon)
                return ltlf_true;
            auto r= and_(children.at(1)._delta(map, epsilon),
                        or_(children.at(0)._delta(map,epsilon), wx({*this, true})._delta(map,epsilon)));
//            std::cout << r << std::endl;
            return r;
        }

        case WEAKNEXT:
            if (epsilon)
                return ltlf_true;
            else
                return or_({children.at(0), true}, {ltlf_end.to_nnf(), true});
    }
}

ltlf ltlf::_delta(const ltlf::atom_t &map, bool epsilon) const {
    switch (t) {
        case TRUE:
            return epsilon ? ltlf_false : ltlf_true;
        case FALSE:
            return ltlf_false;
        case ATOM:
            if (epsilon)
                return ltlf_false;
            else {
                return atom == map ? ltlf_true : ltlf_false;
            }
            break;
        case UNTIL:
        {
            if (epsilon)
                return ltlf_false;
            auto u= or_(children.at(1)._delta(map, epsilon),
                        and_(children.at(0)._delta(map,epsilon), x({*this, true})._delta(map,epsilon)));
//            std::cout << u << std::endl;
            return u;
        }

        case END:
        case GLOBALLY:
        case FUTURE:
            return to_nnf()._delta(map, epsilon);

        case NOT: {
            const auto& c = children.at(0);
            if (c.isAtomic() || (c.t == END))
                if (epsilon)
                    return ltlf_false;
                else
                    return c._delta(map, epsilon).t == FALSE ? ltlf_true : ltlf_false;
            else
                throw std::runtime_error("ERROR: the formula must be in NNF!");
        }

        case AND:
            return and_(children.at(0)._delta(map,epsilon),
                        children.at(1)._delta(map,epsilon));

        case OR:
            return or_(children.at(0)._delta(map,epsilon),
                       children.at(1)._delta(map,epsilon));

        case NEXT:
            if (epsilon)
                return ltlf_false;
            else
                return and_({children.at(0), true}, {not_(ltlf_end).to_nnf(),true});


        case RELEASE:
        {
            if (epsilon)
                return ltlf_true;
            auto r= and_(children.at(1)._delta(map, epsilon),
                         or_(children.at(0)._delta(map,epsilon), wx({*this, true})._delta(map,epsilon)));
//            std::cout << r << std::endl;
            return r;
        }

        case WEAKNEXT:
            if (epsilon)
                return ltlf_true;
            else
                return or_({children.at(0), true}, {ltlf_end.to_nnf(), true});
    }
}

ltlf ltlf::delta(const ltlf::atom_t &map, bool epsilon) const {
    auto f = to_nnf();
    auto result = f._delta(map, epsilon);
    if (epsilon)
        return result.truth(map) ? ltlf_true : ltlf_false;
    else
        return result;
}

ltlf ltlf::delta(const std::unordered_map<ltlf::atom_t , bool> &map, bool epsilon) const {
    auto f = to_nnf();
    auto result = f._delta(map, epsilon);
    if (epsilon)
        return result.truth(map) ? ltlf_true : ltlf_false;
    else
        return result;
}

void ltlf::collect_atoms(std::unordered_set<atom_t> &S) const {
    if (t == ATOM)
        S.emplace(atom);
    else
        for (const auto& c : children) c.collect_atoms(S);
}

void ltlf::find_atomics(std::unordered_set<ltlf> &S) const {
    if (isAtomic())
        S.insert(*this);
    else
    switch (t) {
        case ATOM:
        case TRUE:
        case FALSE:
            S.insert(*this);
            break;
        case NOT:
            children.at(0).find_atomics(S);
            break;
        case AND:
        case OR:
            children.at(0).find_atomics(S);
            children.at(1).find_atomics(S);
            break;

        case UNTIL:
        case GLOBALLY:
        case FUTURE:
        case NEXT:
        case RELEASE:
        case WEAKNEXT:
        case END:
            throw std::runtime_error("ERROR: not a PL formula");
    }
}


void ltlf::collect_multi_arg(std::unordered_set<ltlf>& collect_args, cases t) const {
    if (t == this->t) {
        for (const auto& c : children) c.collect_multi_arg(collect_args, t);
    } else {
        collect_args.emplace(*this);
    }
}

ltlf ltlf::multicollect() const {
    switch (t) {
        case NOT:
            return not_(children.at(0).simplify());

        case AND: {
            std::unordered_set<ltlf> collect_and_args;
            collect_multi_arg(collect_and_args, t);
            return unpack_binary_operator(collect_and_args, ltlf::ltlf_true, true, false);
        }

        case OR: {
            std::unordered_set<ltlf> collect_and_args;
            collect_multi_arg(collect_and_args, t);
            return unpack_binary_operator(collect_and_args, ltlf::ltlf_false, false, false);
        }

        case TRUE:
        case FALSE:
        case ATOM:
            return *this;

        case RELEASE:
        case WEAKNEXT:
        case END:
        case NEXT:
        case UNTIL:
        case GLOBALLY:
        case FUTURE:
            throw std::runtime_error("ERROR: unexpected ltlf expression");
            break;
    }
}

ltlf ltlf::simplify() const {
    switch (t) {
        case NOT:
            if (children.at(0).t == OR) {
                return and_(children.at(0).negate(), children.at(1).negate()).simplify();
            } else if (children.at(1).t == AND) {
                return or_(children.at(0).negate(), children.at(1).negate()).simplify();
            } else {
                return not_(children.at(0).simplify());
            }
        case AND: {
            if ((children.at(0).t == FALSE) || (children.at(1).t == FALSE))
                return ltlf_false;
            else if ((children.at(0).t == TRUE) && (children.at(1).t == TRUE))
                return ltlf_true;
            else if ((children.at(0).t == TRUE))
                return children.at(1).simplify();
            else if ((children.at(1).t == TRUE))
                return children.at(0).simplify();
            else {
                auto tmp = and_(children.at(0).simplify(), children.at(1).simplify());
                if (tmp != *this)
                    return tmp.simplify();
                else
                    return tmp;
            }
        }

        case OR:
            if ((children.at(0).t == TRUE) || (children.at(1).t == TRUE))
                return ltlf_true;
            else if ((children.at(0).t == FALSE) && (children.at(1).t == FALSE))
                return ltlf_false;
            else if ((children.at(0).t == FALSE))
                return children.at(1).simplify();
            else if ((children.at(1).t == FALSE))
                return children.at(0).simplify();
            else {
                auto tmp =  or_(children.at(0).simplify(), children.at(1).simplify());
                if (tmp != *this)
                    return tmp.simplify();
                else
                    return tmp;
            }

        case TRUE:
        case FALSE:
        case ATOM:
            return *this;

        case RELEASE:
        case WEAKNEXT:
        case END:
        case NEXT:
        case UNTIL:
        case GLOBALLY:
        case FUTURE:
            throw std::runtime_error("ERROR: unexpected ltlf expression");
            break;
    }
}
