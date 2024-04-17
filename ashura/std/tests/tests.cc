
#include "ashura/std/log.h"
#include "ashura/std/pcg.h"

namespace ash
{
ash::Logger  default_logger_struct;
ash::Logger *default_logger = &default_logger_struct;
}        // namespace ash