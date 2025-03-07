
#include "user_gui.h"
#include "user_new_source.h"

static const char *TAG = "user_gui";

#define COLOR_TORRAS    0xFF7602

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if     1       // example

// 示例:对齐
void lvgl_example_align(void)
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_t *label;

    // 居中对齐
    label = lv_label_create(scr);
    lv_label_set_text(label, "Center");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    // 左上角对齐
    label = lv_label_create(scr);
    lv_label_set_text(label, "Top Left");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);

    // 顶部中央对齐
    label = lv_label_create(scr);
    lv_label_set_text(label, "Top Mid");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);

    // 右上角对齐
    label = lv_label_create(scr);
    lv_label_set_text(label, "Top Right");
    lv_obj_align(label, LV_ALIGN_TOP_RIGHT, 0, 0);

    // 左下角对齐
    label = lv_label_create(scr);
    lv_label_set_text(label, "Bot Left");
    lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    // 底部中央对齐
    label = lv_label_create(scr);
    lv_label_set_text(label, "Bot Mid");
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, 0);

    // 右下角对齐
    label = lv_label_create(scr);
    lv_label_set_text(label, "Bot Right");
    lv_obj_align(label, LV_ALIGN_BOTTOM_RIGHT, 0, 0);

    // 左侧中间对齐
    label = lv_label_create(scr);
    lv_label_set_text(label, "Left Mid");
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);

    // 右侧中间对齐
    label = lv_label_create(scr);
    lv_label_set_text(label, "Right Mid");
    lv_obj_align(label, LV_ALIGN_RIGHT_MID, 0, 0);
}

