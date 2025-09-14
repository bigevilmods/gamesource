/*   SCE CONFIDENTIAL                                       */
/*   PLAYSTATION(R)3 Programmer Tool Runtime Library 270.001 */
/*   Copyright (C) 2008 Sony Computer Entertainment Inc.    */
/*   All Rights Reserved.                                   */
/*   File: main.c
 *   Description:
 *     simple sample to show how to use game data system utility
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timer.h>
#include <sys/return_code.h>
#include <cell/gcm.h>
#include <cell/pad.h>
#include <cell/sysmodule.h>
#include <cell/cell_fs.h>
#include <stddef.h>
#include <math.h>
#include <sys/process.h>
#include <sys/ppu_thread.h>
#include <sys/paths.h>
#include <sys/memory.h>

#include <sysutil/sysutil_sysparam.h>

#include "gamedata.h"

// --- debug --------------------------------------------------
//#define PRINTF printf
#define PRINTF(x...) 

#define ERR_PRINTF printf

//#define TRACE printf( "TRACE : %s:%s - %d\n", __FILE__, __FUNCTION__, __LINE__ )
#define TRACE

/* double buffering */
#define COLOR_BUFFER_NUM 2

typedef struct
{
	float x, y, z;
	uint32_t rgba; 
} Vertex_t;

// button[2]
#define	CTRL_SELECT		(1<<0)
#define	CTRL_L3			(1<<1)
#define	CTRL_R3			(1<<2)
#define	CTRL_START		(1<<3)
#define	CTRL_UP			(1<<4)
#define	CTRL_RIGHT		(1<<5)
#define	CTRL_DOWN		(1<<6)
#define	CTRL_LEFT		(1<<7)
// button[3]
#define	CTRL_L2			(1<<8)
#define	CTRL_R2			(1<<9)
#define	CTRL_L1			(1<<10)
#define	CTRL_R1			(1<<11)
#define	CTRL_TRIANGLE	(1<<12)
#define	CTRL_CIRCLE		(1<<13)
#define	CTRL_CROSS		(1<<14)
#define	CTRL_SQUARE		(1<<15)

/* local memory allocation */
static uint32_t local_mem_heap = 0;
static void *localMemoryAlloc(const uint32_t size) 
{
	uint32_t allocated_size = (size + 1023) & (~1023);
	uint32_t base = local_mem_heap;
	local_mem_heap += allocated_size;
	return (void*)base;
}

static void *localMemoryAlign(const uint32_t alignment, 
		const uint32_t size)
{
	local_mem_heap = (local_mem_heap + alignment-1) & (~(alignment-1));
	return (void*)localMemoryAlloc(size);
}

#define HOST_SIZE (1*1024*1024)

uint32_t display_width;
uint32_t display_height; 

float display_aspect_ratio;
uint32_t color_pitch;
uint32_t depth_pitch;
uint32_t color_offset[4];
uint32_t depth_offset;

extern uint32_t _binary_incremental_installer_vpshader_vpo_start;
extern uint32_t _binary_incremental_installer_vpshader_vpo_end;
extern uint32_t _binary_incremental_installer_fpshader_fpo_start;
extern uint32_t _binary_incremental_installer_fpshader_fpo_end;

static unsigned char *vertex_program_ptr = 
(unsigned char *)&_binary_incremental_installer_vpshader_vpo_start;
static unsigned char *fragment_program_ptr = 
(unsigned char *)&_binary_incremental_installer_fpshader_fpo_start;

static CGprogram vertex_program;
static CGprogram fragment_program;
static CGparameter model_view_projection;

static void *vertex_program_ucode;
static void *fragment_program_ucode;
static uint32_t fragment_offset;
static uint32_t vertex_offset[2];
static uint32_t color_index ;
static uint32_t position_index ;
static float MVP[16];

uint32_t frame_index = 0;
static int _setRenderObject_initFlag = 0;

