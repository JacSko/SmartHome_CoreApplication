/* =============================
 *   Includes of common headers
 * =============================*/

/* =============================
 *  Includes of project headers
 * =============================*/
#include "stairs_led_module.h"
#include "task_scheduler.h"
/* =============================
 *          Defines
 * =============================*/

/* =============================
 *   Internal module functions
 * =============================*/
void slm_prepare_default_programs();
/* =============================
 *       Internal types
 * =============================*/
typedef struct LED_MODULE
{

} LED_MODULE;
/* =============================
 *      Module variables
 * =============================*/
SLM_PROGRAM LED_PROGRAMS[SLM_MAX_PROGRAMS];


RET_CODE slm_initialize(const SLM_CONFIG* config)
{

}
void slm_deinitialize()
{

}
void slm_prepare_default_programs()
{

}
RET_CODE slm_get_config(SLM_CONFIG* buffer)
{

}
RET_CODE slm_start_program_alw_on(SLM_PROGRAM_ID id)
{

}
RET_CODE slm_start_program(SLM_PROGRAM_ID id)
{

}
RET_CODE slm_stop_program()
{

}
SLM_STATE slm_get_state()
{

}
SLM_PROGRAM_ID slm_get_current_program_id()
{

}
RET_CODE slm_set_program_id(SLM_PROGRAM_ID id)
{

}
RET_CODE slm_get_program_by_id(SLM_PROGRAM_ID id, SLM_PROGRAM* buffer)
{

}
RET_CODE slm_replace_program(SLM_PROGRAM_ID id, const SLM_PROGRAM* program)
{

}


