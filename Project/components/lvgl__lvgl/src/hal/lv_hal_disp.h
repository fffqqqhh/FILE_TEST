/**
 * @file lv_hal_disp.h
 *
 * @description Display Driver HAL interface header file
 *
 */

#ifndef LV_HAL_DISP_H
#define LV_HAL_DISP_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>
#include <stdbool.h>
#include "lv_hal.h"
#include "../draw/lv_draw.h"
#include "../misc/lv_color.h"
#include "../misc/lv_area.h"
#include "../misc/lv_ll.h"
#include "../misc/lv_timer.h"
#include "../misc/lv_ll.h"

/*********************
 *      DEFINES
 *********************/
#ifndef LV_INV_BUF_SIZE
#define LV_INV_BUF_SIZE 32 /*Buffer size for invalid areas*/
#endif

#ifndef LV_ATTRIBUTE_FLUSH_READY
#define LV_ATTRIBUTE_FLUSH_READY
#endif

/**********************
 *      TYPEDEFS
 **********************/

struct _lv_obj_t;
struct _lv_disp_t;
struct _lv_disp_drv_t;
struct _lv_theme_t;

/**  
 * 用于保存显示缓冲区信息的结构体。  
 */  
typedef struct _lv_disp_draw_buf_t {  
    void * buf1; /**< 第一个显示缓冲区。*/  
    void * buf2; /**< 第二个显示缓冲区。*/  
  
    /* 内部使用，由库管理 */  
    void * buf_act;  
    uint32_t size; /* 像素数量 */  
    /* 1: 正在刷新。（它不能是一个位字段，因为如果从IRQ中清除，可能会发生读-修改-写问题）*/  
    volatile int flushing;  
    /* 1: 这是要刷新的最后一块数据。（它不能是一个位字段，因为如果从IRQ中清除，可能会发生读-修改-写问题）*/  
    volatile int flushing_last;  
    volatile uint32_t last_area         : 1; /* 1: 正在渲染最后一个区域 */  
    volatile uint32_t last_part         : 1; /* 1: 正在渲染当前区域的最后一部分 */  
} lv_disp_draw_buf_t;

typedef enum {
    LV_DISP_ROT_NONE = 0,
    LV_DISP_ROT_90,
    LV_DISP_ROT_180,
    LV_DISP_ROT_270
} lv_disp_rot_t;


/**
 * 显示驱动程序结构体，由HAL注册。
 * 只有其指针会保存在 `lv_disp_t` 中，因此它应被声明为
 * `static lv_disp_drv_t my_drv` 或动态分配。
 */
typedef struct _lv_disp_drv_t {

    lv_coord_t hor_res;         /**< 水平分辨率。*/
    lv_coord_t ver_res;         /**< 垂直分辨率。*/

    lv_coord_t
    physical_hor_res;     /**< 全部/物理显示的水平分辨率。设置为 -1 表示全屏模式。*/
    lv_coord_t
    physical_ver_res;     /**< 全部/物理显示的垂直分辨率。设置为 -1 表示全屏模式。*/
    lv_coord_t
    offset_x;             /**< 从全部/物理显示的水平偏移。设置为 0 表示全屏模式。*/
    lv_coord_t offset_y;             /**< 从全部/物理显示的垂直偏移。设置为 0 表示全屏模式。*/

    /** 指向使用 `lv_disp_draw_buf_init()` 初始化的缓冲区的指针。
     * LVGL 将使用这个缓冲区(们)来绘制屏幕内容*/
    lv_disp_draw_buf_t * draw_buf;

    uint32_t direct_mode : 1;        /**< 1: 使用屏幕大小的缓冲区并绘制到绝对坐标*/
    uint32_t full_refresh : 1;       /**< 1: 总是重新绘制整个屏幕*/
    uint32_t sw_rotate : 1;          /**< 1: 使用软件旋转（较慢）*/
    uint32_t antialiasing : 1;       /**< 1: 此显示器启用了反锯齿。*/
    uint32_t rotated : 2;            /**< 1: 将显示器旋转90度。 @warning 不会为您更新坐标！*/
    uint32_t screen_transp : 1;      /**处理如果屏幕没有实心（opa == LV_OPA_COVER）背景。
                                       * 只有在必要时才使用，因为它会更慢。*/

    uint32_t dpi : 10;              /** 显示器的 DPI (每英寸点数)。默认值为 `LV_DPI_DEF`。*/

    /** 必须: 用于将内部缓冲区（draw_buf）写入到显示器。完成后必须调用 'lv_disp_flush_ready()'*/
    void (*flush_cb)(struct _lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);

    /** 可选: 将无效区域扩展到与显示器驱动程序要求的匹配。
     * 例如 在单色显示器上将 `y` 四舍五入为 8、16 等。*/
    void (*rounder_cb)(struct _lv_disp_drv_t * disp_drv, lv_area_t * area);

    /** 可选: 根据显示器的特殊要求在缓冲区中设置像素
     * 可用于不受 LittelvGL 支持的颜色格式。例如 2 位 -> 4 灰度级
     * @note 比使用支持的颜色格式绘制要慢得多。*/
    void (*set_px_cb)(struct _lv_disp_drv_t * disp_drv, uint8_t * buf, lv_coord_t buf_w, lv_coord_t x, lv_coord_t y,
                      lv_color_t color, lv_opa_t opa);

    void (*clear_cb)(struct _lv_disp_drv_t * disp_drv, uint8_t * buf, uint32_t size);


    /** 可选: 每次刷新周期结束后调用以告知渲染和刷新时间 + 刷新的像素数*/
    void (*monitor_cb)(struct _lv_disp_drv_t * disp_drv, uint32_t time, uint32_t px);

    /** 可选: 当 lvgl 等待操作完成时定期调用。
     * 例如刷新或 GPU
     * 用户可以在这里执行非常简单的任务或让出任务*/
    void (*wait_cb)(struct _lv_disp_drv_t * disp_drv);

    /** 可选: 当 lvgl 需要清理影响渲染的 CPU 缓存时调用*/
    void (*clean_dcache_cb)(struct _lv_disp_drv_t * disp_drv);

    /** 可选: 当驱动程序参数更新时调用 */
    void (*drv_update_cb)(struct _lv_disp_drv_t * disp_drv);

    /** 可选: 当开始渲染时调用 */
    void (*render_start_cb)(struct _lv_disp_drv_t * disp_drv);

    /** 在 CHROMA_KEYED 图像中，此颜色将是透明的。
     * 默认为 `LV_COLOR_CHROMA_KEY`。 (lv_conf.h)*/
    lv_color_t color_chroma_key;

    lv_draw_ctx_t * draw_ctx;
    void (*draw_ctx_init)(struct _lv_disp_drv_t * disp_drv, lv_draw_ctx_t * draw_ctx);
    void (*draw_ctx_deinit)(struct _lv_disp_drv_t * disp_drv, lv_draw_ctx_t * draw_ctx);
    size_t draw_ctx_size;

#if LV_USE_USER_DATA
    void * user_data; /**< 自定义显示驱动程序用户数据*/
#endif

} lv_disp_drv_t;

