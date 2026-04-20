#include "api_catalog.h"

static const ApiGroupInfo API_GROUP_INFOS[] = {
#define API_GROUP(id, label, subtitle) {API_GROUP_##id, label, subtitle},
#define API_ENTRY(group, name, summary)
#include "api_catalog.inc"
#undef API_ENTRY
#undef API_GROUP
};

static const ApiEntry API_ENTRIES[] = {
#define API_GROUP(id, label, subtitle)
#define API_ENTRY(group, name, summary) {API_GROUP_##group, name, summary},
#include "api_catalog.inc"
#undef API_ENTRY
#undef API_GROUP
};

const ApiGroupInfo *api_catalog_groups(size_t *count)
{
    if (count != NULL)
    {
        *count = sizeof(API_GROUP_INFOS) / sizeof(API_GROUP_INFOS[0]);
    }

    return API_GROUP_INFOS;
}

const ApiEntry *api_catalog_entries(size_t *count)
{
    if (count != NULL)
    {
        *count = sizeof(API_ENTRIES) / sizeof(API_ENTRIES[0]);
    }

    return API_ENTRIES;
}

size_t api_catalog_total_count(void)
{
    return sizeof(API_ENTRIES) / sizeof(API_ENTRIES[0]);
}

size_t api_catalog_count_for_group(ApiGroupId group_id)
{
    size_t count = 0U;
    size_t i;

    for (i = 0U; i < sizeof(API_ENTRIES) / sizeof(API_ENTRIES[0]); ++i)
    {
        if (API_ENTRIES[i].group_id == group_id)
        {
            count++;
        }
    }

    return count;
}
