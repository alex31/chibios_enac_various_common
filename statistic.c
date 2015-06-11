#include <math.h>
#include "statistic.h"
#include "stdutil.h"

// resets all counters
void stat_clear(Statistic *s)
{ 
	s->cnt = 0;     // count at N stored, becoming N+1 at a new iteration
	s->sum = 0.0f;
	s->min = 0.0f;
	s->max = 0.0f;
	s->ssqdif = 0.0f;  // not _ssq but sum of square differences
	                  // which is SUM(from i = 1 to N) of 
                          // (f(i)-_ave_N)**2
}

// adds a new value to the data-set
void stat_add(Statistic *s, const float f)
{
	if (s->cnt < 1)
	{
		s->min = f;
		s->max = f;
	} else {
	  if (f < s->min) s->min = f;
	  if (f > s->max) s->max = f;           
        } // end of if (s->cnt == 0) else
        s->sum += f;
	s->cnt++;

        if (s->cnt >1) {
           s->store = (s->sum / ((float) s->cnt) - f);
           s->ssqdif = s->ssqdif + ((float) s->cnt) * s->store * s->store / (((float) s->cnt)-1);
        } // end if > 1


}

// returns the number of values added
long stat_count(const Statistic *s)
{
	return s->cnt;
}

// returns the average of the data-set added sofar
float stat_average(const Statistic *s)
{
	if (s->cnt < 1) return NAN; // original code returned 0
	return s->sum / ((float) s->cnt);
}

// returns the sum of the data-set (0 if no values added)
float stat_sum(const Statistic *s)
{
	return s->sum;
}

// returns the sum of the data-set (0 if no values added)
float stat_minimum(const Statistic *s)
{
	return s->min;
}

// returns the sum of the data-set (0 if no values added)
float stat_maximum(const Statistic *s)
{
	return s->max;
}

// Population standard deviation = s = sqrt [ S ( Xi - µ )2 / N ]
// http://www.suite101.com/content/how-is-standard-deviation-used-a99084

float stat_pop_stdev(const Statistic *s)
{
	if (s->cnt < 1) return NAN; // otherwise DIV0 error
	return sqrtf( s->ssqdif / ((float) s->cnt));
}

float stat_unbiased_stdev(const Statistic *s)
{
	if (s->cnt < 2) return NAN; // otherwise DIV0 error
	return sqrtf( s->ssqdif / (((float) s->cnt) - 1));
}


#define FOREACH_STAT3(command)  for (uint32_t i=0; i<3; i++) {	\
  command ; \
  }

void  stat3_clear(Statistic3 *s3)
{
  FOREACH_STAT3( stat_clear (&s3->stat3[i]) );
}

void  stat3_add(Statistic3 *s3, const Vec3f* v3) 
{
  FOREACH_STAT3( stat_add (&s3->stat3[i], v3->v[i]) );
}

long stat3_count(const Statistic3 *s3)
{
  return s3->stat3[0].cnt;
}

Vec3f stat3_sum(const Statistic3 *s3)
{
  Vec3f ret;
  FOREACH_STAT3( ret.v[i] = stat_sum (&s3->stat3[i]) );
  return ret;
}

Vec3f stat3_average(const Statistic3 *s3)
{
  Vec3f ret;
  FOREACH_STAT3( ret.v[i] = stat_average (&s3->stat3[i]) );
  return ret;
}

Vec3f stat3_minimum(const Statistic3 *s3)
{
  Vec3f ret;
  FOREACH_STAT3( ret.v[i] = stat_minimum (&s3->stat3[i]) );
  return ret;
}

Vec3f stat3_maximum(const Statistic3 *s3)
{
  Vec3f ret;
  FOREACH_STAT3( ret.v[i] = stat_maximum (&s3->stat3[i]) );
  return ret;
}

Vec3f stat3_pop_stdev(const Statistic3 *s3)
{
  Vec3f ret;
  FOREACH_STAT3( ret.v[i] = stat_pop_stdev (&s3->stat3[i]) );
  return ret;
}	    // population stdev

Vec3f stat3_unbiased_stdev(const Statistic3 *s3)
{
  Vec3f ret;
  FOREACH_STAT3( ret.v[i] = stat_unbiased_stdev (&s3->stat3[i]) );
  return ret;
}