void setRenderTarget(const uint32_t Index)
{
	CellGcmSurface sf;
	sf.colorFormat 	= CELL_GCM_SURFACE_A8R8G8B8;
	sf.colorTarget	= CELL_GCM_SURFACE_TARGET_0;
	sf.colorLocation[0]	= CELL_GCM_LOCATION_LOCAL;
	sf.colorOffset[0] 	= color_offset[Index];
	sf.colorPitch[0] 	= color_pitch;

	sf.colorLocation[1]	= CELL_GCM_LOCATION_LOCAL;
	sf.colorLocation[2]	= CELL_GCM_LOCATION_LOCAL;
	sf.colorLocation[3]	= CELL_GCM_LOCATION_LOCAL;
	sf.colorOffset[1] 	= 0;
	sf.colorOffset[2] 	= 0;
	sf.colorOffset[3] 	= 0;
	sf.colorPitch[1]	= 64;
	sf.colorPitch[2]	= 64;
	sf.colorPitch[3]	= 64;

	sf.depthFormat 	= CELL_GCM_SURFACE_Z24S8;
	sf.depthLocation	= CELL_GCM_LOCATION_LOCAL;
	sf.depthOffset	= depth_offset;
	sf.depthPitch 	= depth_pitch;

	sf.type		= CELL_GCM_SURFACE_PITCH;
	sf.antialias	= CELL_GCM_SURFACE_CENTER_1;

	sf.width 		= display_width;
	sf.height 		= display_height;
	sf.x 		= 0;
	sf.y 		= 0;
	cellGcmSetSurface(gCellGcmCurrentContext, &sf);
}

/* wait until flip */
static void waitFlip(void)
{
	while (cellGcmGetFlipStatus()!=0){
		sys_timer_usleep(300ULL);
	}
	cellGcmResetFlipStatus();
}


void flip(void)
{
	static int first=1;

	/* wait until the previous flip executed */
	if (first!=1) waitFlip();
	else cellGcmResetFlipStatus();

	if(cellGcmSetFlip(gCellGcmCurrentContext, frame_index) != CELL_OK) return;
	cellGcmFlush(gCellGcmCurrentContext);

	/* resend status */
	setDrawEnv();
	setRenderState();

	cellGcmSetWaitFlip(gCellGcmCurrentContext);

	/* New render target */
	frame_index = (frame_index+1)%COLOR_BUFFER_NUM;
	setRenderTarget(frame_index);

	first=0;
}


void initShader(void)
{
	vertex_program   = (CGprogram)vertex_program_ptr;
	fragment_program = (CGprogram)fragment_program_ptr;

	/* init */
	cellGcmCgInitProgram(vertex_program);
	cellGcmCgInitProgram(fragment_program);

	uint32_t ucode_size;
	void *ucode;
	cellGcmCgGetUCode(fragment_program, &ucode, &ucode_size);
	/* 64B alignment required */
	void *ret = localMemoryAlign(64, ucode_size);
	fragment_program_ucode = ret;
	memcpy(fragment_program_ucode, ucode, ucode_size); 

	cellGcmCgGetUCode(vertex_program, &ucode, &ucode_size);
	vertex_program_ucode = ucode;
}

static void buildProjection(float *M, 
		const float top, 
		const float bottom, 
		const float left, 
		const float right, 
		const float near, 
		const float far)
{
	memset(M, 0, 16*sizeof(float)); 

	M[0*4+0] = (2.0f*near) / (right - left);
	M[1*4+1] = (2.0f*near) / (bottom - top);

	float A = (right + left) / (right - left);
	float B = (top + bottom) / (top - bottom);
	float C = -(far + near) / (far - near);
	float D = -(2.0f*far*near) / (far - near);

	M[0*4 + 2] = A;
	M[1*4 + 2] = B;
	M[2*4 + 2] = C;
	M[3*4 + 2] = -1.0f; 
	M[2*4 + 3] = D;
}

static void matrixMul(float *Dest, float *A, float *B)
{
	for (int i=0; i < 4; i++) {
		for (int j=0; j < 4; j++) {
			Dest[i*4+j] 
				= A[i*4+0]*B[0*4+j] 
				+ A[i*4+1]*B[1*4+j] 
				+ A[i*4+2]*B[2*4+j] 
				+ A[i*4+3]*B[3*4+j];
		}
	}
}

static void matrixTranslate(float *M, 
		const float x, 
		const float y, 
		const float z)
{
	memset(M, 0, sizeof(float)*16);
	M[0*4+3] = x;
	M[1*4+3] = y;
	M[2*4+3] = z;

	M[0*4+0] = 1.0f;
	M[1*4+1] = 1.0f;
	M[2*4+2] = 1.0f;
	M[3*4+3] = 1.0f;
}