// 示例:外部对齐
void lvgl_example_align_out(void)
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_t *ref_obj = lv_label_create(scr);
    lv_obj_t *label;

    lv_label_set_text(ref_obj, "Reference");
    lv_obj_align(ref_obj, LV_ALIGN_CENTER, 0, 0);  // 将参考对象居中

    // 左上角外部对齐
    label = lv_label_create(scr);
    lv_label_set_text(label, "TL");
    lv_obj_align_to(label, ref_obj, LV_ALIGN_OUT_TOP_LEFT, 0, 0);

    // 顶部中央外部对齐
    label = lv_label_create(scr);
    lv_label_set_text(label, "TM");
    lv_obj_align_to(label, ref_obj, LV_ALIGN_OUT_TOP_MID, 0, 0);

    // 右上角外部对齐
    label = lv_label_create(scr);
    lv_label_set_text(label, "TR");
    lv_obj_align_to(label, ref_obj, LV_ALIGN_OUT_TOP_RIGHT, 0, 0);

    // 左下角外部对齐
    label = lv_label_create(scr);
    lv_label_set_text(label, "BL");
    lv_obj_align_to(label, ref_obj, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

    // 底部中央外部对齐
    label = lv_label_create(scr);
    lv_label_set_text(label, "BM");
    lv_obj_align_to(label, ref_obj, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    // 右下角外部对齐
    label = lv_label_create(scr);
    lv_label_set_text(label, "BR");
    lv_obj_align_to(label, ref_obj, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0);

    // 左侧中间外部对齐
    label = lv_label_create(scr);
    lv_label_set_text(label, "LM");
    lv_obj_align_to(label, ref_obj, LV_ALIGN_OUT_LEFT_MID, -15, 0);

    // 右侧中间外部对齐
    label = lv_label_create(scr);
    lv_label_set_text(label, "RM");
    lv_obj_align_to(label, ref_obj, LV_ALIGN_OUT_RIGHT_MID, 15, 0);
}


// 示例:文本显示
void lvgl_example_value_cb(lv_timer_t *param);

lv_obj_t *iout_label;
uint16_t iout_value = 1000;
void lvgl_example_label(void)
{
    lv_obj_t *scr = lv_scr_act();

    // 创建样式
    static lv_style_t style1;
    lv_style_init(&style1);

    lv_style_set_text_font(&style1, &lv_font_montserrat_14);        // 设置字体
    lv_style_set_text_color(&style1, lv_color_hex(0xFFFFFF));       // 设置字体颜色
    //lv_style_set_bg_color(&style1, lv_color_hex(0x0000FF));       // 设置背景颜色
    //lv_style_set_bg_opa(&style1, LV_OPA_COVER);                   // 完全覆盖

    static lv_style_t style_main;
    lv_style_init(&style_main);

    lv_style_set_text_font(&style_main, &lv_font_montserrat_16);     // 设置字体
    lv_style_set_text_color(&style_main, lv_color_hex(0x000000));    // 设置字体颜色
    lv_style_set_bg_color(&style_main, lv_color_hex(COLOR_TORRAS));  // 设置背景颜色
    
    lv_style_set_bg_opa(&style_main, LV_OPA_COVER);                  // 完全覆盖
    

    lv_obj_t *label;
    
    // 固定字符串显示
    label = lv_label_create(scr);
    lv_label_set_text(label, "LVGL_V8.3.11");
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_style(label, &style1, 0);

    // 变量显示
    label = lv_label_create(scr);
    lv_label_set_text_fmt(label, "VOUT: %.3fV", 20.123);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 0, 0);
    lv_obj_add_style(label, &style_main, 0);

    // 动态变量显示
    iout_label = lv_label_create(scr);    
    lv_obj_align_to(iout_label, label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_obj_add_style(iout_label, &style_main, 0);

    lv_timer_create(lvgl_example_value_cb, 200, NULL);

}

void lvgl_example_value_cb(lv_timer_t *param)    
{
    iout_value+=10;
    if(iout_value >= 5000)
        iout_value = 0;

    lv_label_set_text_fmt(iout_label, "IOUT: %.2fA", (float)iout_value/1000);
    
}


// 示例:图象显示
void lvgl_example_img(void)
{
    LV_IMG_DECLARE(torras_jpg);
    lv_obj_t * img_jpg = lv_img_create(lv_scr_act());
    lv_img_set_src(img_jpg, &torras_jpg);
    lv_obj_align(img_jpg, LV_ALIGN_TOP_LEFT, 0, 0);

    LV_IMG_DECLARE(torras_png);
    lv_obj_t * img_png = lv_img_create(lv_scr_act());
    lv_img_set_src(img_png, &torras_png);
    lv_obj_align(img_png, LV_ALIGN_TOP_RIGHT, 0, 0);
}


// 示例:按钮
void btn_event_handler(lv_event_t *event) 
{
    lv_event_code_t code = lv_event_get_code(event);
    
    lv_obj_t *obj = lv_event_get_target(event);
    uint8_t *pi = lv_event_get_user_data(event);

    switch(code) 
    {
        case LV_EVENT_PRESSED:
            printf("PRESSED\n");
            break;
        case LV_EVENT_PRESSING:
            printf("PRESSING\n");
            break;
        case LV_EVENT_CLICKED:
            printf("CLICKED\n");
            break;
        case LV_EVENT_SHORT_CLICKED:
            printf("SHORT_CLICKED\n");
            break;
        case LV_EVENT_LONG_PRESSED:
            printf("LONG PRESSED\n");
            printf("obj = %d, user_data = %d\n", (int)obj, *pi);
            break;
        case LV_EVENT_LONG_PRESSED_REPEAT:
            printf("LONG PRESSED REPEAT\n");
            break;
        case LV_EVENT_RELEASED:
            printf("RELEASED\n");
            break;
        default:
            if(code >= LV_EVENT_PRESSED && code <= LV_EVENT_RELEASED)
                printf("OTHER = %d\n", code);
            break;
    }
}

void lvgl_example_btn(void)
{
    static lv_obj_t *button;
    // 创建按钮
    button = lv_btn_create(lv_scr_act());

    lv_obj_set_size(button, 100, 50);    
    lv_obj_align(button, LV_ALIGN_CENTER, 0, 0);    

    static uint8_t i = 123;

    lv_obj_add_event_cb(button, btn_event_handler, LV_EVENT_ALL, &i);
}

#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/// @brief  初始化默认主题
void init_theme_default(void)
{
    // 初始化默认主题
    lv_theme_t * th = lv_theme_default_init(NULL, lv_color_hex(COLOR_TORRAS), lv_color_hex(0x000000), true, LV_FONT_DEFAULT);

    // 将默认主题应用到当前屏幕
    lv_disp_set_theme(NULL, th);
}


#if     0       // 基于Tabview 选项卡的UI 1

lv_obj_t * label_com1;
lv_obj_t * label_com2;
lv_obj_t * label_com1_tab2;
lv_obj_t * label_com2_tab2;

// 数据变量
float com1_voltage = 5.00;
float com1_current = 3.00;
float com1_power = 15.00;
uint8_t com1_max_power = 80;

float com2_voltage = 9.00;
float com2_current = 3.00;
float com2_power = 18.00;
uint8_t com2_max_power = 20;

char* com1_protocol = "protocol";
char* com2_protocol = "protoco2";

void lvgl_timer_cb_charge_info(lv_timer_t *timer);
void lvgl_timer_cb_protocol(lv_timer_t *timer);
    
void tabview1_charge_info(lv_obj_t * parent)
{
    // 为tab标签创建一个容器(container)
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, 220, 180);    // LV_SIZE_CONTENT表示容器的尺寸根据其内容自动调整
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 0);
    // 将容器 cont 的布局设置为灵活布局(Flex layout).灵活布局允许容器内的子对象自动排列和调整.
    //lv_obj_set_layout(cont, LV_LAYOUT_FLEX);
    // 设置灵活布局的流动方向为行方向(水平排列).这意味着容器内的子对象将从左到右排列.
    //lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);

    // 设置灵活布局的对齐方式.主轴(水平轴)上的对齐方式为居中,交叉轴(垂直轴)上的对齐方式为居中,子对象之间的间距对齐方式为居中.
    //lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);


    // 创建样式
    static lv_style_t style_main;
    lv_style_init(&style_main);

   

    lv_style_set_text_font(&style_main, );        // 设置字体大小
    //lv_style_set_text_color(&style_main, lv_color_hex(0xFFFFFF));       // 设置字体颜色
    //lv_style_set_bg_color(&style_main, lv_color_hex(0xF5F5F5));           // 设置背景颜色
    //lv_style_set_bg_opa(&style_main, LV_OPA_COVER);                   // 完全覆盖


    // 为 COM1 创建标签
    label_com1 = lv_label_create(cont);
    // lv_label_set_text_fmt(label_com1, "COM1\nV:%.2fV\nI:%.2fA\nP:%.2fW\nMAX:%dW", com1_voltage, com1_current, com1_power, com1_max_power);
    lv_obj_set_style_text_color(label_com1, lv_color_hex(COLOR_TORRAS), 0);  // 设置字体颜色
    lv_obj_align(label_com1, LV_ALIGN_TOP_LEFT, -5, 0);
    lv_obj_add_style(label_com1, &style_main, 0);


    // 为 COM2 创建标签
    label_com2 = lv_label_create(cont);
    // lv_label_set_text_fmt(label_com2, "COM2\nV:%.2fV\nI:%.2fA\nP:%.2fW\nMAX:%dW\n%s", com2_voltage, com2_current, com2_power, com2_max_power, com2_protocol);
    lv_obj_set_style_text_color(label_com2, lv_color_hex(COLOR_TORRAS), 0);  // 设置字体颜色
    lv_obj_align(label_com2, LV_ALIGN_TOP_LEFT, 100, 0);
    lv_obj_add_style(label_com2, &style_main, 0);


    lv_timer_create(lvgl_timer_cb_charge_info, 500, NULL);
}


void get_com_protocol(uint8_t curr_protocol, char* com_protocol_str)
{
    switch(curr_protocol)
    {
        case 0:          
          strcpy(com_protocol_str, "DCP");
        //   strcpy(com_protocol_str, "Trickle");
        break;
        case 1:
          strcpy(com_protocol_str, "QC2");
        // strcpy(com_protocol_str, "Unknow");
        break;
        case 2:
          strcpy(com_protocol_str, "QC3");
        // strcpy(com_protocol_str, "Unknow");
        break;
        case 3:        
          strcpy(com_protocol_str, "FCP");
        // strcpy(com_protocol_str, "HUAWEI");
        break;
        case 4:          
          strcpy(com_protocol_str, "SCP");
        // strcpy(com_protocol_str, "HUAWEI");
        break;
        case 5:          
          strcpy(com_protocol_str, "PD FIX");
        // strcpy(com_protocol_str, "Unknow");
        break;
        case 6:        
          strcpy(com_protocol_str, "PD PPS");
        // strcpy(com_protocol_str, "Unknow");
        break;
        case 7:          
          strcpy(com_protocol_str, "PE1");
        // strcpy(com_protocol_str, "Unknow");
        break;
        case 8:          
          strcpy(com_protocol_str, "PE2");
        // strcpy(com_protocol_str, "Unknow");
        break;
        case 10:
          strcpy(com_protocol_str, "SFCP");
        // strcpy(com_protocol_str, "HUAWEI");
        break;
        case 11:    
          strcpy(com_protocol_str, "AFC");
        // strcpy(com_protocol_str, "Samsung");
        break;
        default:          
          strcpy(com_protocol_str, "OTHER");
        // strcpy(com_protocol_str, "Unknow");
        break;
    }
}


