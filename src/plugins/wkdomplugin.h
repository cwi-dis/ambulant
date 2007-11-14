#ifndef AMBULANT_PLUGIN_WKDOMPLUGIN_H
#define AMBULANT_PLUGIN_WKDOMPLUGIN_H
#include "ambulant/lib/node.h"

using namespace ambulant;

lib::node_factory *create_wkdom_node_factory(void *webframe);
#endif // AMBULANT_PLUGIN_WKDOMPLUGIN_H