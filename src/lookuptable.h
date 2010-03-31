/*
 * lookuptable.h
 *
 *  Created on: 2010-1-30
 *      Author: cai
 */

#ifndef LOOKUPTABLE_H_
#define LOOKUPTABLE_H_

void svg_set_size(gint *width, gint *height, gpointer user_data);
void button_clicked(GtkButton *button, gpointer user_data);
void table_button_clicked(GtkButton *button, gpointer user_data);
gboolean on_button(GtkWidget* widget, GdkEventButton *event, gpointer user_data);
gboolean on_mouse_move(GtkWidget *widget, GdkEventMotion *event,gpointer user_data);
gboolean on_paint(GtkWidget *widget, GdkEventExpose *event,gpointer user_data);
void widget_realize(GtkWidget *widget, gpointer user_data);

#endif /* LOOKUPTABLE_H_ */