void lvgl_timer_cb_charge_info(lv_timer_t *param)    
{
    if (lvgl_lock(-1)) 
    {
        com1_voltage = (float)PORT[0].curr_vout/1000;
        com1_current = (float)PORT[0].curr_iout/1000;
        com1_power = com1_voltage*com1_current;// (float)PORT[0].curr_power;
        com1_max_power = PORT[0].curr_power_pd;

        com2_voltage = (float)PORT[1].curr_vout/1000;
        com2_current = (float)PORT[1].curr_iout/1000;
        com2_power = com2_voltage*com2_current;// (float)PORT[1].curr_power;
        com2_max_power = PORT[1].curr_power_pd;

        if(PORT[0].port_state)
        {
            get_com_protocol(PORT[0].curr_protocol[2], com1_protocol);
        }
        else
        {
            strcpy(com1_protocol,"OFF");
            com1_max_power = 0;
            com1_voltage = 0;
        }

        if(PORT[1].port_state)
        {
            get_com_protocol(PORT[1].curr_protocol[2], com2_protocol);
        }
        else
        {
            strcpy(com2_protocol,"OFF");
            com2_max_power = 0;
            com2_voltage = 0;
        }        

        lv_label_set_text_fmt(label_com1, "COM1\n\nV:%.2fV\nI:%.2fA\nP:%.2fW\nPD:%dW\n%s", com1_voltage, com1_current, com1_power, com1_max_power, com1_protocol);
        lv_label_set_text_fmt(label_com2, "COM2\n\nV:%.2fV\nI:%.2fA\nP:%.2fW\nPD:%dW\n%s", com2_voltage, com2_current, com2_power, com2_max_power, com2_protocol);

        lvgl_unlock();
    }
}

void lvgl_timer_cb_protocol(lv_timer_t *param)    
{
    int C1_POWER = com1_max_power;
    float V5_current_c1 = 0;float V9_current_c1 = 0;
    float V12_current_c1 = 0;float V15_current_c1 = 0;
    float V20_current_c1 = 0;float V11_current_c1 = 0;

    int C2_POWER = com2_max_power;
    float V5_current_c2 = 0;float V9_current_c2 = 0;
    float V12_current_c2 = 0;float V15_current_c2 = 0;
    float V20_current_c2 = 0;float V11_current_c2 = 0;

    if (lvgl_lock(-1)) 
    {
        if (C1_POWER==0)
        {
            V5_current_c1 = 0;V9_current_c1 = 0;
            V12_current_c1 = 0;V15_current_c1 = 0;
            V20_current_c1 = 0;V11_current_c1 = 0;
        }
        else
        {
            V5_current_c1 = (C1_POWER/5 >= 3) ? 3:(C1_POWER*1000/5 - (float)(C1_POWER*1000%10))/1000;
            V9_current_c1 = (C1_POWER/9 >= 3) ? 3:(C1_POWER*1000/9 - (float)(C1_POWER*1000%10))/1000;
            V12_current_c1 = (C1_POWER/12 >= 3) ? 3:(C1_POWER*1000/12 - (float)(C1_POWER*1000%10))/1000;
            V15_current_c1 = (C1_POWER/15 >= 3) ? 3:(C1_POWER*1000/15 - (float)(C1_POWER*1000%10))/1000;
            V20_current_c1 = (C1_POWER/20 >= 5) ? 5:(C1_POWER*1000/20 - (float)(C1_POWER*1000%10))/1000;
            V11_current_c1 = (C1_POWER/11 >= 5) ? 5:(C1_POWER*1000/11 - (float)(C1_POWER*1000%50))/1000;
        }
        if (C2_POWER==0)
        {
            V5_current_c2 = 0;V9_current_c2 = 0;
            V12_current_c2 = 0;V15_current_c2 = 0;
            V20_current_c2 = 0;V11_current_c2 = 0;
        }
        else
        {
            V5_current_c2 = (C2_POWER/5 >= 3) ? 3:(C2_POWER*1000/5 - (float)(C2_POWER*1000%10))/1000;
            V9_current_c2 = (C2_POWER/9 >= 3) ? 3:(C2_POWER*1000/9 - (float)(C2_POWER*1000%10))/1000;
            V12_current_c2 = (C2_POWER/12 >= 3) ? 3:(C2_POWER*1000/12 - (float)(C2_POWER*1000%10))/1000;
            V15_current_c2 = (C2_POWER/15 >= 3) ? 3:(C2_POWER*1000/15 - (float)(C2_POWER*1000%10))/1000;
            V20_current_c2 = (C2_POWER/20 >= 5) ? 5:(C2_POWER*1000/20 - (float)(C2_POWER*1000%10))/1000;
            V11_current_c2 = (C2_POWER/11 >= 5) ? 5:(C2_POWER*1000/11 - (float)(C2_POWER*1000%50))/1000;
        }

        lv_label_set_text_fmt(label_com1_tab2, "COM1\n5 V%.2fA\n9 V%.2fA\n12V%.2fA\n15V%.2fA\n20V%.2fA\n3.3-11V%.1fA\n", V5_current_c1, V9_current_c1, V12_current_c1, V15_current_c1, V20_current_c1, V11_current_c1);
        lv_label_set_text_fmt(label_com2_tab2, "COM2\n5 V%.2fA\n9 V%.2fA\n12V%.2fA\n15V%.2fA\n20V%.2fA\n3.3-11V%.1fA\n", V5_current_c2, V9_current_c2, V12_current_c2, V15_current_c2, V20_current_c2, V11_current_c2);

        lvgl_unlock();
    }
}