static void unitMatrix(float *M)
{
	M[0*4+0] = 1.0f;
	M[0*4+1] = 0.0f;
	M[0*4+2] = 0.0f;
	M[0*4+3] = 0.0f;

	M[1*4+0] = 0.0f;
	M[1*4+1] = 1.0f;
	M[1*4+2] = 0.0f;
	M[1*4+3] = 0.0f;

	M[2*4+0] = 0.0f;
	M[2*4+1] = 0.0f;
	M[2*4+2] = 1.0f;
	M[2*4+3] = 0.0f;

	M[3*4+0] = 0.0f;
	M[3*4+1] = 0.0f;
	M[3*4+2] = 0.0f;
	M[3*4+3] = 1.0f;
}

#define CB_SIZE	(0x10000)

int32_t initDisplay(void)
{
	uint32_t color_depth=4; /* ARGB8 */
	uint32_t z_depth=4;     /* COMPONENT24 */
	void *color_base_addr;
	void *depth_base_addr;
	void *color_addr[4];
	void *depth_addr;
	int32_t ret;
	CellVideoOutResolution resolution;

	/* display initialize */

	/* read the current video status */
	/* INITIAL DISPLAY MODE HAS TO BE SET BY RUNNING SETMONITOR.SELF */
	CellVideoOutState videoState;
	ret = cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &videoState);
	if (ret != CELL_OK){
		printf("cellVideoOutGetState() failed. (0x%x)\n", ret);
		return -1;
	}
	cellVideoOutGetResolution(videoState.displayMode.resolutionId, &resolution);

	display_width = resolution.width;
	display_height = resolution.height;
	color_pitch = display_width*color_depth;
	depth_pitch = display_width*z_depth;

	switch (videoState.displayMode.aspect){
	  case CELL_VIDEO_OUT_ASPECT_4_3:
		  display_aspect_ratio=4.0f/3.0f;
		  break;
	  case CELL_VIDEO_OUT_ASPECT_16_9:
		  display_aspect_ratio=16.0f/9.0f;
		  break;
	  default:
		  PRINTF("unknown aspect ratio %x\n", videoState.displayMode.aspect);
		  display_aspect_ratio=16.0f/9.0f;
	}

	uint32_t color_size =   color_pitch*display_height;
	uint32_t depth_size =  depth_pitch*display_height;

	CellVideoOutConfiguration videocfg;
	memset(&videocfg, 0, sizeof(CellVideoOutConfiguration));
	videocfg.resolutionId = videoState.displayMode.resolutionId;
	videocfg.format = CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_X8R8G8B8;
	videocfg.pitch = color_pitch;

	ret = cellVideoOutConfigure(CELL_VIDEO_OUT_PRIMARY, &videocfg, NULL, 0);
	if (ret != CELL_OK){
		printf("cellVideoOutConfigure() failed. (0x%x)\n", ret);
		return -1;
	}

	cellGcmSetFlipMode(CELL_GCM_DISPLAY_VSYNC);

	/* get config */
	CellGcmConfig config;
	cellGcmGetConfiguration(&config);
	/* buffer memory allocation */
	local_mem_heap = (uint32_t)config.localAddress;

	color_base_addr = localMemoryAlign(16, COLOR_BUFFER_NUM*color_size);

	for (int i = 0; i < COLOR_BUFFER_NUM; i++) {
		color_addr[i]
			= (void *)((uint32_t)color_base_addr+ (i*color_size));
		ret = cellGcmAddressToOffset(color_addr[i], &color_offset[i]);
		if(ret != CELL_OK) return -1;
	}

	/* regist surface */
	for (int i = 0; i < COLOR_BUFFER_NUM; i++) {
		ret = cellGcmSetDisplayBuffer(i, color_offset[i], color_pitch, display_width, display_height);
		if(ret != CELL_OK) return -1;
	}

	depth_base_addr = localMemoryAlign(16, depth_size);
	depth_addr = depth_base_addr;
	ret = cellGcmAddressToOffset(depth_addr, &depth_offset);
	if(ret != CELL_OK) return -1;

	return 0;
}

#define MASK_R        0xff000000
#define MASK_G        0x00ff0000
#define MASK_B        0x0000ff00
#define INIT_VALUE_R  0x01000000
#define INIT_VALUE_G  0x00010000
#define INIT_VALUE_B  0x00000100
#define MIN_R         0xffff0000
#define MIN_G         0x00ffff00
#define MIN_B         0xff00ff00
enum {
	RGBA_B_UP = 0,
	RGBA_R_DOWN,
	RGBA_G_UP,
	RGBA_B_DOWN,
	RGBA_R_UP,
	RGBA_G_DOWN,
};

