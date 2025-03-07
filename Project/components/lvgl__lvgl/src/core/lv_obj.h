/**
 * @file lv_obj.h
 *
 */

#ifndef LV_OBJ_H
#define LV_OBJ_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../lv_conf_internal.h"

#include <stddef.h>
#include <stdbool.h>
#include "../misc/lv_style.h"
#include "../misc/lv_types.h"
#include "../misc/lv_area.h"
#include "../misc/lv_color.h"
#include "../misc/lv_assert.h"
#include "../hal/lv_hal.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

struct _lv_obj_t;

/**
 * 控件的可能状态。
 * 可以使用 OR 运算符组合多个值
 */
enum {
    LV_STATE_DEFAULT     =  0x0000,  // 默认状态
    LV_STATE_CHECKED     =  0x0001,  // 已选中状态（如复选框、开关等被选中）
    LV_STATE_FOCUSED     =  0x0002,  // 焦点状态（元素获得焦点）
    LV_STATE_FOCUS_KEY   =  0x0004,  // 按键焦点状态（通过键盘导航获得焦点）
    LV_STATE_EDITED      =  0x0008,  // 编辑状态（如文本框处于编辑模式）
    LV_STATE_HOVERED     =  0x0010,  // 悬停状态（鼠标悬停在元素上）
    LV_STATE_PRESSED     =  0x0020,  // 按下状态（元素被按下）
    LV_STATE_SCROLLED    =  0x0040,  // 滚动状态（元素在滚动区域内滚动）
    LV_STATE_DISABLED    =  0x0080,  // 禁用状态（元素被禁用，不能交互）

    LV_STATE_USER_1      =  0x1000,  // 用户自定义状态1
    LV_STATE_USER_2      =  0x2000,  // 用户自定义状态2
    LV_STATE_USER_3      =  0x4000,  // 用户自定义状态3
    LV_STATE_USER_4      =  0x8000,  // 用户自定义状态4

    LV_STATE_ANY = 0xFFFF,    /**< 特殊值，可以在某些函数中用于目标所有状态 */
};

typedef uint16_t lv_state_t;

/**
 * 控件的可能部件。
 * 这些部件可以被视为控件的内部构建块。
 * 例如，滑块 = 背景 + 指示器 + 旋钮
 * 并非所有部件都被每个控件使用
 */
enum {
    LV_PART_MAIN         = 0x000000,   /**< 背景矩形 */
    LV_PART_SCROLLBAR    = 0x010000,   /**< 滚动条 */
    LV_PART_INDICATOR    = 0x020000,   /**< 指示器，例如滑块、进度条、开关或复选框的勾选框 */
    LV_PART_KNOB         = 0x030000,   /**< 旋钮，用于调整数值 */
    LV_PART_SELECTED     = 0x040000,   /**< 当前选中的选项或部分 */
    LV_PART_ITEMS        = 0x050000,   /**< 当控件有多个类似元素时使用（例如表格单元格） */
    LV_PART_TICKS        = 0x060000,   /**< 刻度，例如用于图表或仪表 */
    LV_PART_CURSOR       = 0x070000,   /**< 光标，例如用于文本区域的光标或图表上的标记 */

    LV_PART_CUSTOM_FIRST = 0x080000,    /**< 自定义控件的扩展点 */

    LV_PART_ANY          = 0x0F0000,    /**< 特殊值，可以在某些函数中用来定位所有部件 */
};

typedef uint32_t lv_part_t;

/**
 * 对象的行为控制的开关特性
 * 可以进行位或操作
 */