void tabview2_protocol(lv_obj_t * parent)
{
    // 为tab标签创建一个容器(container)
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, 220, 180);    // LV_SIZE_CONTENT表示容器的尺寸根据其内容自动调整
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 0);

    // 设置滚动条模式为关闭
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);

    // 创建样式
    static lv_style_t style_main;
    lv_style_init(&style_main);

    lv_style_set_text_font(&style_main, &lv_font_montserrat_18);        // 设置字体大小

    // 为 COM1 创建标签
    label_com1_tab2 = lv_label_create(cont);
    // lv_label_set_text_fmt(label_com1_tab2, "COM1\nV:%.2fV\nI:%.2fA\nP:%.2fW\nMAX:%dW", com1_voltage, com1_current, com1_power, com1_max_power);
    lv_obj_set_style_text_color(label_com1_tab2, lv_color_hex(COLOR_TORRAS), 0);  // 设置字体颜色
    lv_obj_align(label_com1_tab2, LV_ALIGN_TOP_LEFT, -5, 0);
    lv_obj_add_style(label_com1_tab2, &style_main, 0);

    // 为 COM2 创建标签
    label_com2_tab2 = lv_label_create(cont);
    // lv_label_set_text_fmt(label_com2_tab2, "COM2\nV:%.2fV\nI:%.2fA\nP:%.2fW\nMAX:%dW\n%s", com2_voltage, com2_current, com2_power, com2_max_power, com2_protocol);
    lv_obj_set_style_text_color(label_com2_tab2, lv_color_hex(COLOR_TORRAS), 0);  // 设置字体颜色
    lv_obj_align(label_com2_tab2, LV_ALIGN_TOP_LEFT, 100, 0);
    lv_obj_add_style(label_com2_tab2, &style_main, 0);


    lv_timer_create(lvgl_timer_cb_protocol, 500, NULL);
}

//chart//begin//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static lv_obj_t * chart;
static const lv_coord_t ecg_sample[] = {
    -2, 2, 0, -15, -39, -63, -71, -68, -67, -69, -84, -95, -104, -107, -108, -107, -107, -107, -107, -114, -118, -117,
        -112, -100, -89, -83, -71, -64, -58, -58, -62, -62, -58, -51, -46, -39, -27, -10, 4, 7, 1, -3, 0, 14, 24, 30, 25, 19,
        13, 7, 12, 15, 18, 21, 13, 6, 9, 8, 17, 19, 13, 11, 11, 11, 23, 30, 37, 34, 25, 14, 15, 19, 28, 31, 26, 23, 25, 31,
        39, 37, 37, 34, 30, 32, 22, 29, 31, 33, 37, 23, 13, 7, 2, 4, -2, 2, 11, 22, 33, 19, -1, -27, -55, -67, -72, -71, -63,
        -49, -18, 35, 113, 230, 369, 525, 651, 722, 730, 667, 563, 454, 357, 305, 288, 274, 255, 212, 173, 143, 117, 82, 39,
        -13, -53, -78, -91, -101, -113, -124, -131, -131, -131, -129, -128, -129, -125, -123, -123, -129, -139, -148, -153,
        -159, -166, -183, -205, -227, -243, -248, -246, -254, -280, -327, -381, -429, -473, -517, -556, -592, -612, -620,
        -620, -614, -604, -591, -574, -540, -497, -441, -389, -358, -336, -313, -284, -222, -167, -114, -70, -47, -28, -4, 12,
        38, 52, 58, 56, 56, 57, 68, 77, 86, 86, 80, 69, 67, 70, 82, 85, 89, 90, 89, 89, 88, 91, 96, 97, 91, 83, 78, 82, 88, 95,
        96, 105, 106, 110, 102, 100, 96, 98, 97, 101, 98, 99, 100, 107, 113, 119, 115, 110, 96, 85, 73, 64, 69, 76, 79,
        78, 75, 85, 100, 114, 113, 105, 96, 84, 74, 66, 60, 75, 85, 89, 83, 67, 61, 67, 73, 79, 74, 63, 57, 56, 58, 61, 55,
        48, 45, 46, 55, 62, 55, 49, 43, 50, 59, 63, 57, 40, 31, 23, 25, 27, 31, 35, 34, 30, 36, 34, 42, 38, 36, 40, 46, 50,
        47, 32, 30, 32, 52, 67, 73, 71, 63, 54, 53, 45, 41, 28, 13, 3, 1, 4, 4, -8, -23, -32, -31, -19, -5, 3, 9, 13, 19,
        24, 27, 29, 25, 22, 26, 32, 42, 51, 56, 60, 57, 55, 53, 53, 54, 59, 54, 49, 26, -3, -11, -20, -47, -100, -194, -236,
        -212, -123, 8, 103, 142, 147, 120, 105, 98, 93, 81, 61, 40, 26, 28, 30, 30, 27, 19, 17, 21, 20, 19, 19, 22, 36, 40,
        35, 20, 7, 1, 10, 18, 27, 22, 6, -4, -2, 3, 6, -2, -13, -14, -10, -2, 3, 2, -1, -5, -10, -19, -32, -42, -55, -60,
        -68, -77, -86, -101, -110, -117, -115, -104, -92, -84, -85, -84, -73, -65, -52, -50, -45, -35, -20, -3, 12, 20, 25,
        26, 28, 28, 30, 28, 25, 28, 33, 42, 42, 36, 23, 9, 0, 1, -4, 1, -4, -4, 1, 5, 9, 9, -3, -1, -18, -50, -108, -190,
        -272, -340, -408, -446, -537, -643, -777, -894, -920, -853, -697, -461, -251, -60, 58, 103, 129, 139, 155, 170, 173,
        178, 185, 190, 193, 200, 208, 215, 225, 224, 232, 234, 240, 240, 236, 229, 226, 224, 232, 233, 232, 224, 219, 219,
        223, 231, 226, 223, 219, 218, 223, 223, 223, 233, 245, 268, 286, 296, 295, 283, 271, 263, 252, 243, 226, 210, 197,
        186, 171, 152, 133, 117, 114, 110, 107, 96, 80, 63, 48, 40, 38, 34, 28, 15, 2, -7, -11, -14, -18, -29, -37, -44, -50,
        -58, -63, -61, -52, -50, -48, -61, -59, -58, -54, -47, -52, -62, -61, -64, -54, -52, -59, -69, -76, -76, -69, -67,
        -74, -78, -81, -80, -73, -65, -57, -53, -51, -47, -35, -27, -22, -22, -24, -21, -17, -13, -10, -11, -13, -20, -20,
        -12, -2, 7, -1, -12, -16, -13, -2, 2, -4, -5, -2, 9, 19, 19, 14, 11, 13, 19, 21, 20, 18, 19, 19, 19, 16, 15, 13, 14,
        9, 3, -5, -9, -5, -3, -2, -3, -3, 2, 8, 9, 9, 5, 6, 8, 8, 7, 4, 3, 4, 5, 3, 5, 5, 13, 13, 12, 10, 10, 15, 22, 17,
        14, 7, 10, 15, 16, 11, 12, 10, 13, 9, -2, -4, -2, 7, 16, 16, 17, 16, 7, -1, -16, -18, -16, -9, -4, -5, -10, -9, -8,
        -3, -4, -10, -19, -20, -16, -9, -9, -23, -40, -48, -43, -33, -19, -21, -26, -31, -33, -19, 0, 17, 24, 9, -17, -47,
        -63, -67, -59, -52, -51, -50, -49, -42, -26, -21, -15, -20, -23, -22, -19, -12, -8, 5, 18, 27, 32, 26, 25, 26, 22,
        23, 17, 14, 17, 21, 25, 2, -45, -121, -196, -226, -200, -118, -9, 73, 126, 131, 114, 87, 60, 42, 29, 26, 34, 35, 34,
        25, 12, 9, 7, 3, 2, -8, -11, 2, 23, 38, 41, 23, 9, 10, 13, 16, 8, -8, -17, -23, -26, -25, -21, -15, -10, -13, -13,
        -19, -22, -29, -40, -48, -48, -54, -55, -66, -82, -85, -90, -92, -98, -114, -119, -124, -129, -132, -146, -146, -138,
        -124, -99, -85, -72, -65, -65, -65, -66, -63, -64, -64, -58, -46, -26, -9, 2, 2, 4, 0, 1, 4, 3, 10, 11, 10, 2, -4,
        0, 10, 18, 20, 6, 2, -9, -7, -3, -3, -2, -7, -12, -5, 5, 24, 36, 31, 25, 6, 3, 7, 12, 17, 11, 0, -6, -9, -8, -7, -5,
        -6, -2, -2, -6, -2, 2, 14, 24, 22, 15, 8, 4, 6, 7, 12, 16, 25, 20, 7, -16, -41, -60, -67, -65, -54, -35, -11, 30,
        84, 175, 302, 455, 603, 707, 743, 714, 625, 519, 414, 337, 300, 281, 263, 239, 197, 163, 136, 109, 77, 34, -18, -50,
        -66, -74, -79, -92, -107, -117, -127, -129, -135, -139, -141, -155, -159, -167, -171, -169, -174, -175, -178, -191,
        -202, -223, -235, -243, -237, -240, -256, -298, -345, -393, -432, -475, -518, -565, -596, -619, -623, -623, -614,
        -599, -583, -559, -524, -477, -425, -383, -357, -331, -301, -252, -198, -143, -96, -57, -29, -8, 10, 31, 45, 60, 65,
        70, 74, 76, 79, 82, 79, 75, 62,
    };
