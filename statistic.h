#pragma once


typedef struct {
	long  cnt;
	float store;        // store to minimise computation
	float sum;
	float min;
	float max;
	float ssqdif;	    // sum of squares difference
} Statistic ;


typedef struct {
  Statistic stat3[3];
} Statistic3;

typedef struct {
  float v[3];
} Vec3f;

typedef struct {
  long l[3];
} Vec3l;


void stat_clear(Statistic *s);
void stat_add(Statistic *s, const float v);
long stat_count(const Statistic *s);
float stat_sum(const Statistic *s);
float stat_average(const Statistic *s);
float stat_minimum(const Statistic *s);
float stat_maximum(const Statistic *s);

float stat_pop_stdev(const Statistic *s);	    // population stdev
float stat_unbiased_stdev(const Statistic *s);

void  stat3_clear(Statistic3 *s3);
void  stat3_add(Statistic3 *s3, const Vec3f* v3f);
long  stat3_count(const Statistic3 *s3);
Vec3f stat3_sum(const Statistic3 *s3);
Vec3f stat3_average(const Statistic3 *s3);
Vec3f stat3_minimum(const Statistic3 *s3);
Vec3f stat3_maximum(const Statistic3 *s3);

Vec3f stat3_pop_stdev(const Statistic3 *s3);	    // population stdev
Vec3f stat3_unbiased_stdev(const Statistic3 *s3);