enum {
    LV_OBJ_FLAG_HIDDEN          = (1L << 0),  /**< 使对象被隐藏。（就像它根本不存在一样）*/
    LV_OBJ_FLAG_CLICKABLE       = (1L << 1),  /**< 使对象可以被输入设备点击*/
    LV_OBJ_FLAG_CLICK_FOCUSABLE = (1L << 2),  /**< 对象被点击时添加焦点状态*/
    LV_OBJ_FLAG_CHECKABLE       = (1L << 3),  /**< 对象被点击时切换选中状态*/
    LV_OBJ_FLAG_SCROLLABLE      = (1L << 4),  /**< 使对象可滚动*/
    LV_OBJ_FLAG_SCROLL_ELASTIC  = (1L << 5),  /**< 允许对象内部滚动，但速度较慢*/
    LV_OBJ_FLAG_SCROLL_MOMENTUM = (1L << 6),  /**< 使对象在“抛出”时滚动得更远*/
    LV_OBJ_FLAG_SCROLL_ONE      = (1L << 7),  /**< 仅允许滚动一个可捕捉的子对象*/
    LV_OBJ_FLAG_SCROLL_CHAIN_HOR = (1L << 8), /**< 允许将水平滚动传播到父级*/
    LV_OBJ_FLAG_SCROLL_CHAIN_VER = (1L << 9), /**< 允许将垂直滚动传播到父级*/
    LV_OBJ_FLAG_SCROLL_CHAIN     = (LV_OBJ_FLAG_SCROLL_CHAIN_HOR | LV_OBJ_FLAG_SCROLL_CHAIN_VER),   /**< 允许将水平以及垂直滚动传播到父级*/
    LV_OBJ_FLAG_SCROLL_ON_FOCUS = (1L << 10),  /**< 自动滚动对象，以使其在焦点状态下可见*/
    LV_OBJ_FLAG_SCROLL_WITH_ARROW  = (1L << 11), /**< 允许使用箭头键滚动焦点对象*/
    LV_OBJ_FLAG_SNAPPABLE       = (1L << 12), /**< 如果父级启用了滚动捕捉，则可以捕捉到该对象*/
    LV_OBJ_FLAG_PRESS_LOCK      = (1L << 13), /**< 即使按住的位置滑出了对象，也保持对象处于按下状态*/
    LV_OBJ_FLAG_EVENT_BUBBLE    = (1L << 14), /**< 将事件传播到父级*/
    LV_OBJ_FLAG_GESTURE_BUBBLE  = (1L << 15), /**< 将手势传播到父级*/
    LV_OBJ_FLAG_ADV_HITTEST     = (1L << 16), /**< 允许执行更准确的点击测试。例如，考虑圆角。*/
    LV_OBJ_FLAG_IGNORE_LAYOUT   = (1L << 17), /**< 通过布局设置对象位置*/
    LV_OBJ_FLAG_FLOATING        = (1L << 18), /**< 当父级滚动时，不滚动对象并忽略布局*/
    LV_OBJ_FLAG_OVERFLOW_VISIBLE = (1L << 19), /**< 不要修剪子对象的内容到父对象的边界*/

    LV_OBJ_FLAG_LAYOUT_1        = (1L << 23), /**< 自定义标志，布局可自由使用*/
    LV_OBJ_FLAG_LAYOUT_2        = (1L << 24), /**< 自定义标志，布局可自由使用*/

    LV_OBJ_FLAG_WIDGET_1        = (1L << 25), /**< 自定义标志，小部件可自由使用*/
    LV_OBJ_FLAG_WIDGET_2        = (1L << 26), /**< 自定义标志，小部件可自由使用*/
    LV_OBJ_FLAG_USER_1          = (1L << 27), /**< 自定义标志，用户可自由使用*/
    LV_OBJ_FLAG_USER_2          = (1L << 28), /**< 自定义标志，用户可自由使用*/
    LV_OBJ_FLAG_USER_3          = (1L << 29), /**< 自定义标志，用户可自由使用*/
    LV_OBJ_FLAG_USER_4          = (1L << 30), /**< 自定义标志，用户可自由使用*/
};


typedef uint32_t lv_obj_flag_t;

/**
 * `type` field in `lv_obj_draw_part_dsc_t` if `class_p = lv_obj_class`
 * Used in `LV_EVENT_DRAW_PART_BEGIN` and `LV_EVENT_DRAW_PART_END`
 */
typedef enum {
    LV_OBJ_DRAW_PART_RECTANGLE,  /**< The main rectangle*/
    LV_OBJ_DRAW_PART_BORDER_POST,/**< The border if style_border_post = true*/
    LV_OBJ_DRAW_PART_SCROLLBAR,  /**< The scrollbar*/
} lv_obj_draw_part_type_t;

#include "lv_obj_tree.h"
#include "lv_obj_pos.h"
#include "lv_obj_scroll.h"
#include "lv_obj_style.h"
#include "lv_obj_draw.h"
#include "lv_obj_class.h"
#include "lv_event.h"
#include "lv_group.h"

/**
 * Make the base object's class publicly available.
 */
extern const lv_obj_class_t lv_obj_class;

/**
 * 特殊的，很少使用的属性。
 * 如果设置了任何元素，它们会自动分配。
 */
