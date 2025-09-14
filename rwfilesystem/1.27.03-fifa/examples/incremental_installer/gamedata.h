#if !defined EXAMPLE_GAMEDATA_H
#define EXAMPLE_GAMEDATA_H 1

extern uint32_t frame_index;

void setRenderTarget(const uint32_t Index);
void flip(void);
void initShader(void);
int32_t initDisplay(void);
void setDrawEnv(void);
void setRenderState(void);
int32_t setRenderObject(void);

int pad_read( void );

#endif
