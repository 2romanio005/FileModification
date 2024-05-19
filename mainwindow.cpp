#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "Functions.h"

#include <QFileDialog>
#include <QRegularExpressionValidator>
#include <QTimer>

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <windows.h>

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

    delete this->timer_periodic;

    delete this->ui;
}

// переключение с переодического запуска на еденичный
void MainWindow::on_radioButton_periodic_toggled(bool checked)
{
    this->ui->horizontalSlider_period_time->setEnabled(checked);
    this->ui->lineEdit_period_time->setEnabled(checked);
}

// выбор папки с входными файлами
void MainWindow::on_pushButton_select_input_path_clicked()
{
    const QString input_path = QFileDialog::getExistingDirectory(this, "Выбор директории с входными файлами", this->ui->lineEdit_input_path->text());

    if (!input_path.isEmpty()) {
        this->ui->lineEdit_input_path->setText(input_path);
    }
}

// выбор папки с выходными файлами
void MainWindow::on_pushButton_select_output_path_clicked()
{
    const QString output_path = QFileDialog::getExistingDirectory(this, "Выбор директории с выходными файлами", this->ui->lineEdit_output_path->text());

    if (!output_path.isEmpty()) {
        this->ui->lineEdit_output_path->setText(output_path);
    }
}

// вывести все файлы подходящие под маску
void MainWindow::on_pushButton_use_mask_clicked()
{
    QString resultText = "";

    for (QFileInfo &fileInfo : this->getSuitableForMaskFilesInfo()) {
        resultText.append(fileInfo.absoluteFilePath() + '\n');
    }
    // записать результат поиска в поле результата
    this->ui->textBrowser_result->setText(resultText);
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

// функция которая вызывается по таймеру
void MainWindow::timerPeriodicAction()
{
    ++number_of_launches;
    this->modifyDirectory();
}

// показать информационное окно (оно при ошибках вылазит, наверное можно поменять на critical но как то вроде нет) и остановить таймер
void MainWindow::showInforamtionMessage(const QString &title, const QString &text)
{
    this->stopTimerPeriodic();
    QMessageBox::information(this, title, text);
}

// активировать таймер
void MainWindow::startTimerPeriodic()
{
    this->number_of_launches = 1;
    this->timer_periodic->start(this->ui->horizontalSlider_period_time->value() * 1000);
    this->ui->pushButton_start->setText("Остановить переодический запуск модификации файлов");
}

// диактивировать таймер
void MainWindow::stopTimerPeriodic()
{
    if (this->timer_periodic->isActive()) {
        this->ui->pushButton_start->setText("Запустить модификацию файлов");
        this->timer_periodic->stop();
        QMessageBox::information(this, "Количество запусков по таймеру", "За вреремя работы по таймеру было совершено " + QString::number(this->number_of_launches) + " запусков.");
    }
}

// получить список файлов исходной директории подходящих под маску
QList<QFileInfo> MainWindow::getSuitableForMaskFilesInfo()
{
    QList<QFileInfo> allFilesInfo;

    QDir inputDir(this->ui->lineEdit_input_path->text());
    if (this->ui->lineEdit_input_path->text() == "" || !inputDir.exists()) {
        this->showInforamtionMessage("Отсутствует исходная дириктория", "Проверьте коректность введеного пути к директории с входными данными. Исходный путь не существует.");
        return allFilesInfo;
    }

    QRegularExpression file_mask(this->ui->lineEdit_file_mask->text());
    if (!file_mask.isValid()) {
        this->showInforamtionMessage("Ошибка при распознании маски", "Проверьте коректность заданой маски. \nОбнаружено: " + file_mask.errorString());
        return allFilesInfo;
    }

    QDirIterator fileIter(inputDir, QDirIterator::Subdirectories);
    QFileInfo fileInfo;
    while (fileIter.hasNext()) { // перебираем все объекты исходной дириктории
        fileInfo = fileIter.nextFileInfo();
        if (fileInfo.isFile() && file_mask.match(fileInfo.absoluteFilePath()).hasMatch()) { // отбираем из них только файлы и только подходящие под маску
            allFilesInfo.append(fileInfo);
        }
    }
    return allFilesInfo;
}

// модифицировать все нужные файлы из выбранной директории и записать их куда надо
void MainWindow::modifyDirectory()
{
    bool okResult;
    uint64_t modifyingValue = this->ui->lineEdit_HEX_for_modification->text().remove(' ').toULongLong(&okResult, 16);
    if (!okResult) {
        this->showInforamtionMessage("Ошибка при конвертацие HEX числа", "Проверьте коректность введённого числа для модификации.");
        return;
    }

    if (this->ui->lineEdit_output_path->text() == "") {
        this->showInforamtionMessage("Ошибка создания результирующей директории", "Не введён путь к результируюшей директории.");
        return;
    }

    //qDebug() << modifyingValue << "=======================================\n";

    QString resultText = "";

    for (QFileInfo &fileInfo : this->getSuitableForMaskFilesInfo()) {
        if (fileInfo.isWritable()) { // проверяем можем ли мы изменять исходный файл, предполагается что если он кем то занят то не можем
            // дальше идёт winApi, т.к. я не нашёл способа открыть файл в эксклюзивном режиме в Qt. Вариация того же, написанного на Qt, в верисии v0.0.1

            QString outputFileName = this->ui->lineEdit_output_path->text() + '/' + fileInfo.fileName();
            LPCWSTR outputFileNameWSTR = reinterpret_cast<LPCWSTR>(outputFileName.utf16());
            // модификация имени файла если стоит чекбокс
            if (this->buttonGroup_modify->checkedId()) {
                // задать новое имя результирующего файла добавив номер в конец   // нужно ли удлять

                while (GetFileAttributesW(outputFileNameWSTR) != INVALID_FILE_ATTRIBUTES) {
                    modifyFileName(outputFileName);
                    outputFileNameWSTR = reinterpret_cast<LPCWSTR>(outputFileName.utf16());
                }
            }

            HANDLE hOutputFile = CreateFileW(outputFileNameWSTR, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hOutputFile == INVALID_HANDLE_VALUE) {
                this->showInforamtionMessage("Ошибка создания результирующего файла", "Проверьте коректность введеного пути к результируюшей директории.");
                CloseHandle(hOutputFile);
                break;
            }

            QString inputFileName = (this->ui->lineEdit_input_path->text() + '/' + fileInfo.fileName());
            LPCWSTR inputFileNameWSTR = reinterpret_cast<LPCWSTR>(inputFileName.utf16());
            HANDLE hInputFile = CreateFileW(inputFileNameWSTR, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hInputFile == INVALID_HANDLE_VALUE) { // файлы которые не можем прочитать просто пропускаем
                CloseHandle(hOutputFile);
                CloseHandle(hInputFile);
                continue;
            }

            modifyFile(hInputFile, hOutputFile, modifyingValue);

            if(!this->ui->checkBox_result->isChecked()){
                resultText.append(inputFileName);
                resultText.append("\n└─────>\t"); // это красивы стрелочка если что
                resultText.append(outputFileName);
                resultText.push_back('\n');
            }

            CloseHandle(hOutputFile);
            CloseHandle(hInputFile);

            // удаление исходного файла если стоит чекбокс
            if (this->ui->checkBox_input_delete->isChecked()) {
                DeleteFile(inputFileNameWSTR);
            }
        }
    }
    // записать результат работы в соответствующее поле
    if(!this->ui->checkBox_result->isChecked()){
        this->ui->textBrowser_result->setText(resultText);
    }
}
