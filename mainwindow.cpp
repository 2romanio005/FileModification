#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "Functions.h"

#include <QFileDialog>
#include <QRegularExpressionValidator>
#include <QTimer>

#include <QDataStream>
#include <QDir>
#include <QDirIterator>
#include <QLockFile>

#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    this->ui->setupUi(this);

    this->buttonGroup_modify = new QButtonGroup(this);
    this->buttonGroup_modify->addButton(this->ui->radioButton_overwrite, false); // внимательней с bool id
    this->buttonGroup_modify->addButton(this->ui->radioButton_modify, true);     // внимательней с bool id
    this->ui->radioButton_modify->setChecked(true);

    this->buttonGroup_periodic = new QButtonGroup(this);
    this->buttonGroup_periodic->addButton(this->ui->radioButton_single, false);  // внимательней с bool id
    this->buttonGroup_periodic->addButton(this->ui->radioButton_periodic, true); // внимательней с bool id
    this->ui->radioButton_single->setChecked(true);

    this->ui->lineEdit_HEX_for_modification->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9a-fA-F ]*")));
    this->ui->lineEdit_period_time->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]*")));

    this->timer_periodic = new QTimer(this);
    connect(this->timer_periodic, SIGNAL(timeout()), this, SLOT(timerPeriodicAction()));
}

MainWindow::~MainWindow()
{
    delete this->buttonGroup_modify;
    delete this->buttonGroup_periodic;

    delete this->ui;
}

// переключение с переодического запуска на еденичный
void MainWindow::on_radioButton_periodic_toggled(bool checked)
{
    this->ui->horizontalSlider_period_time->setEnabled(checked);
    this->ui->lineEdit_period_time->setEnabled(checked);

    if (!checked && this->timer_periodic->isActive()) {
        this->timer_periodic->stop();
    }
}

// выбор папки с входными файлами
void MainWindow::on_pushButton_select_input_path_clicked()
{
    const QString input_path = QFileDialog::getExistingDirectory(this, "Выбор папки с входными файлами", this->ui->lineEdit_input_path->text());

    if (!input_path.isEmpty()) {
        this->ui->lineEdit_input_path->setText(input_path);
    }
}

// выбор папки с выходными файлами
void MainWindow::on_pushButton_select_output_path_clicked()
{
    const QString output_path = QFileDialog::getExistingDirectory(this, "Выбор папки с выходными файлами", this->ui->lineEdit_output_path->text());

    if (!output_path.isEmpty()) {
        this->ui->lineEdit_output_path->setText(output_path);
    }
}

// движение слайдера для таймера повторения
void MainWindow::on_horizontalSlider_period_time_valueChanged(int value)
{
    this->ui->lineEdit_period_time->setText(QString::number(value));
}

// ввод таймера повторения
void MainWindow::on_lineEdit_period_time_textChanged(const QString &arg1)
{
    int value = arg1.toInt();
    this->ui->horizontalSlider_period_time->setMaximum(std::max(value, this->ui->horizontalSlider_period_time->maximum()));
    this->ui->horizontalSlider_period_time->setValue(value);
}

// нажата кнопка начать
void MainWindow::on_pushButton_start_clicked()
{
    if (this->timer_periodic->isActive()) {
        this->stopTimerPeriodic();
        return;
    }

    if (this->buttonGroup_periodic->checkedId()) {
        this->startTimerPeriodic();
    }

    this->modifyDirectory();
}

void MainWindow::timerPeriodicAction()
{
    ++number_of_launches;
    this->modifyDirectory();
}

void MainWindow::startTimerPeriodic()
{
    this->number_of_launches = 1;
    this->timer_periodic->start(this->ui->horizontalSlider_period_time->value() * 1000);
    this->ui->pushButton_start->setText("Остановить переодический запуск");
}

void MainWindow::stopTimerPeriodic()
{
    this->timer_periodic->stop();
    this->ui->pushButton_start->setText("Запустить модификацию");
    QMessageBox::information(this, "Количество запусков по таймеру", "За вреремя работы по таймеру было совершено " + QString::number(this->number_of_launches) + " запусков.");
}

void MainWindow::modifyDirectory()
{
    //QMessageBox::information(this, "Старт", "Старт");
    QDir input_dir(this->ui->lineEdit_input_path->text());
    if (!input_dir.exists()) {
        QMessageBox::information(this, "Отсутствует исходная дириктория", "Проверьте коректность введеного пути к директории с входными данными. Исходный путь не существует.");
        return;
    }

    QRegularExpression file_mask(this->ui->lineEdit_file_mask->text());
    if (!file_mask.isValid()) {
        QMessageBox::information(this, "Ошибка при распознании маски", "Проверьте коректность заданой маски. \nОбнаружено: " + file_mask.errorString());
        return;
    }

    bool okResult;
    uint64_t value_for_modification = this->ui->lineEdit_HEX_for_modification->text().remove(' ').toULongLong(&okResult, 16);
    if (!okResult) {
        QMessageBox::information(this, "Ошибка при конвертацие HEX числа", "Проверьте коректность заданого числа для модификации.");
        return;
    }

    //qDebug() << value_for_modification << "=======================================\n";

    QDirIterator file_iter(input_dir, QDirIterator::Subdirectories);
    QFileInfo file_info;
    while (file_iter.hasNext()) { // перебираем все объекты исходной дириктории
        file_info = file_iter.nextFileInfo();
        if (file_info.isFile() && file_mask.match(file_info.absoluteFilePath()).hasMatch()) { // отбираем из них только файлы и только подходящие под маску
            if (file_info.isWritable()) { // проверяем можем ли мы изменять исходный файл, предполагается что если он кем то занят то не можем
                QFile input_file(file_info.absoluteFilePath());
                QFile output_file(this->ui->lineEdit_output_path->text() + '/' + file_info.fileName());

                // модификация имени файла если чтоит чекбокс
                if (this->buttonGroup_modify->checkedId()) {
                    // задать новое имя результирующего файла добавив номер в конец
                    QString newFileName = output_file.fileName();
                    while (output_file.exists()) {
                        modifyFileName(newFileName);
                        output_file.setFileName(newFileName);
                    }
                }

                bool filtWriteCreated = output_file.open(QFile::WriteOnly);
                if (!filtWriteCreated) {
                    QMessageBox::information(this, "Ошибка создания результирующего файла", "Проверьте коректность введеного пути к результируюшей директории. \nОбнаружено: " + output_file.errorString());
                    return;
                }

                if (input_file.open(QFile::ReadOnly)) { // файлы которые не можем прочитать просто пропускаем
                    QDataStream in(&input_file);
                    QDataStream out(&output_file);

                    modifyFile(in, out, value_for_modification);

                    input_file.close();
                    output_file.close();
                }

                // удаление исходного файла если стоит чекбокс
                if (this->ui->checkBox_input_delete->isChecked()) {
                    input_file.remove();
                }
            }
        }
    }
}