/**
 * 显示结构体。
 * @note `lv_disp_drv_t` 应该是结构体的第一个成员。
 */
typedef struct _lv_disp_t {
    /**< 显示驱动程序*/
    struct _lv_disp_drv_t * driver;

    /**< 一个定时器，定期检查脏区域并刷新它们*/
    lv_timer_t * refr_timer;

    /**< 分配给屏幕的主题*/
    struct _lv_theme_t * theme;

    /** 显示器的屏幕*/
    struct _lv_obj_t ** screens;    /**< 屏幕对象的数组。*/
    struct _lv_obj_t * act_scr;     /**< 当前此显示器上的活动屏幕*/
    struct _lv_obj_t * prev_scr;    /**< 先前的屏幕。用于屏幕动画过程中*/
    struct _lv_obj_t * scr_to_load; /**< 准备在 lv_scr_load_anim 中加载的屏幕*/
    struct _lv_obj_t * top_layer;   /**< @see lv_disp_get_layer_top*/
    struct _lv_obj_t * sys_layer;   /**< @see lv_disp_get_layer_sys*/
    uint32_t screen_cnt;
    uint8_t draw_prev_over_act : 1; /**< 1: 在活动屏幕上绘制先前的屏幕*/
    uint8_t del_prev : 1;           /**< 1: 屏幕加载动画准备就绪时自动删除先前的屏幕*/
    uint8_t rendering_in_progress : 1; /**< 1: 当前屏幕渲染正在进行中*/

    lv_opa_t bg_opa;                /**< 背景颜色或壁纸的透明度*/
    lv_color_t bg_color;            /**< 当屏幕透明时的默认显示颜色*/
    const void * bg_img;            /**< 要显示为壁纸的图像源*/

    /** 无效的（标记为重绘）区域*/
    lv_area_t inv_areas[LV_INV_BUF_SIZE];
    uint8_t inv_area_joined[LV_INV_BUF_SIZE];
    uint16_t inv_p;
    int32_t inv_en_cnt;

    /** 双缓冲同步区域 */
    lv_ll_t sync_areas;

    /*杂项数据*/
    uint32_t last_activity_time;        /**< 此显示器上最后一次活动的时间*/
} lv_disp_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize a display driver with default values.
 * It is used to have known values in the fields and not junk in memory.
 * After it you can safely set only the fields you need.
 * @param driver pointer to driver variable to initialize
 */
void lv_disp_drv_init(lv_disp_drv_t * driver);

/**
 * Initialize a display buffer
 * @param draw_buf pointer `lv_disp_draw_buf_t` variable to initialize
 * @param buf1 A buffer to be used by LVGL to draw the image.
 *             Always has to specified and can't be NULL.
 *             Can be an array allocated by the user. E.g. `static lv_color_t disp_buf1[1024 * 10]`
 *             Or a memory address e.g. in external SRAM
 * @param buf2 Optionally specify a second buffer to make image rendering and image flushing
 *             (sending to the display) parallel.
 *             In the `disp_drv->flush` you should use DMA or similar hardware to send
 *             the image to the display in the background.
 *             It lets LVGL to render next frame into the other buffer while previous is being
 * sent. Set to `NULL` if unused.
 * @param size_in_px_cnt size of the `buf1` and `buf2` in pixel count.
 */
