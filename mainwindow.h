#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
#include <zrmchannelmimimal.h>
#include <qscreen.h>



class MainWindow : public QMainWindow, protected Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:

     void channel_activated(ZrmChannelMimimal *cm, bool bSelect);
     void action_toggled(bool checked);

     void slot_dev_error(const QString error_string);
     void write_log(QtMsgType msg_type, const QString &log_string);
     void set_style(const QString  & styleName);

     void edit_font_changed(const QFont &font);
     void edit_font_changed_props();
     void orientation_changed(Qt::ScreenOrientation screen_orient);

     void configure_apply();

     void onStyleToogled(bool checked);

    private:
      QFont edit_font  (const QFont &f);
      void init_styles ();
      void init_actions();
      void init_slots  ();
      void install_event_filers();

      //void configure_apply();
      void style_apply    ();

      static QString connectivity_file_name();
      static QString window_param_file_name();


      void set_font_for_edit ();

      void write_config       ();
      void read_config        ();
      void set_default_config ();
      bool eventFilter(QObject * target,QEvent * event) override;
#ifdef Q_OS_ANDROID
      void update_android_ui();
#else
      void update_desktop_ui();
#endif
      void update_ui();
      void setupStyleSheet();

      QFile               log_file;
      QTextStream         log_stream;
      QActionGroup      * m_action_grp     = Q_NULLPTR;
static QtMessageHandler   prev_msg_handler;
static void msg_handler   (QtMsgType msg_type, const QMessageLogContext & msg_context, const QString & msg_text);
void setupActions();
void updateFont(const QFont & fnt);
};

#endif // MAINWINDOW_H
