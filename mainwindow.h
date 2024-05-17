#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_radioButton_periodic_toggled(bool checked);

    void on_pushButton_select_input_path_clicked();
    void on_pushButton_select_output_path_clicked();

    void on_horizontalSlider_period_time_valueChanged(int value);
    void on_lineEdit_period_time_editingFinished();

    void on_pushButton_start_clicked();
private:
    Ui::MainWindow *ui;

    QButtonGroup *buttonGroup_modify;
    QButtonGroup *buttonGroup_periodic;
};
#endif // MAINWINDOW_H
