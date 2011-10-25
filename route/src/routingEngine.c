#include "routingEngine.h"


void initRouter(routingEngine *router,
                routingTable *tRouting,
                resourceTable *tResource,
                int nodeID,
                int cycleTime,
                int neighborTimeout,
                int retranTimeout,
                int LSATimeout)
{
    router->tRou = tRouting;
    router->tRes = tResource;
    router->nodeID = nodeID;
    router->cycleTime = cycleTime;
    router->neighborTimeout = neighborTimeout;
    router->retranTimeout = retranTimeout;
    router->LSATimeout = LSATimeout;

}

int startRouter(routingEngine *router)
{
    router = router;
    return EXIT_SUCCESS;
}
