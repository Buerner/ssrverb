#ifndef FDNWINDOW_H
#define FDNWINDOW_H

#include <QMainWindow>
#include <QComboBox>

#include "reverbs/fdnverb/include/DynamicFDN.hpp"

namespace Ui {
class FDNwindow;
}

class FDNwindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit FDNwindow(QWidget *parent = 0);
    ~FDNwindow();

    // Source list callback for scene manager
    static void update_source_list( ssrface::Scene* audio_scene, void* source_list )
    {
        auto ids = audio_scene->get_source_ids();
        if ( ids.size() > 0 )
        {
            // Cast combo box.
            QComboBox* src_list = (QComboBox*)source_list;
            // Check if box is empty.
            if ( src_list->count() > 0 ) src_list->clear();

            for ( unsigned idx = 0; idx < ids.size(); idx++ ) {
                QString source_name = audio_scene->get_source(ids[idx])->name.c_str();
                src_list->addItem( source_name );
                usleep(10000);
            }
        }
    };

private slots:
    void closeEvent( QCloseEvent* event );

    void on_connect_button_clicked();

    void on_t60_band1_slider_valueChanged( int value );

    void on_t60_band2_slider_valueChanged( int value );

    void on_t60_band3_slider_valueChanged( int value );

    void on_low_freq_dial_valueChanged( int value );

    void on_high_freq_dial_valueChanged( int value );

    void on_source_selector_currentIndexChanged( int index );

    void on_source_selector_highlighted(int index);

    void on_gain_slider_valueChanged(int value);

    void on_mix_slider_valueChanged(int value);

    void on_room_x_box_valueChanged(double arg1);

    void on_room_y_box_valueChanged(double arg1);

    void on_room_z_box_valueChanged(double arg1);

    void on_tracking_checkbox_stateChanged(int state);



private:
    SSRverb::DynamicFDN _dfdn;
    Ui::FDNwindow *ui;
    float _a;
    float _b;
};

#endif // FDNWINDOW_H
