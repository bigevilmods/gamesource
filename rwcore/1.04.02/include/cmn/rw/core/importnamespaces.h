// (c) Electronic Arts. All Rights Reserved.

// This header is deprecated as of version 1.01.00 of rwcore.

// This file contains namespace mappings from the 4.0.x branch of
// RenderWare to version 4.5.x.  Please update your code to not
// require this header any more.

#ifndef RW_NAMESPACEIMPORTS_HPP
#define RW_NAMESPACEIMPORTS_HPP

// Declare the structure of the new namespaces
namespace rw
{
    namespace core
    {
        namespace arena {}
    }
    namespace oldanimation
    {
        namespace conditioning {}
    }
    namespace collision
    {
        namespace conditioning {}
    }
    namespace conditioning
    {
        namespace shared {}
        namespace core {}
    }
    namespace graphics
    {
        namespace conditioning {}
    }
    namespace physics
    {
        namespace conditioning {}
    }
    namespace platform
    {
        namespace conditioning {}
        namespace design_view {}
        namespace draw {}
        namespace game_framework {}
        namespace profile_mesh {}
        namespace hierarchy {}
        namespace skin {}
        namespace sink {}
    }
}
namespace sk
{
}


// If the user has defined RW_NO_RW40_COMPATABILITY
// Then these namespace imports will not occur.

#ifndef RW_NO_RW40_COMPATIBILITY
//#ifdef RW_RW40_COMPATIBILITY

// Import them into the old namespaces
namespace rwg
{
    using namespace rw::graphics;
    using namespace rw::graphics::conditioning;
    using namespace rw::platform::design_view;
    using namespace rw::platform::profile_mesh;
}
namespace rwk
{
    using namespace rw::oldanimation;
    using namespace rw::oldanimation::conditioning;
    using namespace rw::platform::hierarchy;
    using namespace rw::platform::sink;
}
namespace rwc
{
    using namespace rw::collision;
    using namespace rw::collision::conditioning;
}
namespace rwp
{
    using namespace rw::physics;
    using namespace rw::physics::conditioning;
}
namespace rwx
{
    using namespace rw::conditioning::core;
    using namespace rw::conditioning::shared;
}
namespace rw
{
    using namespace rw::core::arena;
    using namespace rw::platform::conditioning;
    using namespace rw::platform::profile_mesh;
    using namespace rw::platform::game_framework;
    namespace skin = rw::platform::skin;
    namespace draw
    {
        using namespace rw::platform::draw;
    }
}
using namespace sk;
#endif


// during the 4.5.X releases an error was made so that many RenderWare symbols were pulled into the
// global namespace. For the purpose of migration this define can be used to simulate this behaviour.
// This should be treated as a temporary fix only.
#ifdef RW_RW45_GLOBAL_NAMESPACE_COMPATIBILITY
using namespace rw;
using namespace rw::core::arena;
using namespace rw::oldanimation;
using namespace rw::oldanimation::conditioning;
using namespace rw::collision;
using namespace rw::collision::conditioning;
using namespace rw::conditioning;
using namespace rw::conditioning::shared;
using namespace rw::conditioning::core;
using namespace rw::graphics;
using namespace rw::graphics::conditioning;
using namespace rw::physics;
using namespace rw::physics::conditioning;
using namespace rw::platform;
using namespace rw::platform::conditioning;
using namespace rw::platform::design_view;
using namespace rw::platform::draw;
using namespace rw::platform::game_framework;
using namespace rw::platform::profile_mesh;
using namespace rw::platform::hierarchy;
using namespace rw::platform::skin;
using namespace rw::platform::sink;
using namespace sk;
#endif

#endif // RW_NAMESPACEIMPORTS_HPP
/* RWPUBLICEND */