// static void slider_x_event_cb(lv_event_t * e)
// {
//     lv_obj_t * obj = lv_event_get_target(e);
//     int32_t v = lv_slider_get_value(obj);
//     lv_chart_set_zoom_x(chart, v);
// }

// static void slider_y_event_cb(lv_event_t * e)
// {
//     lv_obj_t * obj = lv_event_get_target(e);
//     int32_t v = lv_slider_get_value(obj);
//     lv_chart_set_zoom_y(chart, v);
// }

/**
 * Display 1000 data points with zooming and scrolling.
 * See how the chart changes drawing mode (draw only vertical lines) when
 * the points get too crowded.
 */
void lv_example_chart_5(lv_obj_t * panel1)
{
    /*Create a chart*/
    // chart = lv_chart_create(lv_scr_act());
    chart = lv_chart_create(panel1);
    lv_obj_set_size(chart, 210, 180);
    lv_obj_align(chart, LV_ALIGN_CENTER, -30, -30);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, -1000, 1000);

    /*Do not display points on the data*/
    lv_obj_set_style_size(chart, 0, LV_PART_INDICATOR);

    lv_chart_series_t * ser = lv_chart_add_series(chart, lv_color_hex(COLOR_TORRAS), LV_CHART_AXIS_PRIMARY_Y);

    uint32_t pcnt = sizeof(ecg_sample) / sizeof(ecg_sample[0]);
    lv_chart_set_point_count(chart, pcnt);
    lv_chart_set_ext_y_array(chart, ser, (lv_coord_t *)ecg_sample);

    // // 设置Y1轴的标签和刻度
    // // lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_Y, 10, 5, 5, 1, true, 50);

    // // lv_chart_series_t * ser1 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_GREEN), LV_CHART_AXIS_PRIMARY_Y);
    // lv_chart_series_t * ser1 = lv_chart_add_series(chart, lv_color_hex(0xFFFFFF), LV_CHART_AXIS_PRIMARY_Y);

    // uint32_t pcnt1 = sizeof(ecg_sample1) / sizeof(ecg_sample1[0]);
    // lv_chart_set_point_count(chart, pcnt1);
    // lv_chart_set_ext_y_array(chart, ser1, (lv_coord_t *)ecg_sample1);

    // // 设置Y2轴的标签和刻度
    // lv_chart_set_axis_tick(chart, LV_CHART_AXIS_SECONDARY_Y, 10, 5, 5, 1, true, 50);

    // lv_obj_t * slider;

    // // slider = lv_slider_create(lv_scr_act());
    // slider = lv_slider_create(panel1);
    // lv_slider_set_range(slider, LV_IMG_ZOOM_NONE, LV_IMG_ZOOM_NONE * 10);
    // lv_obj_add_event_cb(slider, slider_x_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    // lv_obj_set_size(slider, 200, 10);
    // lv_obj_align_to(slider, chart, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

    // // slider = lv_slider_create(lv_scr_act());
    // slider = lv_slider_create(panel1);
    // lv_slider_set_range(slider, LV_IMG_ZOOM_NONE, LV_IMG_ZOOM_NONE * 10);
    // lv_obj_add_event_cb(slider, slider_y_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    // lv_obj_set_size(slider, 10, 150);
    // lv_obj_align_to(slider, chart, LV_ALIGN_OUT_RIGHT_MID, 20, 0);
}

