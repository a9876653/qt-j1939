#include "state_machine.h"
#include "stdlib.h"

bool state_machine_append(rt_slist_t *root, uint32_t step, void *args, state_machine_fun func)
{
    bool ret = false;
    /** 动态申请一片内存 */
    state_machine_t *handle = (state_machine_t *)malloc(sizeof(state_machine_t));
    if (handle)
    {
        handle->step = step;
        handle->args = args;
        handle->func = func;
        rt_slist_append(root, &(handle->node));
    }

    return ret;
}

state_machine_t *switch_state_machine(rt_slist_t *root, uint32_t step)
{
    state_machine_t *sub = NULL;

    /** 遍历单链表 state_machine */
    rt_slist_type_for_each_entry(state_machine_t, sub, root, node)
    {
        /** 匹配状态 */
        if (step == sub->step)
        {
            if (sub->func)
            {
                sub->func(sub, STATE_MACHINE_EVENT_ENTRY);
            }
            return sub;
        }
    }
    return NULL;
}

void state_machine_run(state_machine_t *handle)
{
    if (handle)
    {
        if (handle->func)
        {
            handle->func(handle, STATE_MACHINE_EVENT_RUN);
        }
    }
}
