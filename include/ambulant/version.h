
#ifndef AMBULANT_H
#define AMBULANT_H

#include <string.h>

namespace ambulant {

#define AMBULANT_VERSION "0.2"

const char *get_version(void);

inline bool check_version()
{
	return strcmp(get_version(), AMBULANT_VERSION) == 0;
}

} // namespace ambulant

#endif
