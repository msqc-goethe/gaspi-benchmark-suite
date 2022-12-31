#include "testsome.h"
#ifdef USE_GASPI
#include "success_or_die.h"
#endif
#include "assert.h"
#include "comm_util.h"

int test_or_die
  ( unsigned short rank
  , unsigned char segment_id
  , unsigned short notification_id
  , unsigned int expected
  )
{
  unsigned short id;
  int ret;

  if ( ( WAIT_NOTIFY(rank, notification_id, &id)
       ) == 0 //SUccess
     )
  {
    //printf("id %d, notification id %d rank: %d\n", id, notification_id, iProc);
    ASSERT(id == notification_id);

    unsigned int value;

    NOTIFY_RESET(id,&value);

    //ASSERT (value == expected);
    return 1;
  }
  else
  {
    ASSERT (ret != -1);

    return 0;
  }
}