static int _rgba_flag = RGBA_B_UP;
static int _rgba_value = MASK_R;
static void setVertex(Vertex_t* vertex_buffer)
{
	switch (_rgba_flag) {
	case RGBA_B_UP:															/*J 赤→紫		青アップ */
		if ((_rgba_value & MASK_B) == MASK_B) _rgba_flag = RGBA_R_DOWN;		/*J 青アップ終了	赤ダウンフラグを立てる */
		else _rgba_value += INIT_VALUE_B*3;									/*J 青アップ中 */
	break;
	case RGBA_R_DOWN:														/*J 紫→青		赤ダウン */
		if ((~_rgba_value & MIN_R) == MIN_R) _rgba_flag = RGBA_G_UP;		/*J 赤ダウン終了	緑アップフラグを立てる */
		else _rgba_value -= INIT_VALUE_R*3;									/*J 赤ダウン中 */
	break;
	case RGBA_G_UP:															/*J 青→水色	緑アップ */
		if ((_rgba_value & MASK_G) == MASK_G) _rgba_flag = RGBA_B_DOWN;		/*J 緑アップ終了	青ダウンフラグを立てる */
		else _rgba_value += INIT_VALUE_G*3;									/*J 緑アップ中 */
	break;
	case RGBA_B_DOWN:														/*J 水色→緑	青ダウン */
		if ((~_rgba_value & MIN_B) == MIN_B) _rgba_flag = RGBA_R_UP;		/*J 青ダウン終了	赤アップフラグを立てる */
		else _rgba_value -= INIT_VALUE_B*3;									/*J 青ダウン中 */
	break;
	case RGBA_R_UP:															/*J 緑→黄色	赤アップ */
		if ((_rgba_value & MASK_R) == MASK_R) _rgba_flag = RGBA_G_DOWN;		/*J 赤アップ終了	緑ダウンフラグを立てる */
		else _rgba_value += INIT_VALUE_R*3;									/*J 赤アップ中 */
	break;
	case RGBA_G_DOWN:														/*J 黄色→赤	緑ダウン */
		if ((~_rgba_value & MIN_G) == MIN_G) _rgba_flag = RGBA_B_UP;		/*J 緑ダウン終了	赤アップフラグを立てる */
		else _rgba_value -= INIT_VALUE_G*3;									/*J 緑ダウン中 */
	break;
	}
	
	vertex_buffer[0].x = -1.0f; 
	vertex_buffer[0].y = -1.0f; 
	vertex_buffer[0].z = -1.0f; 
	vertex_buffer[0].rgba=_rgba_value;

	vertex_buffer[1].x =  1.0f; 
	vertex_buffer[1].y = -1.0f; 
	vertex_buffer[1].z = -1.0f; 
	vertex_buffer[1].rgba=_rgba_value;

	vertex_buffer[2].x = -1.0f; 
	vertex_buffer[2].y =  1.0f; 
	vertex_buffer[2].z = -1.0f; 
	vertex_buffer[2].rgba=_rgba_value;
}

void setDrawEnv(void)
{
	cellGcmSetColorMask(gCellGcmCurrentContext, CELL_GCM_COLOR_MASK_B|
			CELL_GCM_COLOR_MASK_G|
			CELL_GCM_COLOR_MASK_R|
			CELL_GCM_COLOR_MASK_A);

	cellGcmSetColorMaskMrt(gCellGcmCurrentContext, 0);
	uint16_t x,y,w,h;
	float min, max;
	float scale[4],offset[4];

	x = 0;
	y = 0;
	w = display_width;
	h = display_height;
	min = 0.0f;
	max = 1.0f;
	scale[0] = w * 0.5f;
	scale[1] = h * -0.5f;
	scale[2] = (max - min) * 0.5f;
	scale[3] = 0.0f;
	offset[0] = x + scale[0];
	offset[1] = y + h * 0.5f;
	offset[2] = (max + min) * 0.5f;
	offset[3] = 0.0f;

	cellGcmSetViewport(gCellGcmCurrentContext, x, y, w, h, min, max, scale, offset);
	cellGcmSetClearColor(gCellGcmCurrentContext, (64<<0)|(64<<8)|(64<<16)|(64<<24));

	cellGcmSetDepthTestEnable(gCellGcmCurrentContext, CELL_GCM_TRUE);
	cellGcmSetDepthFunc(gCellGcmCurrentContext, CELL_GCM_LESS);

}