typedef struct {
    struct _lv_obj_t ** children;       /**< 在数组中存储子对象的指针。*/
    uint32_t child_cnt;                 /**< 子对象数量*/
    lv_group_t * group_p;

    struct _lv_event_dsc_t * event_dsc; /**< 动态分配的事件回调和用户数据数组*/
    lv_point_t scroll;                  /**< 当前的 X/Y 滚动偏移*/

    lv_coord_t ext_click_pad;           /**< 各个方向的额外点击填充*/
    lv_coord_t ext_draw_size;           /**< 各个方向上的绘图扩展大小。*/

    lv_scrollbar_mode_t scrollbar_mode : 2; /**< 如何显示滚动条*/
    lv_scroll_snap_t scroll_snap_x : 2;     /**< 在水平方向对齐可捕捉的子对象的位置*/
    lv_scroll_snap_t scroll_snap_y : 2;     /**< 在垂直方向对齐可捕捉的子对象的位置*/
    lv_dir_t scroll_dir : 4;                /**< 允许的滚动方向*/
    uint8_t event_dsc_cnt : 6;              /**< 存储在 `event_dsc` 数组中的事件回调数量*/
    uint8_t layer_type : 2;    /**< 缓存的层类型。@lv_intermediate_layer_type_t 的元素 */
} _lv_obj_spec_attr_t;

typedef struct _lv_obj_t {
    const lv_obj_class_t * class_p;      /**< 指向对象类的指针。*/
    struct _lv_obj_t * parent;           /**< 指向父对象的指针。*/
    _lv_obj_spec_attr_t * spec_attr;     /**< 指向特殊属性的指针。*/
    _lv_obj_style_t * styles;            /**< 指向样式的指针。*/
#if LV_USE_USER_DATA
    void * user_data;                    /**< 用户数据。*/
#endif
    lv_area_t coords;                    /**< 对象的坐标。*/
    lv_obj_flag_t flags;                 /**< 对象的标志。*/
    lv_state_t state;                    /**< 对象的状态。*/
    uint16_t layout_inv : 1;             /**< 布局无效标志。*/
    uint16_t readjust_scroll_after_layout : 1; /**< 布局后重新调整滚动标志。*/
    uint16_t scr_layout_inv : 1;         /**< 屏幕布局无效标志。*/
    uint16_t skip_trans : 1;             /**< 跳过过渡标志。*/
    uint16_t style_cnt  : 6;             /**< 样式数量。*/
    uint16_t h_layout   : 1;             /**< 水平布局标志。*/
    uint16_t w_layout   : 1;             /**< 垂直布局标志。*/
    uint16_t being_deleted   : 1;        /**< 正在删除标志。*/
} lv_obj_t;



/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize LVGL library.
 * Should be called before any other LVGL related function.
 */
void lv_init(void);

#if LV_ENABLE_GC || !LV_MEM_CUSTOM

/**
 * Deinit the 'lv' library
 * Currently only implemented when not using custom allocators, or GC is enabled.
 */
void lv_deinit(void);

#endif

/**
 * Returns whether the 'lv' library is currently initialized
 */
bool lv_is_initialized(void);

/**
 * Create a base object (a rectangle)
 * @param parent    pointer to a parent object. If NULL then a screen will be created.
 * @return          pointer to the new object
 */
lv_obj_t * lv_obj_create(lv_obj_t * parent);


/*=====================
 * Setter functions
 *====================*/

/**
 * Set one or more flags
 * @param obj   pointer to an object
 * @param f     R-ed values from `lv_obj_flag_t` to set.
 */
void lv_obj_add_flag(lv_obj_t * obj, lv_obj_flag_t f);

/**
 * Clear one or more flags
 * @param obj   pointer to an object
 * @param f     OR-ed values from `lv_obj_flag_t` to set.
 */
void lv_obj_clear_flag(lv_obj_t * obj, lv_obj_flag_t f);


/**
 * Add one or more states to the object. The other state bits will remain unchanged.
 * If specified in the styles, transition animation will be started from the previous state to the current.
 * @param obj       pointer to an object
 * @param state     the states to add. E.g `LV_STATE_PRESSED | LV_STATE_FOCUSED`
 */
void lv_obj_add_state(lv_obj_t * obj, lv_state_t state);

/**
 * Remove one or more states to the object. The other state bits will remain unchanged.
 * If specified in the styles, transition animation will be started from the previous state to the current.
 * @param obj       pointer to an object
 * @param state     the states to add. E.g `LV_STATE_PRESSED | LV_STATE_FOCUSED`
 */
void lv_obj_clear_state(lv_obj_t * obj, lv_state_t state);

/**
 * Set the user_data field of the object
 * @param obj   pointer to an object
 * @param user_data   pointer to the new user_data.
 */
#if LV_USE_USER_DATA
static inline void lv_obj_set_user_data(lv_obj_t * obj, void * user_data)
{
    obj->user_data = user_data;
}
#endif