void tabview3_other(lv_obj_t * parent)
{
    // 为tab标签创建一个容器(container)
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, 220, 180);    // LV_SIZE_CONTENT表示容器的尺寸根据其内容自动调整
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 0);

    // 设置滚动条模式为关闭
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t * panel1 = lv_obj_create(cont);
    lv_obj_set_size(panel1, 210, 180);    // LV_SIZE_CONTENT表示容器的尺寸根据其内容自动调整
    lv_example_chart_5(panel1);

    // 设置滚动条模式为关闭
    lv_obj_set_scrollbar_mode(panel1, LV_SCROLLBAR_MODE_OFF);
}
//chart//end//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int tab_num = 0;

void btn_event_cb_tab_next(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);

    // lv_event_code_t code = lv_event_get_code(e);  
    // lv_indev_data_t * data = lv_event_get_user_data(e);  
  
    if(event_code == LV_EVENT_KEY)
     {  
        switch(data_key) {  
            case LV_KEY_LEFT:  
                tab_num++; 
                break;  
            case LV_KEY_RIGHT:  
                tab_num--;  
                break;  
            // ... 其他按键  
        }
    }

    if ( (tab_num>2) ) 
        tab_num = 0;
    else if ( (tab_num<0) ) 
        tab_num = 2;
    lv_tabview_set_act(obj, tab_num, LV_ANIM_ON); 
    vTaskDelay(pdMS_TO_TICKS(150));
}

