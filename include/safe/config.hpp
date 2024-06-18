#pragma once

#ifndef SAFE_ENABLED
#define SAFE_ENABLED  !NDEBUG 
#endif

namespace safe
{
    struct checked;
    struct unchecked;
    struct checked_shared;

    namespace enabled
    {
        using mode = checked;  // Remove!!
        using strong = checked;
        using weak = checked_shared;
    }

    namespace disabled
    {
        using mode = unchecked;
        using strong = unchecked;
        using weak = unchecked;
    }

#if SAFE_ENABLED
    using namespace enabled;
#else
    using namespace disabled;
#endif
}

