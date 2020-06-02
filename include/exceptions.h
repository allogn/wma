//
// Created by Alvis Logins on 31/01/2018.
//

#ifndef FCLA_EXCEPTIONS_H
#define FCLA_EXCEPTIONS_H

#include <exception>

class InfeasibleSourceAssignmentException : public std::exception {
    virtual const char* what() const noexcept
    {
        return "Problem is infeasible";
    }
} infeasible_solution;

class NoMoreEdgesToAdd : public std::exception {
    virtual const char* what() const noexcept
    {
        return "There must be at least one feasible path to an extra node";
    }
} no_more_edges_to_add_exception;

class NoMoreCapacitiesToIncrease: public std::exception {
    virtual const char* what() const noexcept
    {
        return "No more customers can have capacities increased";
    }
} no_more_capacities_to_increase;

#endif //FCLA_EXCEPTIONS_H