/*=======================
 * Getter functions
 *======================*/

/**
 * Check if a given flag or all the given flags are set on an object.
 * @param obj   pointer to an object
 * @param f     the flag(s) to check (OR-ed values can be used)
 * @return      true: all flags are set; false: not all flags are set
 */
bool lv_obj_has_flag(const lv_obj_t * obj, lv_obj_flag_t f);

/**
 * Check if a given flag or any of the flags are set on an object.
 * @param obj   pointer to an object
 * @param f     the flag(s) to check (OR-ed values can be used)
 * @return      true: at lest one flag flag is set; false: none of the flags are set
 */
bool lv_obj_has_flag_any(const lv_obj_t * obj, lv_obj_flag_t f);

/**
 * Get the state of an object
 * @param obj   pointer to an object
 * @return      the state (OR-ed values from `lv_state_t`)
 */
lv_state_t lv_obj_get_state(const lv_obj_t * obj);

/**
 * Check if the object is in a given state or not.
 * @param obj       pointer to an object
 * @param state     a state or combination of states to check
 * @return          true: `obj` is in `state`; false: `obj` is not in `state`
 */
bool lv_obj_has_state(const lv_obj_t * obj, lv_state_t state);

/**
 * Get the group of the object
 * @param       obj pointer to an object
 * @return      the pointer to group of the object
 */
void * lv_obj_get_group(const lv_obj_t * obj);

/**
 * Get the user_data field of the object
 * @param obj   pointer to an object
 * @return      the pointer to the user_data of the object
 */
#if LV_USE_USER_DATA
static inline void * lv_obj_get_user_data(lv_obj_t * obj)
{
    return obj->user_data;
}
#endif

/*=======================
 * Other functions
 *======================*/

/**
 * Allocate special data for an object if not allocated yet.
 * @param obj   pointer to an object
 */
void lv_obj_allocate_spec_attr(lv_obj_t * obj);

/**
 * Check the type of obj.
 * @param obj       pointer to an object
 * @param class_p   a class to check (e.g. `lv_slider_class`)
 * @return          true: `class_p` is the `obj` class.
 */
bool lv_obj_check_type(const lv_obj_t * obj, const lv_obj_class_t * class_p);

/**
 * Check if any object has a given class (type).
 * It checks the ancestor classes too.
 * @param obj       pointer to an object
 * @param class_p   a class to check (e.g. `lv_slider_class`)
 * @return          true: `obj` has the given class
 */
bool lv_obj_has_class(const lv_obj_t * obj, const lv_obj_class_t * class_p);

/**
 * Get the class (type) of the object
 * @param obj   pointer to an object
 * @return      the class (type) of the object
 */
const lv_obj_class_t * lv_obj_get_class(const lv_obj_t * obj);

/**
 * Check if any object is still "alive".
 * @param obj       pointer to an object
 * @return          true: valid
 */
bool lv_obj_is_valid(const lv_obj_t * obj);

/**
 * Scale the given number of pixels (a distance or size) relative to a 160 DPI display
 * considering the DPI of the `obj`'s display.
 * It ensures that e.g. `lv_dpx(100)` will have the same physical size regardless to the
 * DPI of the display.
 * @param obj   an object whose display's dpi should be considered
 * @param n     the number of pixels to scale
 * @return      `n x current_dpi/160`
 */
static inline lv_coord_t lv_obj_dpx(const lv_obj_t * obj, lv_coord_t n)
{
    return _LV_DPX_CALC(lv_disp_get_dpi(lv_obj_get_disp(obj)), n);
}

/**********************
 *      MACROS
 **********************/

#if LV_USE_ASSERT_OBJ
#  define LV_ASSERT_OBJ(obj_p, obj_class)                                                               \
    do {                                                                                                \
        LV_ASSERT_MSG(obj_p != NULL, "The object is NULL");                                             \
        LV_ASSERT_MSG(lv_obj_has_class(obj_p, obj_class) == true, "Incompatible object type.");         \
        LV_ASSERT_MSG(lv_obj_is_valid(obj_p)  == true, "The object is invalid, deleted or corrupted?"); \
    } while(0)
# else
#  define LV_ASSERT_OBJ(obj_p, obj_class) do{}while(0)
#endif

#if LV_USE_LOG && LV_LOG_TRACE_OBJ_CREATE
#  define LV_TRACE_OBJ_CREATE(...) LV_LOG_TRACE(__VA_ARGS__)
#else
#  define LV_TRACE_OBJ_CREATE(...)
#endif


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_OBJ_H*/
