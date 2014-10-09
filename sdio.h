#ifndef __SDIO_H__
#define __SDIO_H__

#ifdef __cplusplus
extern "C" {
#endif


bool_t sdioConnect (void);
bool_t sdioDisconnect (void);
bool_t isCardInserted  (void);
void cmd_sdiotest(BaseSequentialStream *chp, int argc,const char * const argv[]);

#ifdef __cplusplus
}
#endif


#endif // __SDIO_H__
