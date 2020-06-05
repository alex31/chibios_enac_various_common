#ifndef __SDIO_H__
#define __SDIO_H__

#ifdef __cplusplus
extern "C" {
#endif


bool sdioConnect (void);
bool sdioIsConnected (void);
bool sdioDisconnect (void);
bool isCardInserted  (void);
bool sdioIsCardResponding(void);

void cmd_sdiotest(BaseSequentialStream *chp, int argc,const char * const argv[]);

#ifdef __cplusplus
}
#endif


#endif // __SDIO_H__
