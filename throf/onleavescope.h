#pragma once

class OnLeaveScopeLambda
{
    std::function<void()> lambdaBody;

public:
    OnLeaveScopeLambda(std::function<void()> lambdaBody) : lambdaBody(lambdaBody)
    {
    }

    ~OnLeaveScopeLambda()
    {
        lambdaBody();
    }
};

// boiler-plate
#define CONCATENATE_DETAIL(x, y) x##y
#define CONCATENATE(x, y) CONCATENATE_DETAIL(x, y)
#define __ON_LEAVE_SCOPE_ID(index) CONCATENATE(ONLEAVESCOPELAMBDA, index)

#define ON_LEAVE_SCOPE(expr) OnLeaveScopeLambda __ON_LEAVE_SCOPE_ID(__LINE__) ( [&] () { expr; } );
