#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QRegularExpressionValidator>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    buttonGroup_modify = new QButtonGroup(this);
    buttonGroup_modify->addButton(ui->radioButton_overwrite, false);    // внимательней с bool id
    buttonGroup_modify->addButton(ui->radioButton_modify, true);        // внимательней с bool id
    ui->radioButton_modify->setChecked(true);

    buttonGroup_periodic = new QButtonGroup(this);
    buttonGroup_periodic->addButton(ui->radioButton_single, false);     // внимательней с bool id
    buttonGroup_periodic->addButton(ui->radioButton_periodic, true);    // внимательней с bool id
    ui->radioButton_single->setChecked(true);

    ui->lineEdit_HEX_for_modification->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9a-fA-F ]*")));
    ui->lineEdit_period_time->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]*")));
}

MainWindow::~MainWindow()
{
    delete ui;
}


// переключение с переодического запуска на еденичный
void MainWindow::on_radioButton_periodic_toggled(bool checked)
{
    ui->horizontalSlider_period_time->setEnabled(checked);
    ui->lineEdit_period_time->setEnabled(checked);
}

// выбор папки с входными файлами
void MainWindow::on_pushButton_select_input_path_clicked()
{
    const QString input_path = QFileDialog::getExistingDirectory(this, "Выбор папки с входными файлами", ui->lineEdit_input_path->text());

    if(!input_path.isEmpty()){
        ui->lineEdit_input_path->setText(input_path);
    }
}


// выбор папки с выходными файлами
void MainWindow::on_pushButton_select_output_path_clicked()
{
    const QString output_path = QFileDialog::getExistingDirectory(this, "Выбор папки с выходными файлами", ui->lineEdit_output_path->text());

    if(!output_path.isEmpty()){
        ui->lineEdit_output_path->setText(output_path);
    }
}

// движение слайдера для таймера повторения
void MainWindow::on_horizontalSlider_period_time_valueChanged(int value)
{
    ui->lineEdit_period_time->setText(QString::number(value));
}

// ввод таймера повторения
void MainWindow::on_lineEdit_period_time_editingFinished()
{
    int value = ui->lineEdit_period_time->text().toInt();
    ui->horizontalSlider_period_time->setMaximum(std::max(value, ui->horizontalSlider_period_time->maximum()));
    ui->horizontalSlider_period_time->setValue(value);
}


// нажата кнопка начать
void MainWindow::on_pushButton_start_clicked()
{
    // int a = buttonGroup_modify->checkedId();
    // QMessageBox::information(this, "sd", QString::number(a));

}