void lv_disp_draw_buf_init(lv_disp_draw_buf_t * draw_buf, void * buf1, void * buf2, uint32_t size_in_px_cnt);

/**
 * Register an initialized display driver.
 * Automatically set the first display as active.
 * @param driver pointer to an initialized 'lv_disp_drv_t' variable. Only its pointer is saved!
 * @return pointer to the new display or NULL on error
 */
lv_disp_t * lv_disp_drv_register(lv_disp_drv_t * driver);

/**
 * Update the driver in run time.
 * @param disp pointer to a display. (return value of `lv_disp_drv_register`)
 * @param new_drv pointer to the new driver
 */
void lv_disp_drv_update(lv_disp_t * disp, lv_disp_drv_t * new_drv);

/**
 * Remove a display
 * @param disp pointer to display
 */
void lv_disp_remove(lv_disp_t * disp);

/**
 * Set a default display. The new screens will be created on it by default.
 * @param disp pointer to a display
 */
void lv_disp_set_default(lv_disp_t * disp);

/**
 * Get the default display
 * @return pointer to the default display
 */
lv_disp_t * lv_disp_get_default(void);

/**
 * Get the horizontal resolution of a display
 * @param disp pointer to a display (NULL to use the default display)
 * @return the horizontal resolution of the display
 */
lv_coord_t lv_disp_get_hor_res(lv_disp_t * disp);

/**
 * Get the vertical resolution of a display
 * @param disp pointer to a display (NULL to use the default display)
 * @return the vertical resolution of the display
 */
lv_coord_t lv_disp_get_ver_res(lv_disp_t * disp);

/**
 * Get the full / physical horizontal resolution of a display
 * @param disp pointer to a display (NULL to use the default display)
 * @return the full / physical horizontal resolution of the display
 */
lv_coord_t lv_disp_get_physical_hor_res(lv_disp_t * disp);

/**
 * Get the full / physical vertical resolution of a display
 * @param disp pointer to a display (NULL to use the default display)
 * @return the full / physical vertical resolution of the display
 */
lv_coord_t lv_disp_get_physical_ver_res(lv_disp_t * disp);

/**
 * Get the horizontal offset from the full / physical display
 * @param disp pointer to a display (NULL to use the default display)
 * @return the horizontal offset from the full / physical display
 */
lv_coord_t lv_disp_get_offset_x(lv_disp_t * disp);

/**
 * Get the vertical offset from the full / physical display
 * @param disp pointer to a display (NULL to use the default display)
 * @return the horizontal offset from the full / physical display
 */
lv_coord_t lv_disp_get_offset_y(lv_disp_t * disp);

/**
 * Get if anti-aliasing is enabled for a display or not
 * @param disp pointer to a display (NULL to use the default display)
 * @return true: anti-aliasing is enabled; false: disabled
 */
bool lv_disp_get_antialiasing(lv_disp_t * disp);

/**
 * Get the DPI of the display
 * @param disp pointer to a display (NULL to use the default display)
 * @return dpi of the display
 */
lv_coord_t lv_disp_get_dpi(const lv_disp_t * disp);


/**
 * Set the rotation of this display.
 * @param disp pointer to a display (NULL to use the default display)
 * @param rotation rotation angle
 */
void lv_disp_set_rotation(lv_disp_t * disp, lv_disp_rot_t rotation);

/**
 * Get the current rotation of this display.
 * @param disp pointer to a display (NULL to use the default display)
 * @return rotation angle
 */
lv_disp_rot_t lv_disp_get_rotation(lv_disp_t * disp);

//! @cond Doxygen_Suppress

/**
 * Call in the display driver's `flush_cb` function when the flushing is finished
 * @param disp_drv pointer to display driver in `flush_cb` where this function is called
 */
void /* LV_ATTRIBUTE_FLUSH_READY */ lv_disp_flush_ready(lv_disp_drv_t * disp_drv);

/**
 * Tell if it's the last area of the refreshing process.
 * Can be called from `flush_cb` to execute some special display refreshing if needed when all areas area flushed.
 * @param disp_drv pointer to display driver
 * @return true: it's the last area to flush; false: there are other areas too which will be refreshed soon
 */
bool /* LV_ATTRIBUTE_FLUSH_READY */ lv_disp_flush_is_last(lv_disp_drv_t * disp_drv);

//! @endcond

/**
 * Get the next display.
 * @param disp pointer to the current display. NULL to initialize.
 * @return the next display or NULL if no more. Give the first display when the parameter is NULL
 */
lv_disp_t * lv_disp_get_next(lv_disp_t * disp);

/**
 * Get the internal buffer of a display
 * @param disp pointer to a display
 * @return pointer to the internal buffers
 */
lv_disp_draw_buf_t * lv_disp_get_draw_buf(lv_disp_t * disp);

void lv_disp_drv_use_generic_set_px_cb(lv_disp_drv_t * disp_drv, lv_img_cf_t cf);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