void setRenderState(void)
{
	cellGcmSetVertexProgram(gCellGcmCurrentContext, vertex_program, vertex_program_ucode);
	cellGcmSetVertexProgramParameter(gCellGcmCurrentContext, model_view_projection, MVP);
	cellGcmSetVertexDataArray(gCellGcmCurrentContext, position_index,
			0, 
			sizeof(Vertex_t), 
			3, 
			CELL_GCM_VERTEX_F, 
			CELL_GCM_LOCATION_LOCAL, 
			(uint32_t)vertex_offset[0]);
	cellGcmSetVertexDataArray(gCellGcmCurrentContext, color_index,
			0, 
			sizeof(Vertex_t), 
			4, 
			CELL_GCM_VERTEX_UB, 
			CELL_GCM_LOCATION_LOCAL, 
			(uint32_t)vertex_offset[1]);



	cellGcmSetFragmentProgram(gCellGcmCurrentContext, fragment_program, fragment_offset);

}

int32_t setRenderObject(void)
{
	static Vertex_t *vertex_buffer;
	if (!_setRenderObject_initFlag) 
	{
		_setRenderObject_initFlag = 1;
		void *ret = localMemoryAlign(128*1024, 1024);
		vertex_buffer = (Vertex_t*)ret;
	}
	setVertex(vertex_buffer);

	/* transform */
	float M[16];
	float P[16];
	float V[16];
	float VP[16];

	/* projection */
	buildProjection(P, -1.0f, 1.0f, -1.0f, 1.0f, 1.0, 10000.0f); 

	/*E 16:9 scale or 4:3 scale */
	matrixTranslate(V, 0.0f, 0.0f, -4.0);
	V[0*4 + 0] = 1.0f / display_aspect_ratio;
	V[1*4 + 1] = 1.0f; 

	/* model view */
	matrixMul(VP, P, V);

	unitMatrix(M);
	matrixMul(MVP, VP, M);

	model_view_projection = cellGcmCgGetNamedParameter(vertex_program, "modelViewProj");
	CGparameter position 
		= cellGcmCgGetNamedParameter(vertex_program, "position");
	CGparameter color 
		= cellGcmCgGetNamedParameter(vertex_program, "color");
	/* get Vertex Attribute index */
	position_index = cellGcmCgGetParameterResource(vertex_program, position) - CG_ATTR0; 
	color_index = cellGcmCgGetParameterResource(vertex_program, color) - CG_ATTR0; 

	cellGcmSetVertexProgramParameter(gCellGcmCurrentContext, model_view_projection, MVP);

	cellGcmSetVertexProgram(gCellGcmCurrentContext, vertex_program, vertex_program_ucode);

	/* fragment program offset */
	if(cellGcmAddressToOffset(fragment_program_ucode, &fragment_offset) != CELL_OK)
		return -1;

	cellGcmSetFragmentProgram(gCellGcmCurrentContext, fragment_program, fragment_offset);

	if (cellGcmAddressToOffset(&vertex_buffer->x, &vertex_offset[0]) != CELL_OK)
		return -1;
	if (cellGcmAddressToOffset(&vertex_buffer->rgba, &vertex_offset[1]) != CELL_OK)
		return -1;
	cellGcmSetVertexDataArray(gCellGcmCurrentContext, position_index,
			0, 
			sizeof(Vertex_t), 
			3, 
			CELL_GCM_VERTEX_F, 
			CELL_GCM_LOCATION_LOCAL, 
			(uint32_t)vertex_offset[0]);
	cellGcmSetVertexDataArray(gCellGcmCurrentContext, color_index,
			0, 
			sizeof(Vertex_t), 
			4, 
			CELL_GCM_VERTEX_UB, 
			CELL_GCM_LOCATION_LOCAL, 
			(uint32_t)vertex_offset[1]);
	return 0;
}

int pad_read( void )
{
	int ret;
	uint32_t		padd;

	CellPadData databuf;
	CellPadInfo2 infobuf;

	ret = cellPadGetInfo2( &infobuf );
	if ( ret != CELL_PAD_OK ) {
		PRINTF( "ERROR : cellPadGetInfo2() = 0x%x\n", ret );
		return 1;
	}
	if ( ( infobuf.port_status[0] & CELL_PAD_STATUS_CONNECTED ) == 0 ) {
		return 1;
	}

	ret = cellPadGetData( 0, &databuf );
	if ( ret != 0 ) {
		printf( "cellPadGetData() = 0x%x\n", ret );
		return 1;
	}
	if ( databuf.len == 0 ) {
		return 1;
	}

	padd = ( databuf.button[2] | ( databuf.button[3] << 8 ) ); 

	if ( padd & CTRL_START ) {
		return 0;
	}

	return 1;
}