void power_main_ui_init(void)
{
    lv_obj_t * scr = lv_scr_act();

    init_theme_default();

    // 创建带有三个标签的 tabview
    lv_obj_t * tabview = lv_tabview_create(scr, LV_DIR_TOP, 40);
    // 禁用 TabView 的滚动条
    lv_obj_set_scrollbar_mode(tabview, LV_SCROLLBAR_MODE_OFF);
    //lv_obj_set_size(tabview, 240, 240);

    // 向 tabview 添加标签
    lv_obj_t * tab1 = lv_tabview_add_tab(tabview, "Charge");
    lv_obj_t * tab2 = lv_tabview_add_tab(tabview, "Protocol");
    lv_obj_t * tab3 = lv_tabview_add_tab(tabview, "Other");

    // 禁用每个 Tab 页的滚动条
    lv_obj_set_scrollbar_mode(tab1, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scrollbar_mode(tab2, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scrollbar_mode(tab3, LV_SCROLLBAR_MODE_OFF);

    lv_obj_set_style_bg_color(tab1, lv_color_hex(0x000000), LV_PART_ANY);

    tabview1_charge_info(tab1);
    tabview2_protocol(tab2);
    tabview3_other(tab3);

    lv_obj_scroll_to_view_recursive(lv_label_create(tab1), LV_ANIM_ON);

    lv_group_t *group = lv_port_indev_init();
    lv_obj_add_event_cb(tabview, btn_event_cb_tab_next, LV_EVENT_KEY, NULL);
    lv_group_add_obj(group, tabview);
}

#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if     0       // 基于Base object 基础对象的UI

lv_obj_t *up_btn, *down_btn;
lv_obj_t *pow_ui1;


void scroll_list_by(lv_obj_t * boj, int y_offset) 
{
    if(y_offset > 0)
    {
        // printf("top %d\n", lv_obj_get_scroll_top(boj));
        if(lv_obj_get_scroll_top(boj) >= y_offset)
            lv_obj_scroll_by(boj, 0, y_offset, LV_ANIM_ON);
        else
            lv_obj_scroll_by(boj, 0, lv_obj_get_scroll_top(boj), LV_ANIM_ON);
    }
    else
    if(y_offset < 0)
    {
        // printf("bottom %d\n", lv_obj_get_scroll_bottom(boj));
        if(lv_obj_get_scroll_bottom(boj) >= LV_ABS(y_offset))
        {
            lv_obj_scroll_by(boj, 0, y_offset, LV_ANIM_ON);
        }
        else
        {
            lv_obj_scroll_by(boj, 0, -lv_obj_get_scroll_bottom(boj), LV_ANIM_ON);
        }
    }
}


void button_event_handler(lv_event_t *event) 
{
    lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t *btn_obj = lv_event_get_target(event);

    if(btn_obj == up_btn)
    {
        switch (code)
        {
            case LV_EVENT_SHORT_CLICKED: {
                scroll_list_by(pow_ui1, 100);  // 向上滚动
                
            }break;

            case LV_EVENT_LONG_PRESSED_REPEAT: {
                scroll_list_by(pow_ui1, 10);  // 向上滚动
                
            }break;

            default:
                break;
        }
        return ;
    }
    if(btn_obj == down_btn)
    {
        switch (code)
        {
            case LV_EVENT_SHORT_CLICKED: {
                scroll_list_by(pow_ui1, -100);  // 向下滚动
                
            }break;

            case LV_EVENT_LONG_PRESSED_REPEAT: {
                scroll_list_by(pow_ui1, -10);  // 向下滚动
               
            }break;
            
            default:
                break;
        }
        return ;
    }
}

void power_main_ui_init(void)
{
    lv_obj_t *scr = lv_scr_act();

    pow_ui1 = lv_obj_create(scr);

    lv_obj_set_size(pow_ui1, 240, 240);

    uint8_t panel_width = 220;
    uint8_t panel_heigth = 80;
    uint8_t align_interval_y = 15;

    lv_obj_t *c1_panel = lv_obj_create(pow_ui1);
    lv_obj_t *c2_panel = lv_obj_create(pow_ui1);
    lv_obj_t *c3_panel = lv_obj_create(pow_ui1);
    lv_obj_t *c4_panel = lv_obj_create(pow_ui1);

    lv_obj_set_size(c1_panel, panel_width, panel_heigth);
    lv_obj_align(c1_panel, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_set_size(c2_panel, panel_width, panel_heigth);
    lv_obj_align_to(c2_panel, c1_panel, LV_ALIGN_OUT_BOTTOM_MID, 0, align_interval_y);

    lv_obj_set_size(c3_panel, panel_width, panel_heigth);
    lv_obj_align_to(c3_panel, c2_panel, LV_ALIGN_OUT_BOTTOM_MID, 0, align_interval_y);

    lv_obj_set_size(c4_panel, panel_width, panel_heigth);
    lv_obj_align_to(c4_panel, c3_panel, LV_ALIGN_OUT_BOTTOM_MID, 0, align_interval_y);
    

    static lv_style_t style;
    lv_style_init(&style);

    // 设置圆角半径和半径
    lv_style_set_radius(&style, 10);
    // 设置背景颜色
    lv_style_set_bg_opa(&style, LV_OPA_COVER);
    lv_style_set_bg_color(&style, lv_color_hex(0xffffff));
    // 设置阴影
    lv_style_set_shadow_width(&style, 30);
    lv_style_set_shadow_color(&style, lv_color_hex(COLOR_TORRAS));

    lv_obj_add_style(c1_panel, &style, 0);
    lv_obj_add_style(c2_panel, &style, 0);
    lv_obj_add_style(c3_panel, &style, 0);
    lv_obj_add_style(c4_panel, &style, 0);

/* 
    // 创建上按钮
    up_btn = lv_btn_create(lv_scr_act());
    lv_obj_align(up_btn, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_set_size(up_btn, 20, 20);    
    lv_obj_add_event_cb(up_btn, button_event_handler, LV_EVENT_ALL, NULL);

    // 创建下按钮
    down_btn = lv_btn_create(lv_scr_act());
    lv_obj_align(down_btn, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_size(down_btn, 20, 20);        
    lv_obj_add_event_cb(down_btn, button_event_handler, LV_EVENT_ALL, NULL); */

}
#endif 

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if     1       // 基于Tabview 选项卡的UI 2

#define     PORT_MAX_NUM    4


lv_obj_t *up_btn, *down_btn, *en_btn;

lv_obj_t *tabview;
lv_obj_t *tab1, *tab2, *tab3;
uint8_t tab_idx = 0;

// 数据变量
lv_obj_t *port_info_label[PORT_MAX_NUM];

typedef struct {
    float volt;
    float curr;
    float pow;
    uint8_t max_power;
    char *protocol;
}port_display_t;

port_display_t port_info[4] = {
    {5.0, 3.0, 15.0, 80, "SCP"},
    {9.0, 3.0, 18.0, 20, "QC"},
    {20.0, 3.0, 100.0, 120, "PD3.0"},
    {28.0, 4.8, 134.4, 140, "PD3.1"}
};


/// @brief 带上下限限制的手动滚动条
/// @param boj 滚动对象
/// @param y_offset y轴滚动距离
void scroll_list_by(lv_obj_t * boj, int y_offset) 
{
    if(y_offset > 0)
    {
        // printf("top %d\n", lv_obj_get_scroll_top(boj));
        if(lv_obj_get_scroll_top(boj) >= y_offset)
            lv_obj_scroll_by(boj, 0, y_offset, LV_ANIM_ON);
        else
            lv_obj_scroll_by(boj, 0, lv_obj_get_scroll_top(boj), LV_ANIM_ON);
    }
    else
    if(y_offset < 0)
    {
        // printf("bottom %d\n", lv_obj_get_scroll_bottom(boj));
        if(lv_obj_get_scroll_bottom(boj) >= LV_ABS(y_offset))
        {
            lv_obj_scroll_by(boj, 0, y_offset, LV_ANIM_ON);
        }
        else
        {
            lv_obj_scroll_by(boj, 0, -lv_obj_get_scroll_bottom(boj), LV_ANIM_ON);
        }
    }
}


/// @brief 获取当前活跃的 Tab 对象
lv_obj_t *get_active_tab_obj(lv_obj_t *tabview) 
{
    uint16_t act_id = lv_tabview_get_tab_act(tabview);   // 获取当前活跃的 Tab 索引
    lv_obj_t *tab_content = lv_tabview_get_content(tabview); // 获取 Tab 内容区域
    return lv_obj_get_child(tab_content, act_id);  // 获取对应索引的 Tab 对象
}

void button_event_handler(lv_event_t *event) 
{
    lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t *btn_obj = lv_event_get_target(event);

    if(btn_obj == en_btn)
    {
        switch (code)
        {
            case LV_EVENT_SHORT_CLICKED: {

                
            }break;

            case LV_EVENT_LONG_PRESSED_REPEAT: {
                
            }break;

            default:
                break;
        }
        return ;
    }

    if(btn_obj == up_btn)
    {
        switch (code)
        {
            case LV_EVENT_SHORT_CLICKED: {
                // scroll_list_by(pow_ui1, 200);  // 向上滚动
                
            }break;

            case LV_EVENT_LONG_PRESSED_REPEAT: {
                // scroll_list_by(pow_ui1, 10);  // 向上滚动
                
            }break;

            default:
                break;
        }
        return ;
    }
    if(btn_obj == down_btn)
    {
        switch (code)
        {
            case LV_EVENT_SHORT_CLICKED: {
                // scroll_list_by(pow_ui1, -200);  // 向下滚动
                

                
            }break;

            case LV_EVENT_LONG_PRESSED_REPEAT: {
                // scroll_list_by(pow_ui1, -10);  // 向下滚动
               
            }break;
            
            default:
                break;
        }
        return ;
    }
}

void tabview_tab1(lv_obj_t *tab)
{
    lv_obj_t *c1_panel = lv_obj_create(tab);
    lv_obj_t *c2_panel = lv_obj_create(tab);
    lv_obj_t *c3_panel = lv_obj_create(tab);
    lv_obj_t *c4_panel = lv_obj_create(tab);

    uint8_t panel_width = 220;
    uint8_t panel_heigth = 50;
    uint8_t align_interval_y = 10;

    lv_obj_set_size(c1_panel, panel_width, panel_heigth);
    lv_obj_align(c1_panel, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_set_size(c2_panel, panel_width, panel_heigth);
    lv_obj_align_to(c2_panel, c1_panel, LV_ALIGN_OUT_BOTTOM_MID, 0, align_interval_y);

    lv_obj_set_size(c3_panel, panel_width, panel_heigth);
    lv_obj_align_to(c3_panel, c2_panel, LV_ALIGN_OUT_BOTTOM_MID, 0, align_interval_y);

    lv_obj_set_size(c4_panel, panel_width, panel_heigth);
    lv_obj_align_to(c4_panel, c3_panel, LV_ALIGN_OUT_BOTTOM_MID, 0, align_interval_y);

    // 滚动和滚动条设置
    lv_obj_set_scroll_dir(tab, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(tab, LV_SCROLLBAR_MODE_ACTIVE);
    lv_obj_set_scroll_dir(c1_panel, LV_DIR_NONE);
    lv_obj_set_scroll_dir(c2_panel, LV_DIR_NONE);
    lv_obj_set_scroll_dir(c3_panel, LV_DIR_NONE);
    lv_obj_set_scroll_dir(c4_panel, LV_DIR_NONE);

    static lv_style_t panel_style;
    lv_style_init(&panel_style);

    // 设置圆角半径和半径
    lv_style_set_radius(&panel_style, 10);
    // 设置背景颜色
    lv_style_set_bg_opa(&panel_style, LV_OPA_COVER);
    // lv_style_set_bg_color(&panel_style, lv_color_hex(0xffffff));
    // 设置阴影
    lv_style_set_shadow_width(&panel_style, 15);
    lv_style_set_shadow_color(&panel_style, lv_color_hex(COLOR_TORRAS));

    lv_obj_add_style(c1_panel, &panel_style, 0);
    lv_obj_add_style(c2_panel, &panel_style, 0);
    lv_obj_add_style(c3_panel, &panel_style, 0);
    lv_obj_add_style(c4_panel, &panel_style, 0);


    port_info_label[0] = lv_label_create(c1_panel);
    port_info_label[1] = lv_label_create(c2_panel);
    port_info_label[2] = lv_label_create(c3_panel);
    port_info_label[3] = lv_label_create(c4_panel);
    void timer_callback(lv_timer_t *timer);
    timer_callback(NULL);
    lv_obj_align_to(port_info_label[0], c1_panel, LV_ALIGN_TOP_LEFT, -8, -8);
    lv_obj_align_to(port_info_label[1], c2_panel, LV_ALIGN_TOP_LEFT, -8, -8);
    lv_obj_align_to(port_info_label[2], c3_panel, LV_ALIGN_TOP_LEFT, -8, -8);
    lv_obj_align_to(port_info_label[3], c4_panel, LV_ALIGN_TOP_LEFT, -8, -8);

    static lv_style_t label_style;
    lv_style_init(&label_style);
    lv_style_set_text_font(&label_style, &lv_font_montserrat_16);

    lv_obj_add_style(port_info_label[0], &label_style, 0);
    lv_obj_add_style(port_info_label[1], &label_style, 0);
    lv_obj_add_style(port_info_label[2], &label_style, 0);
    lv_obj_add_style(port_info_label[3], &label_style, 0);
}

void tabview_tab2(lv_obj_t *tab)
{
    LV_IMG_DECLARE(torras_jpg);
    lv_obj_t * img_jpg = lv_img_create(tab2);
    lv_img_set_src(img_jpg, &torras_jpg);
    lv_obj_align(img_jpg, LV_ALIGN_CENTER, 0, 0);
}


void timer_callback(lv_timer_t *timer)
{
    #define MAX_LEN 256

    char label_text[MAX_LEN];
    for(int i = 0; i < 4; i++)
    {
        uint8_t label_len = 0;

        port_info[i].max_power = 100;
        port_info[i].volt = (float)PORT[i].curr_vout / 1000;
        port_info[i].curr = (float)PORT[i].curr_iout / 1000;
        port_info[i].pow = (float)PORT[i].curr_power / 1000;

        if(PORT[i].port_state) {
            port_info[i].protocol = sw3566_protocol_str_list[PORT[i].protocol];
            label_len += snprintf(label_text + label_len, MAX_LEN - label_len, "%6.2fV   %5.2fA   %5.2fW\n", 
                        port_info[i].volt, port_info[i].curr, port_info[i].pow);
        }
        else {            
            port_info[i].protocol = "OFF";
        }  

        label_len += snprintf(label_text + label_len, MAX_LEN - label_len, "C%d    %s    MAX %5dW\n",
                                i+1, port_info[i].protocol, port_info[i].max_power);


        lv_label_set_text_fmt(port_info_label[i], "%s", label_text);
    }
}


void power_main_ui_init(void)
{
    lv_obj_t *scr = lv_scr_act();

    init_theme_default();

    tabview = lv_tabview_create(scr, LV_DIR_TOP, 5);

    // lv_obj_set_size(tabview, 240, 240);

    // lv_obj_t *tab_btns = lv_tabview_get_tab_btns(tabview); // 获取 Tab 按钮对象
    // lv_obj_add_flag(tab_btns, LV_OBJ_FLAG_HIDDEN);         // 隐藏 Tab 按钮栏

    tab1 = lv_tabview_add_tab(tabview, " ");
    tab2 = lv_tabview_add_tab(tabview, " ");
    tab3 = lv_tabview_add_tab(tabview, " ");

    tabview_tab1(tab1);
    tabview_tab2(tab2);

    lv_timer_create(timer_callback, 500, NULL);



/* 
    // 创建确认按钮
    en_btn = lv_btn_create(lv_scr_act());
    lv_obj_align(en_btn, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_size(en_btn, 20, 20);        
    lv_obj_add_event_cb(en_btn, button_event_handler, LV_EVENT_ALL, NULL);

    // 创建上按钮
    up_btn = lv_btn_create(lv_scr_act());
    lv_obj_align(up_btn, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_set_size(up_btn, 20, 20);    
    lv_obj_add_event_cb(up_btn, button_event_handler, LV_EVENT_ALL, NULL);

    // 创建下按钮
    down_btn = lv_btn_create(lv_scr_act());
    lv_obj_align(down_btn, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_size(down_btn, 20, 20);        
    lv_obj_add_event_cb(down_btn, button_event_handler, LV_EVENT_ALL, NULL); */
}
#endif 

void gui_init(void)
{
    if (lvgl_lock(-1)) 
    {
#if     0
        // 示例:对齐
        lvgl_example_align();
        // 示例:外部对齐
        // lvgl_example_align_out();
        // 示例:文本显示
        // lvgl_example_label();
        // 示例:图标显示
        lvgl_example_img();
        // 示例:按钮
        // lvgl_example_btn();

#else
        power_main_ui_init();

#endif 

        ESP_LOGI(TAG, "GUI init success");

        lvgl_unlock();   //释放互斥体
    }
}


