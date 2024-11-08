// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.4.2
// LVGL version: 9.1.0
// Project name: SPA

#include "../ui.h"


// COMPONENT uiFilter

lv_obj_t *ui_uiFilter_create(lv_obj_t *comp_parent) {

lv_obj_t *cui_uiFilter;
cui_uiFilter = lv_obj_create(comp_parent);
lv_obj_remove_style_all(cui_uiFilter);
lv_obj_set_width( cui_uiFilter, 60);
lv_obj_set_height( cui_uiFilter, 140);
lv_obj_set_x( cui_uiFilter, 60 );
lv_obj_set_y( cui_uiFilter, 63 );
lv_obj_set_align( cui_uiFilter, LV_ALIGN_CENTER );
lv_obj_set_flex_flow(cui_uiFilter,LV_FLEX_FLOW_COLUMN);
lv_obj_set_flex_align(cui_uiFilter, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
lv_obj_remove_flag( cui_uiFilter, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE );    /// Flags
lv_obj_set_style_radius(cui_uiFilter, 8, LV_PART_MAIN| LV_STATE_DEFAULT);
ui_object_set_themeable_style_property(cui_uiFilter, LV_PART_MAIN| LV_STATE_DEFAULT, LV_STYLE_BG_COLOR, _ui_theme_color_AccessoryOnBkg);
ui_object_set_themeable_style_property(cui_uiFilter, LV_PART_MAIN| LV_STATE_DEFAULT, LV_STYLE_BG_OPA, _ui_theme_alpha_AccessoryOnBkg);
ui_object_set_themeable_style_property(cui_uiFilter, LV_PART_MAIN| LV_STATE_DEFAULT, LV_STYLE_BORDER_COLOR, _ui_theme_color_widgetBorder);
ui_object_set_themeable_style_property(cui_uiFilter, LV_PART_MAIN| LV_STATE_DEFAULT, LV_STYLE_BORDER_OPA, _ui_theme_alpha_widgetBorder);
lv_obj_set_style_border_width(cui_uiFilter, 3, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_border_side(cui_uiFilter, LV_BORDER_SIDE_FULL, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_pad_left(cui_uiFilter, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_pad_right(cui_uiFilter, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_pad_top(cui_uiFilter, 10, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_pad_bottom(cui_uiFilter, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_color(cui_uiFilter, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_text_opa(cui_uiFilter, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

lv_obj_t *cui_Button;
cui_Button = lv_imagebutton_create(cui_uiFilter);
lv_imagebutton_set_src(cui_Button, LV_IMAGEBUTTON_STATE_RELEASED, NULL, &ui_img_filteron_png, NULL);
lv_imagebutton_set_src(cui_Button, LV_IMAGEBUTTON_STATE_PRESSED, NULL, &ui_img_filteron_png, NULL);
lv_imagebutton_set_src(cui_Button, LV_IMAGEBUTTON_STATE_DISABLED, NULL, &ui_img_filteron_png, NULL);
lv_imagebutton_set_src(cui_Button, LV_IMAGEBUTTON_STATE_CHECKED_PRESSED, NULL, &ui_img_filteron_png, NULL);
lv_imagebutton_set_src(cui_Button, LV_IMAGEBUTTON_STATE_CHECKED_RELEASED, NULL, &ui_img_filteron_png, NULL);
lv_imagebutton_set_src(cui_Button, LV_IMAGEBUTTON_STATE_CHECKED_DISABLED, NULL, &ui_img_filteron_png, NULL);
lv_obj_set_height( cui_Button, 100);
lv_obj_set_width( cui_Button, LV_SIZE_CONTENT);  /// 1
lv_obj_set_x( cui_Button, 82 );
lv_obj_set_y( cui_Button, 20 );
lv_obj_set_align( cui_Button, LV_ALIGN_CENTER );

lv_obj_t *cui_Label;
cui_Label = lv_label_create(cui_uiFilter);
lv_obj_set_width( cui_Label, 100);
lv_obj_set_height( cui_Label, 30);
lv_obj_set_x( cui_Label, 41 );
lv_obj_set_y( cui_Label, 47 );
lv_obj_set_align( cui_Label, LV_ALIGN_BOTTOM_MID );
lv_obj_set_flex_flow(cui_Label,LV_FLEX_FLOW_ROW);
lv_obj_set_flex_align(cui_Label, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
lv_label_set_text(cui_Label,"Filter");
lv_obj_set_style_pad_left(cui_Label, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_pad_right(cui_Label, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_pad_top(cui_Label, 3, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_pad_bottom(cui_Label, 8, LV_PART_MAIN| LV_STATE_DEFAULT);

lv_obj_t ** children = lv_malloc(sizeof(lv_obj_t *) * _UI_COMP_UIFILTER_NUM);
children[UI_COMP_UIFILTER_UIFILTER] = cui_uiFilter;
children[UI_COMP_UIFILTER_BUTTON] = cui_Button;
children[UI_COMP_UIFILTER_LABEL] = cui_Label;
lv_obj_add_event_cb(cui_uiFilter, get_component_child_event_cb, LV_EVENT_GET_COMP_CHILD, children);
lv_obj_add_event_cb(cui_uiFilter, del_component_child_event_cb, LV_EVENT_DELETE, children);
ui_comp_uiFilter_create_hook(cui_uiFilter);
return cui_uiFilter; 
}
