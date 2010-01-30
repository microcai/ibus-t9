/*
 * lookuptable.h
 *
 *  Created on: 2010-1-30
 *      Author: cai
 */

#ifndef LOOKUPTABLE_H_
#define LOOKUPTABLE_H_


gboolean on_button(GtkWidget* widget, GdkEventButton *event, gpointer user_data);
gboolean on_mouse_move(GtkWidget *widget, GdkEventMotion *event,gpointer user_data);
gboolean on_paint(GtkWidget *widget, GdkEventExpose *event,gpointer user_data);

#endif /* LOOKUPTABLE_H_ */
