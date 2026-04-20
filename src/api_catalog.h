#ifndef API_CATALOG_H
#define API_CATALOG_H

#include <stddef.h>

typedef enum
{
#define API_GROUP(id, label, subtitle) API_GROUP_##id,
#define API_ENTRY(group, name, summary)
#include "api_catalog.inc"
#undef API_ENTRY
#undef API_GROUP
    API_GROUP_COUNT
} ApiGroupId;

typedef struct
{
    ApiGroupId id;
    const char *name;
    const char *subtitle;
} ApiGroupInfo;

typedef struct
{
    ApiGroupId group_id;
    const char *name;
    const char *summary;
} ApiEntry;

const ApiGroupInfo *api_catalog_groups(size_t *count);
const ApiEntry *api_catalog_entries(size_t *count);
size_t api_catalog_total_count(void);
size_t api_catalog_count_for_group(ApiGroupId group_id);

#endif
