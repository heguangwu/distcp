/*
 * ScopeGuard.h
 *
 *  Created on: 2014-10-23
 *      Author: hgw
 */

#ifndef SCOPEGUARD_H_
#define SCOPEGUARD_H_

#include <functional>

class ScopeGuard
{
public:
    explicit ScopeGuard(std::function< void() > onExitScope)
        : onExitScope_(onExitScope)
    { }

    ~ScopeGuard()
    {
        onExitScope_();
    }

private:
    std::function< void() > onExitScope_;

private: // noncopyable
    ScopeGuard(ScopeGuard const&);
    ScopeGuard& operator=(ScopeGuard const&);
};

#define SCOPEGUARD_LINENAME_CAT(name, line) name##line
#define SCOPEGUARD_LINENAME(name, line) SCOPEGUARD_LINENAME_CAT(name, line)

#define ON_SCOPE_EXIT(callback) ScopeGuard SCOPEGUARD_LINENAME(EXIT, __LINE__)(callback)

#endif /* SCOPEGUARD_H_ */
