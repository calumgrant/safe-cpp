#pragma once

#ifndef SAFE_ENABLED
#define SAFE_ENABLED  !NDEBUG 
#endif

namespace safe
{
    struct checked;
    struct unchecked;
    struct checked_weak;

    namespace enabled
    {
        using mode = checked;  // Remove!!
        using strong = checked;
        using weak = checked_weak;
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

