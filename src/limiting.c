/* Return time delta in ms */
static long tvdiff(struct timeval newer, struct timeval older)
{
  return (newer.tv_sec - older.tv_sec) * 1000 + (newer.tv_usec - older.tv_usec)/1000;
}

static void go_sleep(long ms)
{
  struct timeval timeout;

  timeout.tv_sec = ms/1000;
  timeout.tv_usec = ms % 1000 * 1000;

  select(0, NULL, NULL, NULL, &timeout);
}

/* Return the current time in a timeval struct */
static struct timeval tvnow(void)
{
  struct timeval now;
  (void)gettimeofday(&now, NULL);
  return now;
}

wtf {
  off_t sz = (off_t)(size *nmemb);
  if (recvpersecond) {
    struct timeval now;
    long timediff;
    long sleep_time;

    static off_t addit = 0;

    now = tvnow();
    timediff = tvdiff (now, lastrecvtime);

    if (recvpersecond > CURL_MAX_WRITE_SIZE && timediff < 100)
      addit += sz;
    else {
      sz += addit;
      addit = 0;

      if (sz * 1000 > recvpersecond * timediff) {
        sleep_time = (long)(sz * 1000/recvpersecond - timediff);

        if (sleep_time > 0) {
          go_sleep(sleep_time);
          now = tvnow();
        }
      }
      lastrecvtime = now;
    }
  }
}

static long decode_rate_limit(char *arg)
{
   char *m;
   long n;

   n = strtoll(arg, &m, 0);

   switch (strlen(m)) {
     case 0:
       /* We've got a whole string of digits, we're fine */
       return n;
     case 1:
       /* We may have a valid character after digits */
      switch (m[0]) {
        case 'b':
        case 'B':
          return n;
        case 'k':
        case 'K':
          return n * 1024;
        case 'm':
        case 'M':
          return n * 1024 * 1024;
        case 'g':
        case 'G':
          return n * 1024 * 1024 * 1024;
        default:
          return -1;
      }
     default:
       /* We've got garbage: fail */
       return -1;
   }
}

