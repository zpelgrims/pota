#include <ai.h>

AtCritSec l_critsec;
bool l_critsec_active;
inline bool lentil_crit_sec_init();
inline void lentil_crit_sec_close();
inline void lentil_crit_sec_enter();
inline void lentil_crit_sec_leave();