#include "Body.h"
#include "PhysicsWorld.h"


namespace TIPHYS {

    Box* PhysicsBodyHandle::operator->() {
        assert(world);
        return world->GetBody(id);
    }

    Box* PhysicsBodyHandle::Get()
    {
        if (!world)
            return nullptr;
        return world->GetBody(id);
    }
}